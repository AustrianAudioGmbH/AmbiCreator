/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

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
        std::make_unique<AudioParameterBool>("diffLowShelf", "differential low shelf", false, "",
                                            [](bool value, int maximumStringLength) {return (value) ? "on" : "off";}, nullptr)
    }),
    zFirCoeffBuffer(1, Z_FIR_LEN)
{
    params.addParameterListener("combinedW", this);
    isWCombined = params.getRawParameterValue("combinedW");
    
    params.addParameterListener("diffLowShelf", this);
    doDifferentialZEqualization = params.getRawParameterValue("diffLowShelf");
    
    zFirCoeffBuffer.copyFrom(0, 0, DIFF_Z_EIGHT_EQ_COEFFS, Z_FIR_LEN);
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
    foaChannelBuffer.setSize(4, samplesPerBlock);
    foaChannelBuffer.clear();
    
    // low frequency compensation IIR for differential z signal
    dsp::ProcessSpec specLowShelf { sampleRate, static_cast<uint32> (samplesPerBlock), 1 };
    iirLowShelf.prepare (specLowShelf);
    iirLowShelf.reset();
    setLowShelfCoefficients(sampleRate);
    
    // prepare fir filter
    dsp::ProcessSpec firSpec {sampleRate, static_cast<uint32>(samplesPerBlock), 1};
    zFilterConv.prepare (firSpec); // must be called before loading an ir
    zFilterConv.copyAndLoadImpulseResponseFromBuffer (zFirCoeffBuffer, FIR_SAMPLE_RATE, false, false, false, Z_FIR_LEN);
    zFilterConv.reset();
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
    
    if (*isWCombined == 1.0f)
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
    
    if (*doDifferentialZEqualization == 1.0f)
    {
        dsp::AudioBlock<float> zEqualizationBlock(&writePointerZ, 1, numSamples);
        dsp::ProcessContextReplacing<float> zEqualizationContext(zEqualizationBlock);
        iirLowShelf.process(zEqualizationContext);
        zFilterConv.process(zEqualizationContext);
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
