#pragma once

#include "juce_dsp/juce_dsp.h"
#include <juce_audio_processors/juce_audio_processors.h>

enum eCurrentActiveLayer
{
    layerA = 1,
    layerB = 2
};

//==============================================================================
/**
*/
class AmbiCreatorAudioProcessor final : public juce::AudioProcessor,
                                        public juce::AudioProcessorValueTreeState::Listener
{
public:
    static constexpr float MIN_Z_GAIN_DB = -40.0f;
    static constexpr float GAIN_TO_ZERO_THRESH_DB = 1.0f;

    enum eChannelOrder
    {
        ACN = 0, // ACN implies AmbiX, SN3D normalization
        FUMA = 1 // N3D normalization
    };

    //==============================================================================
    AmbiCreatorAudioProcessor();
    ~AmbiCreatorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    int getEditorWidth() { return editorWidth; }
    //    void setEditorWidth(int width) {editorWidth = width;}
    int getEditorHeight() { return editorHeight; }
    //    void setEditorHeight(int height) {editorHeight = height;}

    bool isLegacyModeActive() { return legacyMode->load() > 0.5; }
    void setAbLayer (int desiredLayer);
    void changeAbLayerState();

    juce::Atomic<bool> wrongBusConfiguration = false;
    juce::Atomic<bool> channelActive[4] = { true, true, true, true };
    juce::Atomic<bool> isPlaying = false;
    juce::Atomic<float> inRms[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    juce::Atomic<float> outRms[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    static const int EDITOR_DEFAULT_WIDTH = 650;
    static const int EDITOR_DEFAULT_HEIGHT = 500;

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState params;

    juce::AudioBuffer<float> foaChannelBuffer;
    juce::AudioBuffer<float> rotatorBuffer;
    juce::AudioBuffer<float> legacyModeReorderBuffer;

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

    // for n3d normalization multiply X,Y,Z by sqrt(3)
    static constexpr double SQRT_THREE = 1.732050807568877;

    // differential (z) compensation filters
    juce::dsp::IIR::Filter<float> iirLowShelf;
    juce::AudioBuffer<float> zFirCoeffBuffer;
    juce::dsp::Convolution zFilterConv;
    juce::AudioBuffer<float> coincEightXFirCoeffBuffer;
    juce::AudioBuffer<float> coincEightYFirCoeffBuffer;
    juce::dsp::Convolution coincXEightFilterConv;
    juce::dsp::Convolution coincYEightFilterConv;
    juce::AudioBuffer<float> coincOmniFirCoeffBuffer;
    juce::dsp::Convolution coincOmniFilterConv;

    int editorWidth;
    int editorHeight;

    enum eChannelOrderACN
    {
        W = 0,
        Y = 1,
        Z = 2,
        X = 3
    };

    void setLowShelfCoefficients (double sampleRate);
    void ambiRotateAroundZ (juce::AudioBuffer<float>* ambiBuffer);
    void updateLatency();

    std::atomic<float>* legacyMode;
    // AB Layer handling
    juce::Identifier nodeA = "layerA";
    juce::Identifier nodeB = "layerB";
    juce::Identifier allStates = "savedLayers";
    juce::ValueTree layerA;
    juce::ValueTree layerB;
    juce::ValueTree allValueTreeStates;
    int abLayerState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmbiCreatorAudioProcessor)
};
