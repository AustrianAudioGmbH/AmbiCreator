#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"

//==============================================================================
AmbiCreatorAudioProcessorEditor::AmbiCreatorAudioProcessorEditor (AmbiCreatorAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
    setResizable (true, true);
    fixedAspectRatioConstrainer.setFixedAspectRatio (double(processor.EDITOR_DEFAULT_WIDTH) / processor.EDITOR_DEFAULT_HEIGHT);
    fixedAspectRatioConstrainer.setSizeLimits (processor.EDITOR_DEFAULT_WIDTH, processor.EDITOR_DEFAULT_HEIGHT, 2 * processor.EDITOR_DEFAULT_WIDTH, 2 * processor.EDITOR_DEFAULT_HEIGHT);
    setConstrainer (&fixedAspectRatioConstrainer);
    setSize (processor.getEditorWidth(), processor.getEditorHeight());
    
    setLookAndFeel (&globalLaF);
    
    addAndMakeVisible (&title);
    title.setTitle (String("AustrianAudio"),String("AmbiCreator"));
    title.setFont (globalLaF.aaMedium,globalLaF.aaRegular);
    title.showAlertSymbol(false);
    title.setAlertMessage(wrongBusConfigMessageShort, wrongBusConfigMessageLong);
    cbAttOutChOrder.reset(new ComboBoxAttachment (valueTreeState, "channelOrder", *title.getOutputWidgetPtr()->getCbOutChOrder()));
    title.getOutputWidgetPtr()->getCbOutChOrder()->addListener(this);
    
    addAndMakeVisible (&footer);
    tooltipWindow.setLookAndFeel (&globalLaF);
    tooltipWindow.setMillisecondsBeforeTipAppears(500);
    
    arrayImage = ImageCache::getFromMemory (arrayPng, arrayPngSize);
    aaLogoBgPath.loadPathFromData (aaLogoData, sizeof (aaLogoData));
    
    // add labels
    addAndMakeVisible (&lbSlOutGain);
    lbSlOutGain.setText("Output Gain");
    
    addAndMakeVisible (&lbSlZGain);
    lbSlZGain.setText("Z Gain");
    
    addAndMakeVisible (&lbSlHorizontalRotation);
    lbSlHorizontalRotation.setText("Horizontal Rotation");
    
    // add sliders
    addAndMakeVisible (&slOutGain);
    slAttOutGain.reset(new ReverseSlider::SliderAttachment (valueTreeState, "outGainDb", slOutGain));
    slOutGain.setSliderStyle (Slider::LinearHorizontal);
    slOutGain.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    slOutGain.setColour (Slider::thumbColourId, globalLaF.AARed);
    slOutGain.addListener (this);
    
    addAndMakeVisible (&slHorizontalRotation);
    slAttHorizontalRotation.reset(new ReverseSlider::SliderAttachment (valueTreeState, "horRotation", slHorizontalRotation));
    slHorizontalRotation.setSliderStyle (Slider::LinearHorizontal);
    slHorizontalRotation.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    slHorizontalRotation.setColour (Slider::thumbColourId, globalLaF.AARed);
    slHorizontalRotation.addListener (this);
    
    addAndMakeVisible (&slZGain);
    slAttZGain.reset(new ReverseSlider::SliderAttachment (valueTreeState, "zGainDb", slZGain));
    slZGain.setSliderStyle (Slider::LinearHorizontal);
    slZGain.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    slZGain.setColour (Slider::thumbColourId, globalLaF.AARed);
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
    const int currHeight = getHeight();
    const int currWidth = getWidth();
    
    g.fillAll (globalLaF.ClBackground);
    // g.drawImage(arrayImage, arrayImageArea, RectanglePlacement::centred);
    g.drawImage(arrayImage, -40, 0, arrayImageArea.getWidth() + 100, currHeight + 40, 0, 0, arrayImage.getWidth(), arrayImage.getHeight());
    
    // background logo
    aaLogoBgPath.applyTransform (aaLogoBgPath.getTransformToScaleToFit (0.4f * currWidth, 0.25f * currHeight,
                                                                        0.7f * currWidth, 0.7f * currWidth, true, Justification::centred));
    g.setColour (Colours::white.withAlpha(0.1f));
    g.strokePath (aaLogoBgPath, PathStrokeType (0.1f));
    g.fillPath (aaLogoBgPath);
}

