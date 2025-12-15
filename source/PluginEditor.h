#pragma once

#include "../resources/customComponents/LevelMeter.h"
#include "../resources/customComponents/ReverseSlider.h"
#include "../resources/customComponents/SimpleLabel.h"
#include "../resources/customComponents/TitleBar.h"
#include "../resources/lookAndFeel/AA_LaF.h"
#include "PluginProcessor.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
/**
*/
class AmbiCreatorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Button::Listener,
                                        private juce::ComboBox::Listener,
                                        private juce::Slider::Listener,
                                        private juce::Timer
{
public:
    AmbiCreatorAudioProcessorEditor (AmbiCreatorAudioProcessor&,
                                     juce::AudioProcessorValueTreeState&);
    ~AmbiCreatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (juce::Button* button) override;
    void comboBoxChanged (juce::ComboBox* cb) override;
    void sliderValueChanged (juce::Slider* slider) override;

    int getControlParameterIndex (Component& control) override;

private:
    static const int EDITOR_WIDTH = 650;
    static const int EDITOR_HEIGHT = 500;

    AmbiCreatorAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::ComponentBoundsConstrainer fixedAspectRatioConstrainer;

    TitleBar<AALogo, ChannelOrderIOWidget> title;
    Footer footer;
    LaF globalLaF;
    juce::TooltipWindow tooltipWindow;

    juce::Path aaLogoBgPath;
    juce::Image arrayLegacyImage;
    juce::Image arrayImage;
    juce::Rectangle<float> arrayImageArea;

    ReverseSlider slOutGain, slHorizontalRotation, slZGain;
    std::unique_ptr<ReverseSlider::SliderAttachment> slAttOutGain, slAttHorizontalRotation,
        slAttZGain;
    std::unique_ptr<ComboBoxAttachment> cbAttOutChOrder;

    SimpleLabel lbSlOutGain, lbSlHorizontalRotation, lbSlZGain;

    LevelMeter inputMeter[4];
    LevelMeter outputMeter[4];

    const juce::String wrongBusConfigMessageShort = "Wrong Bus Configuration!";
    const juce::String wrongBusConfigMessageLong =
        "Make sure to use a four-channel track configuration such 1st Order Ambisonics, Quadraphonics or LRCS.";
    const juce::String inputInactiveMessageShort = "No four-channel input detected!";
    const juce::String inputInactiveMessageLong =
        "Make sure to have an active input signal on all four input channels.";
    const juce::String inMeterLabelTextLegacy[4] = { "F", "B", "L", "R" };
    const juce::String outMeterLabelTextFUMA[4] = { "W", "X", "Y", "Z" };
    const juce::String outMeterLabelTextACN[4] = { "W", "Y", "Z", "X" };

    void updateOutputMeterLabelTexts();

    void timerCallback() override;

    // Components for new AmbiCreator Layout
    void setModeDisplay (bool legacyModeActive);
    void setAbButtonAlphaFromLayerState (int layerState);

    const juce::String inMeterLabelText[4] = { "L", "R", "F", "B" };

    juce::ComboBox cbOutChannelOrder;

    juce::Slider slRotOutGain, slRotZGain;

    juce::TextButton tbAbLayer[2], tbLegacyMode;

    SimpleLabel lbSlRotOutGain, lbSlRotZGain, lbOutConfig;

    std::unique_ptr<SliderAttachment> slAttRotOutGain, slAttRotZGain;
    std::unique_ptr<ComboBoxAttachment> cbAttOutChannelOrder;
    std::unique_ptr<ButtonAttachment> tbAttLegacyMode;

    SimpleLabel helpToolTip;

    const juce::String helpText = {
        "left/right are seen from the recording side. front of upper mic should point towards the source."
    };
    const juce::String helpTextLegacy = {
        "left/right are seen from the recording side. front of lower mic should point towards the source."
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmbiCreatorAudioProcessorEditor)
};
