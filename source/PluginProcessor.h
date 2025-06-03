#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

enum eCurrentActiveLayer
{
    layerA = 0,
    layerB = 1
};

//==============================================================================
/**
*/
class AmbiCreatorAudioProcessor  : public AudioProcessor, public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    AmbiCreatorAudioProcessor();
    ~AmbiCreatorAudioProcessor() override;

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

    int getChannelOrder();

    //==============================================================================
    void parameterChanged (const String &parameterID, float newValue) override;
    
    int getEditorWidth() {return editorWidth;}
//    void setEditorWidth(int width) {editorWidth = width;}
    int getEditorHeight() {return editorHeight;}
//    void setEditorHeight(int height) {editorHeight = height;}
    
    bool isNormalLRFBMode() {
        return *legacyModePtr > 0.5f;
    }

    void setAbLayer(int desiredLayer);
    void changeAbLayerState(int desiredLayer);
    
    Atomic<bool> wrongBusConfiguration = false;
    Atomic<bool> channelActive[4] = { true, true, true, true };
    Atomic<bool> isPlaying = false;
    Atomic<float> inRms[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Atomic<float> outRms[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    static const int EDITOR_DEFAULT_WIDTH = 650;
    static const int EDITOR_DEFAULT_HEIGHT = 500;
    int abLayerState;

private:
    //==============================================================================
    AudioProcessorValueTreeState params;
    CriticalSection stateLock; // Protect ValueTree operations

    AudioBuffer<float> foaChannelBuffer;
    AudioBuffer<float> rotatorBuffer;
    AudioBuffer<float> legacyModeReorderBuffer;
    
    int channelOrder;
    float outGainLin;
    float previousOutGainLin;
    float zGainLin;
    float previousZGainLin;
    float horRotationDeg;
    float previousCosPhi;
    float previousSinPhi;
    
    float firLatencySec;
    double currentSampleRate;
    
    bool isBypassed;
    
    // for n3d normalization multiply X,Y,Z by sqrt(3)
    static constexpr double SQRT_THREE = 1.732050807568877;
        
    // differential (z) compensation filters
    dsp::IIR::Filter<float> iirLowShelf;
    AudioBuffer<float> zFirCoeffBuffer;
    dsp::Convolution zFilterConv;
    AudioBuffer<float> coincEightXFirCoeffBuffer;
    AudioBuffer<float> coincEightYFirCoeffBuffer;
    dsp::Convolution coincXEightFilterConv;
    dsp::Convolution coincYEightFilterConv;
    AudioBuffer<float> coincOmniFirCoeffBuffer;
    dsp::Convolution coincOmniFilterConv;
    
    static constexpr float MIN_Z_GAIN_DB = -40.0f;
    static constexpr float GAIN_TO_ZERO_THRESH_DB = 1.0f;
    
    int editorWidth;
    int editorHeight;

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
    
    std::atomic<float>* legacyModePtr;
    // AB Layer handling
    Identifier nodeA = "layerA";
    Identifier nodeB = "layerB";
    Identifier allStates = "savedLayers";
    ValueTree layerA;
    ValueTree layerB;
    ValueTree allValueTreeStates;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmbiCreatorAudioProcessor)
};
