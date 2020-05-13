#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"

//==============================================================================
AmbiCreatorAudioProcessorEditor::AmbiCreatorAudioProcessorEditor (AmbiCreatorAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);
    setLookAndFeel (&globalLaF);
    
    addAndMakeVisible (&title);
    title.setTitle (String("AustrianAudio"),String("AmbiCreator"));
    title.setFont (globalLaF.avenirMedium,globalLaF.avenirRegular);
    title.showAlertSymbol(false);
    title.setAlertMessage(wrongBusConfigMessageShort, wrongBusConfigMessageLong);
    cbAttOutChOrder.reset(new ComboBoxAttachment (valueTreeState, "channelOrder", *title.getOutputWidgetPtr()->getCbOutChOrder()));
    title.getOutputWidgetPtr()->getCbOutChOrder()->addListener(this);
    
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
    
    for (int i = 0; i < 4; ++i)
    {
        addAndMakeVisible(&inputMeter[i]);
        inputMeter[i].setColour(globalLaF.AARed);
        inputMeter[i].setLabelText(inMeterLabelText[i]);
        
        addAndMakeVisible(&outputMeter[i]);
        outputMeter[i].setColour(globalLaF.AARed);
    }
    updateOutputMeterLabelTexts();
    
    startTimer (100);
}

AmbiCreatorAudioProcessorEditor::~AmbiCreatorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void AmbiCreatorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
    g.drawImage(arrayImage, arrayImageArea, RectanglePlacement::centred);
}

void AmbiCreatorAudioProcessorEditor::resized()
{
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    const int linearSliderWidth = 130;
//    const int linearSliderHorizontalSpacing = 32;
    const int linearSliderVerticalSpacing = 20;
    const int linearSliderHeight = 40;
    const int labelHeight = 20;
    const int meterWidth = 10;
    const int meterHeight = 120;
    const int meterSpacing = 2;
    const int meterToSliderSpacing = 20;
    
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom(footerHeight));
    footer.setBounds (footerArea);
    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    title.setBounds (headerArea);
    
    arrayImageArea = area.removeFromLeft(200).toFloat();
    
    // -------- MAIN AREA ---------
    const int contentWidth = 8 * meterWidth + 2 * meterToSliderSpacing + 8 * meterSpacing + linearSliderWidth;
    const int mainMarginLeft = (area.getWidth() - contentWidth) / 2;
    
    area.removeFromLeft(mainMarginLeft);
    
    Rectangle<int> inMeterArea = area.removeFromLeft(4 * meterWidth + 4 * meterSpacing).withHeight(meterHeight);
    inMeterArea = inMeterArea.withCentre(Point<int> (inMeterArea.getCentreX(), int(area.getHeight() / 2)));
    
    for (int i = 0; i < 4; ++i)
    {
        inputMeter[i].setBounds(inMeterArea.removeFromLeft(meterWidth));
        inMeterArea.removeFromLeft(meterSpacing);
    }
    area.removeFromLeft(meterToSliderSpacing);
    
    Rectangle<int> sliderArea = area.removeFromLeft(linearSliderWidth).withHeight(3 * linearSliderVerticalSpacing + 3 * labelHeight + 3 * linearSliderHeight);
    sliderArea = sliderArea.withCentre(Point<int> (sliderArea.getCentreX(), int(area.getHeight() / 2)));
    
    sliderArea.removeFromTop(linearSliderVerticalSpacing);
    lbSlOutGain.setBounds(sliderArea.removeFromTop(labelHeight));
    slOutGain.setBounds(sliderArea.removeFromTop(linearSliderHeight));
    
    sliderArea.removeFromTop(linearSliderVerticalSpacing);
    lbSlZGain.setBounds(sliderArea.removeFromTop(labelHeight));
    slZGain.setBounds(sliderArea.removeFromTop(linearSliderHeight));
    
    sliderArea.removeFromTop(linearSliderVerticalSpacing);
    lbSlHorizontalRotation.setBounds(sliderArea.removeFromTop(labelHeight));
    slHorizontalRotation.setBounds(sliderArea.removeFromTop(linearSliderHeight));
    
    area.removeFromLeft(meterToSliderSpacing);
    Rectangle<int> outMeterArea = area.removeFromLeft(4 * meterWidth + 4 * meterSpacing).withHeight(meterHeight);
    outMeterArea = outMeterArea.withCentre(Point<int> (outMeterArea.getCentreX(), int(area.getHeight() / 2)));
    
    for (int i = 0; i < 4; ++i)
    {
        outputMeter[i].setBounds(outMeterArea.removeFromLeft(meterWidth).withHeight(meterHeight));
        outMeterArea.removeFromLeft(meterSpacing);
    }
}

void AmbiCreatorAudioProcessorEditor::sliderValueChanged (Slider* slider) {
    
}

void AmbiCreatorAudioProcessorEditor::buttonClicked (Button* button) {
    
}

void AmbiCreatorAudioProcessorEditor::comboBoxChanged (ComboBox* cb) {
    if (cb == title.getOutputWidgetPtr()->getCbOutChOrder())
        updateOutputMeterLabelTexts();
}

void AmbiCreatorAudioProcessorEditor::updateOutputMeterLabelTexts()
{
    auto cb = title.getOutputWidgetPtr()->getCbOutChOrder();
    if (cb->getText() == "AmbiX")
    {
        for (int i = 0; i < 4; ++i)
            outputMeter[i].setLabelText(outMeterLabelTextACN[i]);
    }
    else if (cb->getText() == "FUMA")
    {
        for (int i = 0; i < 4; ++i)
            outputMeter[i].setLabelText(outMeterLabelTextFUMA[i]);
    }
}

void AmbiCreatorAudioProcessorEditor::timerCallback()
{
    for (int i = 0; i < 4; ++i)
    {
        inputMeter[i].setLevel(processor.inRms[i].get());
        outputMeter[i].setLevel(processor.outRms[i].get());
    }
    
    // show alert message if bus configuration is wrong, i.e. there are
    // not 4 ins and outs
    if (processor.wrongBusConfiguration.get())
    {
        title.setAlertMessage(wrongBusConfigMessageShort, wrongBusConfigMessageLong);
        title.showAlertSymbol(true);
        return;
    }
    
    // also alert if the processor is playing, but some input channels remain silent
    if (processor.isPlaying.get())
    {        
        for (auto& active : processor.channelActive)
        {
            if (!active.get())
            {
                title.setAlertMessage(inputInactiveMessageShort, inputInactiveMessageLong);
                title.showAlertSymbol(true);
                return;
            }
        }
    }
    
    title.showAlertSymbol(false);
}
