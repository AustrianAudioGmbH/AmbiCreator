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
class AafoaCreatorAudioProcessorEditor  : public AudioProcessorEditor, private Button::Listener, private ComboBox::Listener, private Slider::Listener
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
    
    Image arrayImage;
    Rectangle<float> arrayImageArea;
    
    ReverseSlider slOutGain, slHorizontalRotation, slZGain;
    std::unique_ptr<ReverseSlider::SliderAttachment> slAttOutGain, slAttHorizontalRotation, slAttZGain;
    
    SimpleLabel lbSlOutGain, lbSlHorizontalRotation, lbSlZGain;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AafoaCreatorAudioProcessorEditor)
};
