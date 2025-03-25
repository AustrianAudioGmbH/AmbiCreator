#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../resources/irs.h"

/* We use versionHint of ParameterID from now on - rigorously! */
#define PD_PARAMETER_V1 1

//==============================================================================
AmbiCreatorAudioProcessor::AmbiCreatorAudioProcessor() :
    AudioProcessor (BusesProperties()
                           .withInput  ("Input",  AudioChannelSet::ambisonic(1), true)
                           .withOutput ("Output", AudioChannelSet::ambisonic(1), true)
                           ),
    params(*this, nullptr, "AmbiCreator", {

        std::make_unique<AudioParameterInt>(ParameterID{"channelOrder",PD_PARAMETER_V1}, "channel order", eChannelOrder::ACN, eChannelOrder::FUMA, eChannelOrder::ACN, "", [](int value, int) {return (value == eChannelOrder::ACN) ? "AmbiX (WYZX)" : "FuMa (WXYZ)";}, nullptr),
        std::make_unique<AudioParameterFloat>(ParameterID{"outGainDb", PD_PARAMETER_V1},"output gain", NormalisableRange<float>(-40.0f, 10.0f, 0.1f),0.0f, "dB", AudioProcessorParameter::genericParameter,[](float value, int) { return String(value, 1); }, nullptr),
        std::make_unique<AudioParameterFloat>(ParameterID{"zGainDb", PD_PARAMETER_V1}, "z gain", NormalisableRange<float>(MIN_Z_GAIN_DB, 10.0f, 0.1f),0.0f, "dB", AudioProcessorParameter::genericParameter,[](float value, int) { return (value > MIN_Z_GAIN_DB + GAIN_TO_ZERO_THRESH_DB) ? String(value, 1) : "-inf"; }, nullptr),
        std::make_unique<AudioParameterFloat>(ParameterID{"horRotation",PD_PARAMETER_V1}, "horizontal rotation", NormalisableRange<float>(-180.0f, 180.0f, 1.0f),0.0f, String (CharPointer_UTF8 ("°")), AudioProcessorParameter::genericParameter,[](float value, int) { return String(value, 1); }, nullptr),
        std::make_unique<AudioParameterBool>(ParameterID{"legacyMode",PD_PARAMETER_V1}, "Legacy Mode", true, "", [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr)

    }),
    firLatencySec((static_cast<float>(FIR_LEN) / 2 - 1) / FIR_SAMPLE_RATE),
    currentSampleRate(48000), isBypassed(false), zFirCoeffBuffer(1, FIR_LEN),
    coincEightXFirCoeffBuffer(1, FIR_LEN), coincEightYFirCoeffBuffer(1, FIR_LEN), coincOmniFirCoeffBuffer(1, FIR_LEN),
    editorWidth(EDITOR_DEFAULT_WIDTH), editorHeight(EDITOR_DEFAULT_HEIGHT),
    layerA(nodeA), layerB(nodeB), allValueTreeStates(allStates)
{
    channelOrder                = static_cast<int>(params.getParameterAsValue("channelOrder").getValue());
    outGainLin                  = Decibels::decibelsToGain(static_cast<float>(params.getParameterAsValue("outGainDb").getValue()));
    zGainLin                    = Decibels::decibelsToGain(static_cast<float>(params.getParameterAsValue("zGainDb").getValue()));
    horRotationDeg              = static_cast<float>(params.getParameterAsValue("horRotation").getValue());
    
    params.addParameterListener("channelOrder", this);
    params.addParameterListener("outGainDb", this);
    params.addParameterListener("zGainDb", this);
    params.addParameterListener("horRotation", this);
    params.addParameterListener("legacyMode", this);

    legacyModePtr = params.getRawParameterValue("legacyMode");

//    DBG("PROCESSOR INIT LEGACY MODE: " << String((legacyModePtr->load() > 0) ? "TRUE" : "FALSE"));

    zFirCoeffBuffer.copyFrom(0, 0, DIFF_Z_EIGHT_EQ_COEFFS, FIR_LEN);
    coincEightXFirCoeffBuffer.copyFrom(0, 0, COINC_EIGHT_EQ_COEFFS, FIR_LEN);
    coincEightYFirCoeffBuffer.copyFrom(0, 0, COINC_EIGHT_EQ_COEFFS, FIR_LEN);
    coincOmniFirCoeffBuffer.copyFrom(0, 0, COINC_OMNI_EQ_COEFFS, FIR_LEN);


}

AmbiCreatorAudioProcessor::~AmbiCreatorAudioProcessor()
{

}

//==============================================================================
const String AmbiCreatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int AmbiCreatorAudioProcessor::getChannelOrder()
{
    return channelOrder;
}

bool AmbiCreatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AmbiCreatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AmbiCreatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AmbiCreatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AmbiCreatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AmbiCreatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmbiCreatorAudioProcessor::setCurrentProgram (int)
{
}

const String AmbiCreatorAudioProcessor::getProgramName (int)
{
    return {};
}

void AmbiCreatorAudioProcessor::changeProgramName (int, const String&)
{
}

//==============================================================================
void AmbiCreatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
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
    
    legacyModeReorderBuffer.setSize(4, samplesPerBlock);
    legacyModeReorderBuffer.clear();
    
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
    zFilterConv.loadImpulseResponse(std::move(zFirCoeffBuffer), FIR_SAMPLE_RATE, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    zFilterConv.reset();
    coincXEightFilterConv.prepare (spec); // must be called before loading an ir
    coincXEightFilterConv.loadImpulseResponse(std::move(coincEightXFirCoeffBuffer), FIR_SAMPLE_RATE, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    coincXEightFilterConv.reset();

    coincYEightFilterConv.prepare (spec); // must be called before loading an ir
    coincYEightFilterConv.loadImpulseResponse(std::move(coincEightYFirCoeffBuffer), FIR_SAMPLE_RATE, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    coincYEightFilterConv.reset();

    coincOmniFilterConv.prepare (spec); // must be called before loading an ir
    coincOmniFilterConv.loadImpulseResponse(std::move(coincOmniFirCoeffBuffer), FIR_SAMPLE_RATE, dsp::Convolution::Stereo::no, dsp::Convolution::Trim::no, dsp::Convolution::Normalise::no);
    coincOmniFilterConv.reset();

    // set delay compensation
    updateLatency();
}

void AmbiCreatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AmbiCreatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannels() != 4 || layouts.getMainOutputChannels() != 4)
    {
        return false;
    }
    
    return true;
}

void AmbiCreatorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    int numSamples = buffer.getNumSamples();
    
    // if legacy mode is disabled (input: LRFB) - audio is always processed with FBLR
    if (legacyModePtr->load() > 0.5)
    {
        legacyModeReorderBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples()); // L
        legacyModeReorderBuffer.copyFrom(1, 0, buffer, 1, 0, buffer.getNumSamples()); // R
        legacyModeReorderBuffer.copyFrom(2, 0, buffer, 2, 0, buffer.getNumSamples()); // F
        legacyModeReorderBuffer.copyFrom(3, 0, buffer, 3, 0, buffer.getNumSamples()); // B
        
        buffer.clear();
        buffer.copyFrom(0, 0, legacyModeReorderBuffer, 2, 0, buffer.getNumSamples()); // F
        buffer.copyFrom(1, 0, legacyModeReorderBuffer, 3, 0, buffer.getNumSamples()); // B
        buffer.copyFrom(2, 0, legacyModeReorderBuffer, 0, 0, buffer.getNumSamples()); // L
        buffer.copyFrom(3, 0, legacyModeReorderBuffer, 1, 0, buffer.getNumSamples()); // R
        
        for (int i = 0; i < buffer.getNumChannels(); ++i)
        {
            inRms[i] = legacyModeReorderBuffer.getRMSLevel (i, 0, numSamples);
            
            // cubase inputs a small noise when bypassing due to wrong channel layout
            if (inRms[i].get() < 0.000000001f)
                channelActive[i] = false;
            else
                channelActive[i] = true;
        }
    }
    else
    {
        for (int i = 0; i < buffer.getNumChannels(); ++i)
        {
            inRms[i] = buffer.getRMSLevel (i, 0, numSamples);
            
            // cubase inputs a small noise when bypassing due to wrong channel layout
            if (inRms[i].get() < 0.000000001f)
                channelActive[i] = false;
            else
                channelActive[i] = true;
        }
    }
    
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
    
    
    jassert(numSamples != 0);
    if (numSamples == 0)
        return;
    

    
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
    FloatVectorOperations::copy (writePointerW, readPointerLeft, numSamples);
    FloatVectorOperations::add (writePointerW, readPointerRight, numSamples);
    
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
    
    // equalization of differential z channel
    dsp::AudioBlock<float> zEqualizationBlock(&writePointerZ, 1, static_cast<size_t>(numSamples));
    dsp::ProcessContextReplacing<float> zEqualizationContext(zEqualizationBlock);
    iirLowShelf.process(zEqualizationContext);
    zFilterConv.process(zEqualizationContext);
    
    // equalization of coincident channels
    dsp::AudioBlock<float> wEqualizationBlock(&writePointerW, 1, static_cast<size_t>(numSamples));
    dsp::ProcessContextReplacing<float> wEqualizationContext(wEqualizationBlock);
    coincOmniFilterConv.process(wEqualizationContext);
    
    dsp::AudioBlock<float> xEqualizationBlock(&writePointerX, 1, static_cast<size_t>(numSamples));
    dsp::ProcessContextReplacing<float> xEqualizationContext(xEqualizationBlock);
    coincXEightFilterConv.process(xEqualizationContext);
    
    dsp::AudioBlock<float> yEqualizationBlock(&writePointerY, 1, static_cast<size_t>(numSamples));
    dsp::ProcessContextReplacing<float> yEqualizationContext(yEqualizationBlock);
    coincYEightFilterConv.process(yEqualizationContext);
    
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
            // reorder and normalize for FUMA (N3D)
            FloatVectorOperations::multiply(writePointerX, static_cast<float>(SQRT_THREE), numSamples);
            FloatVectorOperations::multiply(writePointerY, static_cast<float>(SQRT_THREE), numSamples);
            FloatVectorOperations::multiply(writePointerZ, static_cast<float>(SQRT_THREE), numSamples);
            
            buffer.copyFrom(0, 0, foaChannelBuffer, eChannelOrderACN::W, 0, numSamples);
            buffer.copyFrom(1, 0, foaChannelBuffer, eChannelOrderACN::X, 0, numSamples);
            buffer.copyFrom(2, 0, foaChannelBuffer, eChannelOrderACN::Y, 0, numSamples);
            buffer.copyFrom(3, 0, foaChannelBuffer, eChannelOrderACN::Z, 0, numSamples);
        }
        else
        {
            // only reorder for AmbiX (ACN + SN3D) -> SN3D in FOA applies same weights to all components
            buffer.copyFrom(0, 0, foaChannelBuffer, eChannelOrderACN::W, 0, numSamples);
            buffer.copyFrom(1, 0, foaChannelBuffer, eChannelOrderACN::Y, 0, numSamples);
            buffer.copyFrom(2, 0, foaChannelBuffer, eChannelOrderACN::Z, 0, numSamples);
            buffer.copyFrom(3, 0, foaChannelBuffer, eChannelOrderACN::X, 0, numSamples);
        }
        
    }
    
    for (int i = 0; i < buffer.getNumChannels(); ++i)
        outRms[i] = buffer.getRMSLevel (i, 0, numSamples);
    
}

