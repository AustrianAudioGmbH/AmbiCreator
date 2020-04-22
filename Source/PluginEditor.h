#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "../resources/lookAndFeel/AA_LaF.h"
#include "../resources/customComponents/TitleBar.h"
#include "../resources/customComponents/SimpleLabel.h"
#include "../resources/customComponents/MuteSoloButton.h"
#include "../resources/customComponents/ReverseSlider.h"

//==============================================================================
/**
*/
class AafoaCreatorAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    AafoaCreatorAudioProcessorEditor (AafoaCreatorAudioProcessor&);
    ~AafoaCreatorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    AafoaCreatorAudioProcessor& processor;
    
    static const int EDITOR_WIDTH = 900;
    static const int EDITOR_HEIGHT = 700;
    
    TitleBar<AALogo, NoIOWidget> title;
    Footer footer;
    LaF globalLaF;
    
    Image arrayImage;
    Rectangle<float> arrayImageArea;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AafoaCreatorAudioProcessorEditor)
};
