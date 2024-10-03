#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

#include "../resources/lookAndFeel/AA_LaF.h"
#include "../resources/lookAndFeel/MainLookAndFeel.h"

#include "../resources/customComponents/TitleBar.h"
#include "../resources/customComponents/SimpleLabel.h"
#include "../resources/customComponents/MuteSoloButton.h"
#include "../resources/customComponents/ReverseSlider.h"
#include "../resources/customComponents/LevelMeter.h"



typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
/**
*/
class AmbiCreatorAudioProcessorEditor  : public AudioProcessorEditor, private Button::Listener, private ComboBox::Listener, private Slider::Listener, private Timer
{
public:
    AmbiCreatorAudioProcessorEditor (AmbiCreatorAudioProcessor&, AudioProcessorValueTreeState&);
    ~AmbiCreatorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void buttonClicked (Button* button) override;
    void comboBoxChanged (ComboBox* cb) override;
    void sliderValueChanged (Slider* slider) override;
    
    int getControlParameterIndex (Component& control) override;

private:
    AmbiCreatorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    ComponentBoundsConstrainer fixedAspectRatioConstrainer;
    
    TitleBar<AALogo, ChannelOrderIOWidget> title;
    Footer footer;
    MainLookAndFeel mainLaF;
    TooltipWindow tooltipWindow;
    
    Path aaLogoBgPath;
    Image arrayImage;
    Rectangle<float> arrayImageArea;
    
    ReverseSlider slOutGain, slHorizontalRotation, slZGain;
    std::unique_ptr<ReverseSlider::SliderAttachment> slAttOutGain, slAttHorizontalRotation, slAttZGain;
    std::unique_ptr<ComboBoxAttachment> cbAttOutChOrder;
    
    SimpleLabel lbSlOutGain, lbSlHorizontalRotation, lbSlZGain;
    
    LevelMeter inputMeter[4];
    LevelMeter outputMeter[4];
    
    const juce::String wrongBusConfigMessageShort = "Wrong Bus Configuration!";
    const juce::String wrongBusConfigMessageLong = "Make sure to use a four-channel track configuration such 1st Order Ambisonics, Quadraphonics or LRCS.";
    const juce::String inputInactiveMessageShort = "No four-channel input detected!";
    const juce::String inputInactiveMessageLong = "Make sure to have an active input signal on all four input channels.";
    const juce::String inMeterLabelText[4] = { "F", "B", "L", "R" };
    const juce::String outMeterLabelTextFUMA[4] = { "W", "X", "Y", "Z" };
    const juce::String outMeterLabelTextACN[4] = { "W", "Y", "Z", "X" };
    
    void updateOutputMeterLabelTexts();
    
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmbiCreatorAudioProcessorEditor)
};
