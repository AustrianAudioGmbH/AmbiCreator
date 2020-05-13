#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"

//==============================================================================
AafoaCreatorAudioProcessorEditor::AafoaCreatorAudioProcessorEditor (AafoaCreatorAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);
    setLookAndFeel (&globalLaF);
    
    addAndMakeVisible (&title);
    title.setTitle (String("AustrianAudio"),String("FOACreator"));
    title.setFont (globalLaF.avenirMedium,globalLaF.avenirRegular);
    title.showAlertSymbol(false);
    
    addAndMakeVisible (&footer);
    tooltipWindow.setLookAndFeel (&globalLaF);
    tooltipWindow.setMillisecondsBeforeTipAppears(500);
    
    arrayImage = ImageCache::getFromMemory (arrayPng, arrayPngSize);
    
    // add labels
    addAndMakeVisible (&lbSlOutGain);
    lbSlOutGain.setText("output gain");
    
    addAndMakeVisible (&lbSlZGain);
    lbSlZGain.setText("z gain");
    
    addAndMakeVisible (&lbSlHorizontalRotation);
    lbSlHorizontalRotation.setText("horizontal rotation");
    
    // add sliders
    const int slTbWidth = 60;
    const int slTbHeight = 15;
    
    addAndMakeVisible (&slOutGain);
    slAttOutGain.reset(new ReverseSlider::SliderAttachment (valueTreeState, "outGainDb", slOutGain));
    slOutGain.setSliderStyle (Slider::LinearHorizontal);
    slOutGain.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    slOutGain.setColour (Slider::thumbColourId, Colours::black);
    slOutGain.setTextBoxStyle (Slider::TextBoxBelow, false, slTbWidth, slTbHeight);
    slOutGain.addListener (this);
    
    addAndMakeVisible (&slHorizontalRotation);
    slAttHorizontalRotation.reset(new ReverseSlider::SliderAttachment (valueTreeState, "horRotation", slHorizontalRotation));
    slHorizontalRotation.setSliderStyle (Slider::LinearHorizontal);
    slHorizontalRotation.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    slHorizontalRotation.setColour (Slider::thumbColourId, Colours::black);
    slHorizontalRotation.setTextBoxStyle (Slider::TextBoxBelow, false, slTbWidth, slTbHeight);
    slHorizontalRotation.addListener (this);
    
    addAndMakeVisible (&slZGain);
    slAttZGain.reset(new ReverseSlider::SliderAttachment (valueTreeState, "zGainDb", slZGain));
    slZGain.setSliderStyle (Slider::LinearHorizontal);
    slZGain.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    slZGain.setColour (Slider::thumbColourId, Colours::black);
    slZGain.setTextBoxStyle (Slider::TextBoxBelow, false, slTbWidth, slTbHeight);
    slZGain.addListener (this);
    
    startTimer (30);
}

AafoaCreatorAudioProcessorEditor::~AafoaCreatorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void AafoaCreatorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
    g.drawImage(arrayImage, arrayImageArea, RectanglePlacement::centred);
}

void AafoaCreatorAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    const int linearSliderWidth = 130;
//    const int linearSliderHorizontalSpacing = 32;
    const int linearSliderVerticalSpacing = 20;
    const int linearSliderHeight = 40;
    const int labelHeight = 20;
    
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom(footerHeight));
    footer.setBounds (footerArea);
    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    title.setTitleCentreX (headerArea.getCentreX());
    title.setBounds (headerArea);
    title.setAlertMessage("Wrong Bus Configuration!", "Make sure to use a four-channel track configuration such 1st Order Ambisonics, Quadraphonics or LRCS");
    
    arrayImageArea = area.removeFromLeft(200).toFloat();
    
    // -------- MAIN AREA ---------
    Rectangle<int> sliderArea (area.removeFromLeft(linearSliderWidth));
    sliderArea.removeFromTop(linearSliderVerticalSpacing);
    lbSlOutGain.setBounds(sliderArea.removeFromTop(labelHeight));
    slOutGain.setBounds(sliderArea.removeFromTop(linearSliderHeight));
    
    sliderArea.removeFromTop(linearSliderVerticalSpacing);
    lbSlZGain.setBounds(sliderArea.removeFromTop(labelHeight));
    slZGain.setBounds(sliderArea.removeFromTop(linearSliderHeight));
    
    sliderArea.removeFromTop(linearSliderVerticalSpacing);
    lbSlHorizontalRotation.setBounds(sliderArea.removeFromTop(labelHeight));
    slHorizontalRotation.setBounds(sliderArea.removeFromTop(linearSliderHeight));
}

void AafoaCreatorAudioProcessorEditor::sliderValueChanged (Slider* slider) {
    
}

void AafoaCreatorAudioProcessorEditor::buttonClicked (Button* button) {
    
}

void AafoaCreatorAudioProcessorEditor::comboBoxChanged (ComboBox* cb) {
    
}

void AafoaCreatorAudioProcessorEditor::timerCallback()
{
    if (processor.wrongBusConfiguration.get() != title.isAlerting())
    {
        title.showAlertSymbol(processor.wrongBusConfiguration.get());
    }
}
