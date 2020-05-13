#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../resources/irs.h"

//==============================================================================
AafoaCreatorAudioProcessor::AafoaCreatorAudioProcessor() :
    AudioProcessor (BusesProperties()
                           .withInput  ("Input",  AudioChannelSet::ambisonic(1), true)
                           .withOutput ("Output", AudioChannelSet::ambisonic(1), true)
                           ),
    params(*this, nullptr, "AAFoaCreator", {
        std::make_unique<AudioParameterBool>("combinedW", "combined w channel", false, "",
                                            [](bool value, int) {return (value) ? "on" : "off";}, nullptr),
        std::make_unique<AudioParameterBool>("diffEqualization", "differential z equalization", true, "",
                                            [](bool value, int) {return (value) ? "on" : "off";}, nullptr),
        std::make_unique<AudioParameterBool>("coincEqualization", "omni and eight diffuse-field equalization", true, "",
                                            [](bool value, int) {return (value) ? "on" : "off";}, nullptr),
        std::make_unique<AudioParameterInt>("channelOrder", "channel order", eChannelOrder::ACN, eChannelOrder::FUMA, eChannelOrder::ACN, "",
                                            [](int value, int) {return (value == eChannelOrder::ACN) ? "ACN (WYZX)" : "FuMa (WXYZ)";}, nullptr),
        std::make_unique<AudioParameterFloat>("outGainDb", "output gain", NormalisableRange<float>(-40.0f, 10.0f, 0.1f),
                                              0.0f, "dB", AudioProcessorParameter::genericParameter,
                                              [](float value, int) { return String(value, 1); }, nullptr),
        std::make_unique<AudioParameterFloat>("zGainDb", "z gain", NormalisableRange<float>(MIN_Z_GAIN_DB, 10.0f, 0.1f),
                                              0.0f, "dB", AudioProcessorParameter::genericParameter,
                                              [](float value, int) { return (value > MIN_Z_GAIN_DB + GAIN_TO_ZERO_THRESH_DB) ? String(value, 1) : "-inf"; }, nullptr),
        std::make_unique<AudioParameterFloat>("horRotation", "horizontal rotation", NormalisableRange<float>(-180.0f, 180.0f, 1.0f),
                                              0.0f, String (CharPointer_UTF8 ("°")), AudioProcessorParameter::genericParameter,
                                              [](float value, int) { return String(value, 1); }, nullptr)
    }),
    firLatencySec((static_cast<float>(FIR_LEN) / 2 - 1) / FIR_SAMPLE_RATE),
    currentSampleRate(48000), isBypassed(false), zFirCoeffBuffer(1, FIR_LEN),
    coincEightFirCoeffBuffer(1, FIR_LEN), coincOmniFirCoeffBuffer(1, FIR_LEN)
{
    isWCombined                 = static_cast<bool>(params.getParameterAsValue("combinedW").getValue());
    doDifferentialZEqualization = static_cast<bool>(params.getParameterAsValue("diffEqualization").getValue());
    doCoincPatternEqualization  = static_cast<bool>(params.getParameterAsValue("coincEqualization").getValue());
    channelOrder                = static_cast<int>(params.getParameterAsValue("channelOrder").getValue());
    outGainLin                  = Decibels::decibelsToGain(static_cast<float>(params.getParameterAsValue("outGainDb").getValue()));
    zGainLin                    = Decibels::decibelsToGain(static_cast<float>(params.getParameterAsValue("zGainDb").getValue()));
    horRotationDeg              = static_cast<float>(params.getParameterAsValue("horRotation").getValue());
    
    params.addParameterListener("combinedW", this);
    params.addParameterListener("diffEqualization", this);
    params.addParameterListener("coincEqualization", this);
    params.addParameterListener("channelOrder", this);
    params.addParameterListener("outGainDb", this);
    params.addParameterListener("zGainDb", this);
    params.addParameterListener("horRotation", this);
    
    zFirCoeffBuffer.copyFrom(0, 0, DIFF_Z_EIGHT_EQ_COEFFS, FIR_LEN);
    coincEightFirCoeffBuffer.copyFrom(0, 0, COINC_EIGHT_EQ_COEFFS, FIR_LEN);
    coincOmniFirCoeffBuffer.copyFrom(0, 0, COINC_OMNI_EQ_COEFFS, FIR_LEN);
    
    for (auto& delay : delays)
        delay.setDelayTime (firLatencySec);    
}