void AmbiCreatorAudioProcessor::processBlockBypassed (AudioBuffer<float>& buffer, MidiBuffer&)
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
bool AmbiCreatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AmbiCreatorAudioProcessor::createEditor()
{
    return new AmbiCreatorAudioProcessorEditor (*this, params);
}

//==============================================================================
void AmbiCreatorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
//    params.state.setProperty("editorWidth", var(editorWidth), nullptr);
//    params.state.setProperty("editorHeight", var(editorHeight), nullptr);
//    std::unique_ptr<XmlElement> xml (params.state.createXml());
//    copyXmlToBinary (*xml, destData);


//DBG("PROCESSOR GETSTATEINFORMATION: isLegacyMode: " << juce::String(isNormalLRFBMode() ? "TRUE" : "FALSE"));
    
    if (abLayerState == eCurrentActiveLayer::layerA)
    {
        layerA.getPropertyAsValue("legacyMode", nullptr).setValue(var(isNormalLRFBMode()));
        layerA = params.copyState();
    }
    else if (abLayerState == eCurrentActiveLayer::layerB)
    {
        layerB.getPropertyAsValue("legacyMode", nullptr).setValue(var(isNormalLRFBMode()));
        layerB = params.copyState();
    }

//    layerA.setProperty("editorWidth", var(editorWidth), nullptr);
//    layerA.setProperty("editorHeight", var(editorWidth), nullptr);
//    layerB.setProperty("editorWidth", var(editorWidth), nullptr);
//    layerB.setProperty("editorHeight", var(editorWidth), nullptr);
    
    ValueTree vtsState = params.copyState();
    ValueTree AState = layerA.createCopy();
    ValueTree BState = layerB.createCopy();
    
    allValueTreeStates.removeAllChildren(nullptr);
    allValueTreeStates.addChild(vtsState, 0, nullptr);
    allValueTreeStates.addChild(AState, 1, nullptr);
    allValueTreeStates.addChild(BState, 2, nullptr);
    
    std::unique_ptr<XmlElement> xml (allValueTreeStates.createXml());
    copyXmlToBinary (*xml, destData);
}

void AmbiCreatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
//    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
//    if (xmlState != nullptr)
//    {
//        if (xmlState->hasTagName (params.state.getType()))
//        {
//            params.state = ValueTree::fromXml (*xmlState);
//        }
//        if (params.state.hasProperty("editorWidth"))
//        {
//            Value val = params.state.getPropertyAsValue("editorWidth", nullptr);
//            if (val.getValue().toString() != "")
//            {
//                editorWidth = static_cast<int>(val.getValue());
//            }
//        }
//        if (params.state.hasProperty("editorHeight"))
//        {
//            Value val = params.state.getPropertyAsValue("editorHeight", nullptr);
//            if (val.getValue().toString() != "")
//            {
//                editorHeight = static_cast<int>(val.getValue());
//            }
//        }
//    }
    
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (allValueTreeStates.getType()))
        {
            allValueTreeStates = ValueTree::fromXml (*xmlState);
            params.replaceState(allValueTreeStates.getChild(0));
            layerB = allValueTreeStates.getChild(2).createCopy();
        }
        else if (xmlState->hasTagName (params.state.getType()))
        {
            params.state = ValueTree::fromXml (*xmlState);
        }

        
//        if (params.state.hasProperty("editorWidth"))
//        {
//            Value val = params.state.getPropertyAsValue("editorWidth", nullptr);
//            if (val.getValue().toString() != "")
//            {
//                editorWidth = static_cast<int>(val.getValue());
//            }
//        }
//        if (params.state.hasProperty("editorHeight"))
//        {
//            Value val = params.state.getPropertyAsValue("editorHeight", nullptr);
//            if (val.getValue().toString() != "")
//            {
//                editorHeight = static_cast<int>(val.getValue());
//            }
//        }
    }
}

