#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "../resources/lookAndFeel/AA_LaF.h"
#include "../resources/customComponents/TitleBar.h"
#include "../resources/customComponents/SimpleLabel.h"
#include "../resources/customComponents/MuteSoloButton.h"
#include "../resources/customComponents/ReverseSlider.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
/**
*/
class AafoaCreatorAudioProcessorEditor  : public AudioProcessorEditor, private Button::Listener, private ComboBox::Listener, private Slider::Listener, private Timer
{
public:
    AafoaCreatorAudioProcessorEditor (AafoaCreatorAudioProcessor&, AudioProcessorValueTreeState&);
    ~AafoaCreatorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void buttonClicked (Button* button) override;
    void comboBoxChanged (ComboBox* cb) override;
    void sliderValueChanged (Slider* slider) override;

private:
    AafoaCreatorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    static const int EDITOR_WIDTH = 900;
    static const int EDITOR_HEIGHT = 700;
    
    TitleBar<AALogo, NoIOWidget> title;
    Footer footer;
    LaF globalLaF;
    TooltipWindow tooltipWindow;
    
    Image arrayImage;
    Rectangle<float> arrayImageArea;
    
    ReverseSlider slOutGain, slHorizontalRotation, slZGain;
    std::unique_ptr<ReverseSlider::SliderAttachment> slAttOutGain, slAttHorizontalRotation, slAttZGain;
    
    SimpleLabel lbSlOutGain, lbSlHorizontalRotation, lbSlZGain;
    
    const juce::String wrongBusConfigMessageShort = "Wrong Bus Configuration!";
    const juce::String wrongBusConfigMessageLong = "Make sure to use a four-channel track configuration such 1st Order Ambisonics, Quadraphonics or LRCS.";
    const juce::String inputInactiveMessageShort = "No four-channel input detected!";
    const juce::String inputInactiveMessageLong = "Make sure to have an active input signal on all four input channels.";
    
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AafoaCreatorAudioProcessorEditor)
};