AafoaCreatorAudioProcessor::~AafoaCreatorAudioProcessor()
{
}

//==============================================================================
const String AafoaCreatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AafoaCreatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AafoaCreatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AafoaCreatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AafoaCreatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AafoaCreatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AafoaCreatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AafoaCreatorAudioProcessor::setCurrentProgram (int)
{
}

const String AafoaCreatorAudioProcessor::getProgramName (int)
{
    return {};
}

void AafoaCreatorAudioProcessor::changeProgramName (int, const String&)
{
}

//==============================================================================
void AafoaCreatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    if (getTotalNumInputChannels() != 4 || getTotalNumOutputChannels() != 4)
        wrongBusConfiguration = true;
    else
        wrongBusConfiguration = false;
    
    foaChannelBuffer.setSize(4, samplesPerBlock);
    foaChannelBuffer.clear();
    
    rotatorBuffer.setSize(2, samplesPerBlock);
    rotatorBuffer.clear();
    
    previousOutGainLin = outGainLin;
    previousZGainLin = zGainLin;
    previousCosPhi = std::cosf(degreesToRadians(horRotationDeg));
    previousSinPhi = std::sinf(degreesToRadians(horRotationDeg));
    
    // low frequency compensation IIR for differential z signal
    //dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock), 1 };
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = 1;
    spec.maximumBlockSize = static_cast<uint32>(samplesPerBlock);
    
    iirLowShelf.prepare (spec);
    iirLowShelf.reset();
    setLowShelfCoefficients(sampleRate);
    
    // prepare fir filters
    zFilterConv.prepare (spec); // must be called before loading an ir
    zFilterConv.copyAndLoadImpulseResponseFromBuffer (zFirCoeffBuffer, FIR_SAMPLE_RATE, false, false, false, FIR_LEN);
    zFilterConv.reset();
    coincXEightFilterConv.prepare (spec); // must be called before loading an ir
    coincXEightFilterConv.copyAndLoadImpulseResponseFromBuffer (coincEightFirCoeffBuffer, FIR_SAMPLE_RATE, false, false, false, FIR_LEN);
    coincXEightFilterConv.reset();
    coincYEightFilterConv.prepare (spec); // must be called before loading an ir
    coincYEightFilterConv.copyAndLoadImpulseResponseFromBuffer (coincEightFirCoeffBuffer, FIR_SAMPLE_RATE, false, false, false, FIR_LEN);
    coincYEightFilterConv.reset();
    coincOmniFilterConv.prepare (spec); // must be called before loading an ir
    coincOmniFilterConv.copyAndLoadImpulseResponseFromBuffer (coincOmniFirCoeffBuffer, FIR_SAMPLE_RATE, false, false, false, FIR_LEN);
    coincOmniFilterConv.reset();
    
    // set delay time for w,x,y according to z fir delay
    for (auto& delay : delays)
        delay.prepare (spec);

    // set delay compensation
    updateLatency();
}

void AafoaCreatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AafoaCreatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannels() != 4 || layouts.getMainOutputChannels() != 4)
    {
        return false;
    }
    
    return true;
}

void AafoaCreatorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    auto playhead = getPlayHead();
    juce::AudioPlayHead::CurrentPositionInfo posInfo;
    playhead->getCurrentPosition(posInfo);
    isPlaying = posInfo.isPlaying;
    
    if (wrongBusConfiguration.get())
        return;
    
    jassert(buffer.getNumChannels() == 4 && totalNumOutputChannels == 4 && totalNumInputChannels == 4);
    
    int numSamples = buffer.getNumSamples();
    if (numSamples == 0)
        return;
    
    jassert(numSamples != 0);
    
    for (int i = 0; i < buffer.getNumChannels(); ++i)
    {
        float chRms = buffer.getRMSLevel (i, 0, numSamples);
        
        // cubase inputs a small noise when bypassing due to wrong channel layout
        if (chRms < 0.000000001f)
            channelActive[i] = false;
        else
            channelActive[i] = true;
    }
    
    if (isBypassed) {
        isBypassed = false;
        updateLatency();
    }
    
    const float* readPointerFront = buffer.getReadPointer (0);
    const float* readPointerBack = buffer.getReadPointer (1);
    const float* readPointerLeft = buffer.getReadPointer (2);
    const float* readPointerRight = buffer.getReadPointer (3);
    
    // internally everything is in ACN order
    float* writePointerW = foaChannelBuffer.getWritePointer (eChannelOrderACN::W);
    float* writePointerX = foaChannelBuffer.getWritePointer (eChannelOrderACN::X);
    float* writePointerY = foaChannelBuffer.getWritePointer (eChannelOrderACN::Y);
    float* writePointerZ = foaChannelBuffer.getWritePointer (eChannelOrderACN::Z);

    // W: take omni from mic 1
    FloatVectorOperations::copy (writePointerW, readPointerFront, numSamples);
    FloatVectorOperations::add (writePointerW, readPointerBack, numSamples);
    
    if (isWCombined)
    {
        // W: add omni signal from second mic
        FloatVectorOperations::add (writePointerW, readPointerLeft, numSamples);
        FloatVectorOperations::add (writePointerW, readPointerRight, numSamples);
        FloatVectorOperations::multiply(writePointerW, 0.5f, numSamples);
    }
    
    // X
    FloatVectorOperations::copy (writePointerX, readPointerFront, numSamples);
    FloatVectorOperations::subtract (writePointerX, readPointerBack, numSamples);
    
    // Y
    FloatVectorOperations::copy (writePointerY, readPointerLeft, numSamples);
    FloatVectorOperations::subtract (writePointerY, readPointerRight, numSamples);
    
    // Z: differential from both omnis, second mic is upper mic (positive z)
    FloatVectorOperations::copy (writePointerZ, readPointerLeft, numSamples);
    FloatVectorOperations::add (writePointerZ, readPointerRight, numSamples);
    FloatVectorOperations::subtract (writePointerZ, readPointerFront, numSamples);
    FloatVectorOperations::subtract (writePointerZ, readPointerBack, numSamples);
    
    if (doDifferentialZEqualization)
    {
        dsp::AudioBlock<float> zEqualizationBlock(&writePointerZ, 1, static_cast<size_t>(numSamples));
        dsp::ProcessContextReplacing<float> zEqualizationContext(zEqualizationBlock);
        iirLowShelf.process(zEqualizationContext);
        zFilterConv.process(zEqualizationContext);
    }
    
    if (doCoincPatternEqualization)
    {
        dsp::AudioBlock<float> wEqualizationBlock(&writePointerW, 1, static_cast<size_t>(numSamples));
        dsp::ProcessContextReplacing<float> wEqualizationContext(wEqualizationBlock);
        coincOmniFilterConv.process(wEqualizationContext);
        
        dsp::AudioBlock<float> xEqualizationBlock(&writePointerX, 1, static_cast<size_t>(numSamples));
        dsp::ProcessContextReplacing<float> xEqualizationContext(xEqualizationBlock);
        coincXEightFilterConv.process(xEqualizationContext);
        
        dsp::AudioBlock<float> yEqualizationBlock(&writePointerY, 1, static_cast<size_t>(numSamples));
        dsp::ProcessContextReplacing<float> yEqualizationContext(yEqualizationBlock);
        coincYEightFilterConv.process(yEqualizationContext);
    }
    else
    {
        // delay w, x and y accordingly
        dsp::AudioBlock<float> wDelayBlock(&writePointerW, 1, static_cast<size_t>(numSamples));
        dsp::ProcessContextReplacing<float> wDelayContext(wDelayBlock);
        delays[0].process(wDelayContext);
        
        dsp::AudioBlock<float> xDelayBlock(&writePointerX, 1, static_cast<size_t>(numSamples));
        dsp::ProcessContextReplacing<float> xDelayContext(xDelayBlock);
        delays[1].process(xDelayContext);
        
        dsp::AudioBlock<float> yDelayBlock(&writePointerY, 1, static_cast<size_t>(numSamples));
        dsp::ProcessContextReplacing<float> yDelayContext(yDelayBlock);
        delays[2].process(yDelayContext);
    }
        
    
    // apply sn3d weighting
    FloatVectorOperations::multiply(writePointerW, static_cast<float>(SQRT_ONE_OVER_4_PI), numSamples);
    FloatVectorOperations::multiply(writePointerX, static_cast<float>(SQRT_ONE_OVER_4_PI), numSamples);
    FloatVectorOperations::multiply(writePointerY, static_cast<float>(SQRT_ONE_OVER_4_PI), numSamples);
    FloatVectorOperations::multiply(writePointerZ, static_cast<float>(SQRT_ONE_OVER_4_PI), numSamples);
    
    // apply z gain
    foaChannelBuffer.applyGainRamp(eChannelOrderACN::Z, 0, numSamples, previousZGainLin, zGainLin);
    previousZGainLin = zGainLin;
    
    // apply output gain to all channels
    foaChannelBuffer.applyGainRamp(0, numSamples, previousOutGainLin, outGainLin);
    previousOutGainLin = outGainLin;
    
    // rotate
    ambiRotateAroundZ(&foaChannelBuffer);
    
    // write to output
    buffer.clear();
    
    if (buffer.getNumChannels() == 4 && totalNumOutputChannels == 4 && totalNumInputChannels == 4)
    {
        if (channelOrder == eChannelOrder::FUMA)
        {
            // reorder to fuma
            buffer.copyFrom(0, 0, foaChannelBuffer, eChannelOrderACN::W, 0, numSamples);
            buffer.copyFrom(1, 0, foaChannelBuffer, eChannelOrderACN::X, 0, numSamples);
            buffer.copyFrom(2, 0, foaChannelBuffer, eChannelOrderACN::Y, 0, numSamples);
            buffer.copyFrom(3, 0, foaChannelBuffer, eChannelOrderACN::Z, 0, numSamples);
        }
        else
        {
            // simply copy to output
            buffer.copyFrom(0, 0, foaChannelBuffer, eChannelOrderACN::W, 0, numSamples);
            buffer.copyFrom(1, 0, foaChannelBuffer, eChannelOrderACN::Y, 0, numSamples);
            buffer.copyFrom(2, 0, foaChannelBuffer, eChannelOrderACN::Z, 0, numSamples);
            buffer.copyFrom(3, 0, foaChannelBuffer, eChannelOrderACN::X, 0, numSamples);
        }
        
    }
    
}