//==============================================================================
void AmbiCreatorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "channelOrder")
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
    else if (parameterID == "legacyMode") {
        legacyModePtr->store(newValue);
    }
}

//========================= CUSTOM METHODS =====================================
void AmbiCreatorAudioProcessor::setLowShelfCoefficients(double sampleRate)
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

void AmbiCreatorAudioProcessor::ambiRotateAroundZ(AudioBuffer<float>* ambiBuffer) {
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

void AmbiCreatorAudioProcessor::updateLatency() {
    if (isBypassed)
        setLatencySamples(0);
    else
        setLatencySamples(static_cast<int>(firLatencySec * currentSampleRate));
}

void AmbiCreatorAudioProcessor::setAbLayer(int desiredLayer)
{
    abLayerState = desiredLayer;
    changeAbLayerState();
}

void AmbiCreatorAudioProcessor::changeAbLayerState()
{
    bool currentLegacyMode = isNormalLRFBMode();
    
    if (abLayerState == eCurrentActiveLayer::layerB)
    {
        layerA = params.copyState();
        params.state = layerB.createCopy();
    }
    else
    {
        layerB = params.copyState();
        params.state = layerA.createCopy();
    }
    params.getParameter("legacyMode")->setValueNotifyingHost(currentLegacyMode);
}


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmbiCreatorAudioProcessor();
}