void AmbiCreatorAudioProcessorEditor::resized()
{
    const float currentWidth = getWidth();
    const float currentHeight = getHeight();
    processor.setEditorWidth(currentWidth);
    processor.setEditorHeight(currentHeight);
    
    const float leftRightMargin = 0.046f * currentWidth;
    const float topMargin = 0.01 * currentHeight;
    const float headerHeight = 0.13f * currentHeight;
    const float footerHeight = 0.05f * currentHeight;
    const float linearSliderWidth = 0.246f * currentWidth;
    const float linearSliderVerticalSpacing = 0.08f * currentHeight;
    const float linearSliderHeight = 0.08f * currentHeight;
    const float labelHeight = 0.03f * currentHeight;
    const float meterWidth = 0.023f * currentWidth;
    const float meterHeight = 0.32f * currentHeight;
    const float meterSpacing = 0.003f * currentWidth;
    const float meterToSliderSpacing = 0.046f * currentWidth;
    const float arrayWidth = 0.308f * currentWidth;
    const float slTbWidth = 0.092f * currentWidth;
    const float slTbHeight = 0.03f * currentHeight;
    
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom(footerHeight));
    footer.setBounds (footerArea);
    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    title.setBounds (headerArea);
//    title.toFront(false);
    
    area.removeFromTop(topMargin);
    arrayImageArea = area.removeFromLeft(arrayWidth).toFloat();
    
    // -------- MAIN AREA ---------
    const float contentWidth = 8 * meterWidth + 2 * meterToSliderSpacing + 8 * meterSpacing + linearSliderWidth;
    const float mainMarginLeft = (area.getWidth() - contentWidth) / 2;
    
    area.removeFromLeft(mainMarginLeft);
    
    Rectangle<int> inMeterArea = area.removeFromLeft(4 * meterWidth + 4 * meterSpacing).withHeight(meterHeight);
    inMeterArea = inMeterArea.withCentre(Point<int> (inMeterArea.getCentreX(), int(area.getHeight() * 0.6f)));
    
    for (int i = 0; i < 4; ++i)
    {
        inputMeter[i].setBounds(inMeterArea.removeFromLeft(meterWidth));
        inMeterArea.removeFromLeft(meterSpacing);
    }
    area.removeFromLeft(meterToSliderSpacing);
    
    Rectangle<int> sliderArea = area.removeFromLeft(linearSliderWidth).withHeight(3 * linearSliderVerticalSpacing + 3 * labelHeight + 3 * linearSliderHeight);
    sliderArea = sliderArea.withCentre(Point<int> (sliderArea.getCentreX(), int(area.getHeight() * 0.55f)));
    
    sliderArea.removeFromTop(linearSliderVerticalSpacing);
    lbSlOutGain.setBounds(sliderArea.removeFromTop(labelHeight));
    slOutGain.setTextBoxStyle (Slider::TextBoxBelow, false, slTbWidth, slTbHeight);
    slOutGain.setBounds(sliderArea.removeFromTop(linearSliderHeight));
    
    sliderArea.removeFromTop(linearSliderVerticalSpacing);
    lbSlZGain.setBounds(sliderArea.removeFromTop(labelHeight));
    slZGain.setTextBoxStyle (Slider::TextBoxBelow, false, slTbWidth, slTbHeight);
    slZGain.setBounds(sliderArea.removeFromTop(linearSliderHeight));
    
    sliderArea.removeFromTop(linearSliderVerticalSpacing);
    lbSlHorizontalRotation.setBounds(sliderArea.removeFromTop(labelHeight));
    slHorizontalRotation.setTextBoxStyle (Slider::TextBoxBelow, false, slTbWidth, slTbHeight);
    slHorizontalRotation.setBounds(sliderArea.removeFromTop(linearSliderHeight));
    
    area.removeFromLeft(meterToSliderSpacing);
    Rectangle<int> outMeterArea = area.removeFromLeft(4 * meterWidth + 4 * meterSpacing).withHeight(meterHeight);
    outMeterArea = outMeterArea.withCentre(Point<int> (outMeterArea.getCentreX(), int(area.getHeight() * 0.6f)));
    
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

// implement this for AAX automation shortchut
int AmbiCreatorAudioProcessorEditor::getControlParameterIndex (Component& control)
{
    if (&control == &slOutGain)
        return 1;
    else if (&control == &slHorizontalRotation)
        return 2;
    else if (&control == &slZGain)
        return 3;
    
    return -1;
}
