#pragma once

#include "../resources/customComponents/LevelMeter.h"
#include "../resources/customComponents/MultiTextButton.h"
#include "../resources/customComponents/ReverseSlider.h"
#include "../resources/customComponents/SimpleLabel.h"
#include "../resources/customComponents/TitleBar.h"
#include "../resources/lookAndFeel/MainLookAndFeel.h"

#include "PluginProcessor.h"

#include <juce_audio_processors/juce_audio_processors.h>

#ifdef USE_MELATONIN_INSPECTOR
    #include <melatonin_inspector/melatonin_inspector.h>
#endif

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

enum eChannelOrder
{
    ACN = 0, // ACN implies AmbiX, SN3D normalization
    FUMA = 1 // N3D normalization
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

    juce::ComponentBoundsConstrainer fixedAspectRatioConstrainer;

    TitleBar<AALogo, ChannelOrderIOWidget> title;
    Footer footer;

    MainLookAndFeel ambiCreatorLookAndFeel;

    juce::TooltipWindow tooltipWindow;

    juce::Path aaLogoBgPath;

    juce::Image legacyModeImage;
    juce::Image fourChannelModeImage;
    juce::Rectangle<float> arrayImageArea;

    juce::GroupComponent outGainGroup, horizontalRotationGroup, zGainGroup;
    ReverseSlider outGainSlider, horizontalRotationSlider, zGainSlider;
    std::unique_ptr<ReverseSlider::SliderAttachment> outGainAttachment,
        horizontalRotationAttachment, zGainAttachment;

    std::unique_ptr<ButtonAttachment> cbAttOutChOrder;

    // !J! Not needed with Groups:
    //    SimpleLabel outGainLabel, horizontalRotationLabel, zGainLabel;

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

#ifdef AA_CONFIG_COMBOBOX
    ComboBox cbOutChannelOrder;
#endif
    // !J! comboboxx is replaced with texmultibutton functioning as a radiobutton
    TextMultiButton tmbOutChannelOrder;

    juce::TextButton tbAbLayer[2], tbLegacyMode;

#ifdef AA_CONFIG_ROTARY_UI
    Slider slRotOutGain, slRotZGain;
    SimpleLabel lbSlRotOutGain, lbSlRotZGain;
#endif
    SimpleLabel outputConfigLabel;

    std::unique_ptr<SliderAttachment> slAttRotOutGain, slAttRotZGain;
    std::unique_ptr<ComboBoxAttachment> cbAttOutChannelOrder;
    std::unique_ptr<ButtonAttachment> tbAttLegacyMode;

    const float titleLineX1Start = 0.0f;
    const float titleLineX1End = 0.116f;
    const float titleLineX2Start = 0.186f;

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