void AafoaCreatorAudioProcessor::processBlockBypassed (AudioBuffer<float>& buffer, MidiBuffer&)
{
    if (!isBypassed) {
        isBypassed = true;
        updateLatency();
    }
    
    jassert (getLatencySamples() == 0);

    for (int ch = getMainBusNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

//==============================================================================
bool AafoaCreatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AafoaCreatorAudioProcessor::createEditor()
{
    return new AafoaCreatorAudioProcessorEditor (*this, params);
}

//==============================================================================
void AafoaCreatorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    std::unique_ptr<XmlElement> xml (params.state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AafoaCreatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (params.state.getType()))
        {
            params.state = ValueTree::fromXml (*xmlState);
        }
    }
}

//==============================================================================
void AafoaCreatorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "combinedW")
    {
        isWCombined = (newValue == 1.0f);
    }
    else if (parameterID == "diffEqualization")
    {
        doDifferentialZEqualization = (newValue == 1.0f);
        updateLatency();
    }
    else if (parameterID == "coincEqualization")
    {
        doCoincPatternEqualization = (newValue == 1.0f);
        updateLatency();
    }
    else if (parameterID == "channelOrder")
    {
        channelOrder = (static_cast<int>(newValue) == eChannelOrder::FUMA) ? eChannelOrder::FUMA : eChannelOrder::ACN;
    }
    else if (parameterID == "outGainDb")
    {
        outGainLin = Decibels::decibelsToGain(newValue);
    }
    else if (parameterID == "zGainDb")
    {
        if (newValue < MIN_Z_GAIN_DB + GAIN_TO_ZERO_THRESH_DB)
            zGainLin = 0.0f;
        else
            zGainLin = Decibels::decibelsToGain(newValue);
    }
    else if (parameterID == "horRotation")
    {
        horRotationDeg = newValue;
    }
}

