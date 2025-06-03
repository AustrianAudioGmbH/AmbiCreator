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
#include "../resources/customComponents/MultiTextButton.h"

#ifdef USE_MELATONIN_INSPECTOR
#include <melatonin_inspector/melatonin_inspector.h>
#endif

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

enum eChannelOrder
{
    ACN = 0, // ACN implies AmbiX, SN3D normalization
    FUMA = 1 // N3D normalization
};

//==============================================================================
/**
*/
class AmbiCreatorAudioProcessorEditor  :    public AudioProcessorEditor,
                                            private Button::Listener,
                                            private ComboBox::Listener,
                                            private Slider::Listener,
                                            private Timer,
                                            private AudioProcessorValueTreeState::Listener
{
public:
    AmbiCreatorAudioProcessorEditor (AmbiCreatorAudioProcessor&, AudioProcessorValueTreeState&);
    ~AmbiCreatorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void buttonClicked (Button* button) override;
    void mouseUp(const juce::MouseEvent& event) override;

    void comboBoxChanged (ComboBox* cb) override;
    void sliderValueChanged (Slider* slider) override;
    
    int getControlParameterIndex (Component& control) override;

    void parameterChanged(const String& parameterID, float newValue) override;

private:
    static const int EDITOR_WIDTH = 650;
    static const int EDITOR_HEIGHT = 490;
    
    AmbiCreatorAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    ComponentBoundsConstrainer fixedAspectRatioConstrainer;
    
    TitleBar<AALogo, ChannelOrderIOWidget> title;
    Footer footer;

    MainLookAndFeel ambiCreatorLookAndFeel;

    TooltipWindow tooltipWindow;

    Path aaLogoBgPath;

    Image legacyModeImage;
    Image fourChannelModeImage;
    Rectangle<float> arrayImageArea;

    GroupComponent outGainGroup, horizontalRotationGroup, zGainGroup;
    ReverseSlider outGainSlider, horizontalRotationSlider, zGainSlider;
    std::unique_ptr<ReverseSlider::SliderAttachment> outGainAttachment, horizontalRotationAttachment, zGainAttachment;

    std::unique_ptr<ButtonAttachment> cbAttOutChOrder;

    // !J! Not needed with Groups:
//    SimpleLabel outGainLabel, horizontalRotationLabel, zGainLabel;
    
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

#ifdef AA_CONFIG_COMBOBOX
    ComboBox cbOutChannelOrder;
#endif
    // !J! comboboxx is replaced with texmultibutton functioning as a radiobutton
    TextMultiButton tmbOutChannelOrder;

    TextButton tbAbLayer[2], tbLegacyMode;

#ifdef AA_CONFIG_ROTARY_UI 0
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

    const juce::String helpText = {"left/right are seen from the recording side. front of upper mic should point towards the source."};
    const juce::String helpTextLegacy = {"left/right are seen from the recording side. front of lower mic should point towards the source."};

#ifdef USE_MELATONIN_INSPECTOR
#warning "MELATONIN INSPECTOR IS CONFIGURED TO BE INCLUDED IN BUILD"
    melatonin::Inspector inspector { *this, true };
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmbiCreatorAudioProcessorEditor)
};
