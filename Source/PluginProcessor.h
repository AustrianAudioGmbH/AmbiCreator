#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../resources/Delay.h"

//==============================================================================
/**
*/
class AafoaCreatorAudioProcessor  : public AudioProcessor, public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    AafoaCreatorAudioProcessor();
    ~AafoaCreatorAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    void processBlockBypassed (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    void parameterChanged (const String &parameterID, float newValue) override;

private:
    //==============================================================================
    AudioProcessorValueTreeState params;
    
    AudioBuffer<float> foaChannelBuffer;
    AudioBuffer<float> rotatorBuffer;
    
    bool isWCombined;
    bool doDifferentialZEqualization;
    bool doCoincPatternEqualization;
    int channelOrder;
    float outGainLin;
    float previousOutGainLin;
    float zGainLin;
    float previousZGainLin;
    float horRotationDeg;
    float previousCosPhi;
    float previousSinPhi;
    
    const float firLatencySec;
    double currentSampleRate;
    
    bool isBypassed;
    
    static constexpr double SQRT_ONE_OVER_4_PI = 0.282094791773878;
    
    Delay delays[3];
    
    // differential (z) compensation filters
    dsp::IIR::Filter<float> iirLowShelf;
    AudioBuffer<float> zFirCoeffBuffer;
    dsp::Convolution zFilterConv;
    AudioBuffer<float> coincEightFirCoeffBuffer;
    dsp::Convolution coincXEightFilterConv;
    dsp::Convolution coincYEightFilterConv;
    AudioBuffer<float> coincOmniFirCoeffBuffer;
    dsp::Convolution coincOmniFilterConv;
    
    static constexpr float MIN_Z_GAIN_DB = -40.0f;
    static constexpr float GAIN_TO_ZERO_THRESH_DB = 1.0f;
    
    enum eChannelOrder
    {
        ACN = 0,
        FUMA = 1
    };
    
    enum eChannelOrderACN
    {
        W = 0,
        Y = 1,
        Z = 2,
        X = 3
    };
    
    void setLowShelfCoefficients(double sampleRate);
    void ambiRotateAroundZ(AudioBuffer<float>* ambiBuffer);
    void updateLatency();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AafoaCreatorAudioProcessor)
};