//========================= CUSTOM METHODS =====================================
void AafoaCreatorAudioProcessor::setLowShelfCoefficients(double sampleRate)
{
    const double wc2 = 8418.4865639164;
    const double wc3 = 62.831853071795862;
    const double T = 1 / sampleRate;
    
    float b0 = static_cast<float>(T / 4 * (wc2 - wc3) + 0.5f);
    float b1 = static_cast<float>(-0.5f * std::exp(-wc3 * T) * (1 - T / 2 * (wc2 - wc3)));
    float a0 = 1.0f;
    float a1 = static_cast<float>(-std::exp(-wc3 * T));
    
    *iirLowShelf.coefficients = dsp::IIR::Coefficients<float>(b0,b1,a0,a1);
}

void AafoaCreatorAudioProcessor::ambiRotateAroundZ(AudioBuffer<float>* ambiBuffer) {
    auto numSamples = ambiBuffer->getNumSamples();
    jassert(numSamples == rotatorBuffer.getNumSamples());
    
    float cosPhi = std::cosf(degreesToRadians(horRotationDeg));
    float sinPhi = std::sinf(degreesToRadians(horRotationDeg));
    
    // write result to rotator buffer, it holds Y in channel 0 and X in channel 1
    rotatorBuffer.clear();
    rotatorBuffer.addFromWithRamp(0, 0, ambiBuffer->getReadPointer(eChannelOrderACN::Y), numSamples, previousCosPhi, cosPhi);
    rotatorBuffer.addFromWithRamp(0, 0, ambiBuffer->getReadPointer(eChannelOrderACN::X), numSamples, previousSinPhi, sinPhi);
    rotatorBuffer.addFromWithRamp(1, 0, ambiBuffer->getReadPointer(eChannelOrderACN::Y), numSamples, -previousSinPhi, -sinPhi);
    rotatorBuffer.addFromWithRamp(1, 0, ambiBuffer->getReadPointer(eChannelOrderACN::X), numSamples, previousCosPhi, cosPhi);
 
    ambiBuffer->copyFrom (eChannelOrderACN::Y, 0, rotatorBuffer, 0, 0, numSamples);
    ambiBuffer->copyFrom (eChannelOrderACN::X, 0, rotatorBuffer, 1, 0, numSamples);
    
    previousCosPhi = cosPhi;
    previousSinPhi = sinPhi;
}

void AafoaCreatorAudioProcessor::updateLatency() {
    if (isBypassed)
    {
        setLatencySamples(0);
    }
    else
    {
        if (doDifferentialZEqualization || doCoincPatternEqualization)
            setLatencySamples(static_cast<int>(firLatencySec * currentSampleRate));
        else // set delay compensation
            setLatencySamples(0);
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AafoaCreatorAudioProcessor();
}
