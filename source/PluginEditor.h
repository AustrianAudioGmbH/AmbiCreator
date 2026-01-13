#pragma once

#include "../resources/customComponents/ABComponent.hpp"
#include "../resources/customComponents/Footer.hpp"
#include "../resources/customComponents/LevelMeter.h"
#include "../resources/customComponents/Logo.hpp"
#include "../resources/customComponents/RotarySlider.hpp"
#include "../resources/customComponents/RotarySliderGroup.hpp"
#include "../resources/customComponents/SimpleLabel.h"
#include "PluginProcessor.h"
#include "uiComponents/OutputConfig.hpp"

#include <juce_audio_processors/juce_audio_processors.h>

#ifdef USE_MELATONIN_INSPECTOR
    #include <melatonin_inspector/melatonin_inspector.h>
#endif

using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

enum eChannelOrder
{
    ACN = 1, // ACN implies AmbiX, SN3D normalization
    FUMA = 2 // N3D normalization
};

//==============================================================================
/**
*/
class AmbiCreatorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Button::Listener,
                                        private juce::ComboBox::Listener,
                                        private juce::Slider::Listener,
                                        private juce::Timer,
                                        private juce::AudioProcessorValueTreeState::Listener
{
public:
    AmbiCreatorAudioProcessorEditor (AmbiCreatorAudioProcessor&,
                                     juce::AudioProcessorValueTreeState&);

    ~AmbiCreatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (juce::Button* button) override;
    void mouseUp (const juce::MouseEvent& event) override;

    void comboBoxChanged (juce::ComboBox* cb) override;
    void sliderValueChanged (juce::Slider* slider) override;

    int getControlParameterIndex (Component& control) override;

    void parameterChanged (const juce::String& parameterID, float newValue) override;

private:
    static const int EDITOR_WIDTH = 650;
    static const int EDITOR_HEIGHT = 490;

    AmbiCreatorAudioProcessor& ambiCreatorProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    AAGuiComponents::AALogo logo;
    AAGuiComponents::ABComponent abComponent;

    AAGuiComponents::Footer footer;

    juce::TooltipWindow tooltipWindow;

    juce::Path aaLogoBgPath;

    juce::Image legacyModeImage;
    juce::Image fourChannelModeImage;
    juce::Rectangle<int> arrayImageArea;

    AAGuiComponents::RotarySliderGroup<AAGuiComponents::RotarySlider> outGainGroup,
        horizontalRotationGroup, zGainGroup;
    // ReverseSlider outGainSlider, horizontalRotationSlider, zGainSlider;
    std::unique_ptr<SliderAttachment> outGainAttachment, horizontalRotationAttachment,
        zGainAttachment;

    // !J! Not needed with Groups:
    //    SimpleLabel outGainLabel, horizontalRotationLabel, zGainLabel;

    AAGuiComponents::LevelMeter inputMeter[4];
    AAGuiComponents::LevelMeter outputMeter[4];

    const juce::String wrongBusConfigMessageShort = "Wrong Bus Configuration!";
    const juce::String wrongBusConfigMessageLong =
        "Make sure to use a four-channel track configuration such 1st Order Ambisonics, Quadraphonics or LRCS.";
    const juce::String inputInactiveMessageShort = "No four-channel input detected!";
    const juce::String inputInactiveMessageLong =
        "Make sure to have an active input signal on all four input channels.";
    const juce::String inMeterLabelTextLegacy[4] = { "F", "B", "L", "R" };
    const juce::String inMeterLabelText[4] = { "L", "R", "F", "B" };
    const juce::String outMeterLabelTextFUMA[4] = { "W", "X", "Y", "Z" };
    const juce::String outMeterLabelTextACN[4] = { "W", "Y", "Z", "X" };

    void updateOutputMeterLabelTexts();

    void timerCallback() override;

    // Components for new AmbiCreator Layout
    void setModeDisplay (bool legacyModeActive);

    AAGuiComponents::OutputConfig outputConfigGroup;
    AAGuiComponents::TextButton<AAGuiComponents::ButtonColor::red> tbLegacyMode;

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

#ifdef USE_MELATONIN_INSPECTOR
    #warning "MELATONIN INSPECTOR IS CONFIGURED TO BE INCLUDED IN BUILD"
    melatonin::Inspector inspector { *this, true };
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmbiCreatorAudioProcessorEditor)
};
