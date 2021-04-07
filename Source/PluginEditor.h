#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "../resources/lookAndFeel/AA_LaF.h"
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
    static const int EDITOR_WIDTH = 650;
    static const int EDITOR_HEIGHT = 500;
    
    AmbiCreatorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    ComponentBoundsConstrainer fixedAspectRatioConstrainer;
    
    TitleBar<AALogo, ChannelOrderIOWidget> title;
    Footer footer;
    LaF globalLaF;
    TooltipWindow tooltipWindow;
    
    Path aaLogoBgPath;
    Image arrayLegacyImage;
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
    const juce::String inMeterLabelTextLegacy[4] = { "F", "B", "L", "R" };
    const juce::String outMeterLabelTextFUMA[4] = { "W", "X", "Y", "Z" };
    const juce::String outMeterLabelTextACN[4] = { "W", "Y", "Z", "X" };
    
    void updateOutputMeterLabelTexts();
    
    void timerCallback() override;
    
    
    // Components for new AmbiCreator Layout
    void setModeDisplay (bool legacyModeActive);
    void setAbButtonAlphaFromLayerState(int layerState);
    
    const juce::String inMeterLabelText[4] = { "L", "R", "F", "B" };
    
    ComboBox cbOutChannelOrder;
    
    Slider slRotOutGain, slRotZGain;
    
    TextButton tbAbLayer[2], tbLegacyMode;
    
    SimpleLabel lbSlRotOutGain, lbSlRotZGain, lbOutConfig;
    
    std::unique_ptr<SliderAttachment> slAttRotOutGain, slAttRotZGain;
    std::unique_ptr<ComboBoxAttachment> cbAttOutChannelOrder;
    std::unique_ptr<ButtonAttachment> tbAttLegacyMode;
    
    const float titleLineX1Start = 0.0f;
    const float titleLineX1End = 0.116f;
    const float titleLineX2Start = 0.186f;
    
    SimpleLabel helpToolTip;
    
    const juce::String helpText = {"left/right are seen from the recording side. front of upper mic should point towards the source."};
    const juce::String helpTextLegacy = {"left/right are seen from the recording side. front of lower mic should point towards the source."};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmbiCreatorAudioProcessorEditor)
};
