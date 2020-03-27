#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AafoaCreatorAudioProcessor::AafoaCreatorAudioProcessor() :
    AudioProcessor (BusesProperties()
                           .withInput  ("Input",  AudioChannelSet::ambisonic(1), true)
                           .withOutput ("Output", AudioChannelSet::ambisonic(1), true)
                           ),
    params(*this, nullptr, "AAFoaCreator", {
        std::make_unique<AudioParameterBool>("combinedW", "combined W channel", false, "",
                                            [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
        std::make_unique<AudioParameterBool>("diffEqualization", "differential z equalization", false, "",
                                            [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr),
        std::make_unique<AudioParameterBool>("coincEqualization", "omni and eight diffuse-field equalization", false, "",
                                            [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr)
    }),
    currentSampleRate(48000), zFirCoeffBuffer(1, FIR_LEN),
    coincEightFirCoeffBuffer(1, FIR_LEN), coincOmniFirCoeffBuffer(1, FIR_LEN)
{
    params.addParameterListener("combinedW", this);
    isWCombined = params.getRawParameterValue("combinedW");
    
    params.addParameterListener("diffEqualization", this);
    doDifferentialZEqualization = params.getRawParameterValue("diffEqualization");
    
    params.addParameterListener("coincEqualization", this);
    doCoincPatternEqualization = params.getRawParameterValue("coincEqualization");
    
    zFirCoeffBuffer.copyFrom(0, 0, DIFF_Z_EIGHT_EQ_COEFFS, FIR_LEN);
    coincEightFirCoeffBuffer.copyFrom(0, 0, COINC_EIGHT_EQ_COEFFS, FIR_LEN);
    coincOmniFirCoeffBuffer.copyFrom(0, 0, COINC_OMNI_EQ_COEFFS, FIR_LEN);
    
    firLatencySec = (static_cast<float>(FIR_LEN) / 2 - 1) / FIR_SAMPLE_RATE;
    
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

void AafoaCreatorAudioProcessor::setCurrentProgram (int index)
{
}

const String AafoaCreatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void AafoaCreatorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void AafoaCreatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    foaChannelBuffer.setSize(4, samplesPerBlock);
    foaChannelBuffer.clear();
    
    // low frequency compensation IIR for differential z signal
    //dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock), 1 };
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = 1;
    spec.maximumBlockSize = samplesPerBlock;
    
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
    if (doDifferentialZEqualization->load() == 1.0f)
        setLatencySamples(static_cast<int>(firLatencySec * sampleRate));
    
}

void AafoaCreatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AafoaCreatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::ambisonic(1)
     && layouts.getMainOutputChannelSet() != AudioChannelSet::discreteChannels(4))
        return false;
    
    return true;
}

void AafoaCreatorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    jassert(buffer.getNumChannels() == 4 && totalNumOutputChannels == 4 && totalNumInputChannels == 4);
    
    int numSamples = buffer.getNumSamples();
    
    const float* readPointerFront = buffer.getReadPointer (0);
    const float* readPointerBack = buffer.getReadPointer (1);
    const float* readPointerLeft = buffer.getReadPointer (2);
    const float* readPointerRight = buffer.getReadPointer (3);
    
    float* writePointerW = foaChannelBuffer.getWritePointer (0);
    float* writePointerX = foaChannelBuffer.getWritePointer (3);
    float* writePointerY = foaChannelBuffer.getWritePointer (1);
    float* writePointerZ = foaChannelBuffer.getWritePointer (2);

    // W: take omni from mic 1
    FloatVectorOperations::copy (writePointerW, readPointerFront, numSamples);
    FloatVectorOperations::add (writePointerW, readPointerBack, numSamples);
    
    if (isWCombined->load() == 1.0f)
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
    
    if (doDifferentialZEqualization->load() == 1.0f)
    {
        dsp::AudioBlock<float> zEqualizationBlock(&writePointerZ, 1, numSamples);
        dsp::ProcessContextReplacing<float> zEqualizationContext(zEqualizationBlock);
        iirLowShelf.process(zEqualizationContext);
        zFilterConv.process(zEqualizationContext);
    }
    
    if (doCoincPatternEqualization->load() == 1.0f)
    {
        dsp::AudioBlock<float> wEqualizationBlock(&writePointerW, 1, numSamples);
        dsp::ProcessContextReplacing<float> wEqualizationContext(wEqualizationBlock);
        coincOmniFilterConv.process(wEqualizationContext);
        
        dsp::AudioBlock<float> xEqualizationBlock(&writePointerX, 1, numSamples);
        dsp::ProcessContextReplacing<float> xEqualizationContext(xEqualizationBlock);
        coincXEightFilterConv.process(xEqualizationContext);
        
        dsp::AudioBlock<float> yEqualizationBlock(&writePointerY, 1, numSamples);
        dsp::ProcessContextReplacing<float> yEqualizationContext(yEqualizationBlock);
        coincYEightFilterConv.process(yEqualizationContext);
    }
    else
    {
        // delay w, x and y accordingly
        dsp::AudioBlock<float> wDelayBlock(&writePointerW, 1, numSamples);
        dsp::ProcessContextReplacing<float> wDelayContext(wDelayBlock);
        delays[0].process(wDelayContext);
        
        dsp::AudioBlock<float> xDelayBlock(&writePointerX, 1, numSamples);
        dsp::ProcessContextReplacing<float> xDelayContext(xDelayBlock);
        delays[1].process(xDelayContext);
        
        dsp::AudioBlock<float> yDelayBlock(&writePointerY, 1, numSamples);
        dsp::ProcessContextReplacing<float> yDelayContext(yDelayBlock);
        delays[2].process(yDelayContext);
    }
        
    
    // apply sn3d weighting
    FloatVectorOperations::multiply(writePointerW, SN3D_WEIGHT_0, numSamples);
    FloatVectorOperations::multiply(writePointerX, SN3D_WEIGHT_1, numSamples);
    FloatVectorOperations::multiply(writePointerY, SN3D_WEIGHT_1, numSamples);
    FloatVectorOperations::multiply(writePointerZ, SN3D_WEIGHT_1, numSamples);
    
    // write to output
    buffer.clear();
    
    if (buffer.getNumChannels() == 4 && totalNumOutputChannels == 4 && totalNumInputChannels == 4)
    {
        for (int out = 0; out < 4; ++out)
            buffer.copyFrom(out, 0, foaChannelBuffer, out, 0, numSamples);
    }
    
}

//==============================================================================
bool AafoaCreatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AafoaCreatorAudioProcessor::createEditor()
{
    return new AafoaCreatorAudioProcessorEditor (*this);
}

//==============================================================================
void AafoaCreatorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AafoaCreatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
void AafoaCreatorAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "diffEqualization")
    {
        if (newValue == 0)
        {
            setLatencySamples(0);
        }
        else
        {
            // set delay compensation
            setLatencySamples(static_cast<int>(firLatencySec * currentSampleRate));
        }
    }
}

void AafoaCreatorAudioProcessor::setLowShelfCoefficients(double sampleRate)
{
    const double wc2 = 8418.486563916398;
    const double wc3 = 62.831853071795862;
    const double T = 1 / sampleRate;
    
    float b0 = T/2 * (wc2 - wc3) + 1;
    float b1 = -std::exp(-wc3 * T) * (1 - T/2 * (wc2 - wc3));
    float a0 = 1.0f;
    float a1 = -std::exp(-wc3 * T);
    
    *iirLowShelf.coefficients = dsp::IIR::Coefficients<float>(b0,b1,a0,a1);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AafoaCreatorAudioProcessor();
}
