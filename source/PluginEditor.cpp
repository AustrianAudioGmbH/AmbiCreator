#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"

//==============================================================================
AmbiCreatorAudioProcessorEditor::AmbiCreatorAudioProcessorEditor (AmbiCreatorAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
//    setResizable (true, true);
//    fixedAspectRatioConstrainer.setFixedAspectRatio (double(processor.EDITOR_DEFAULT_WIDTH) / processor.EDITOR_DEFAULT_HEIGHT);
//    fixedAspectRatioConstrainer.setSizeLimits (processor.EDITOR_DEFAULT_WIDTH, processor.EDITOR_DEFAULT_HEIGHT, 2 * processor.EDITOR_DEFAULT_WIDTH, 2 * processor.EDITOR_DEFAULT_HEIGHT);
//    setConstrainer (&fixedAspectRatioConstrainer);

    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);
    
    setLookAndFeel (&mainLaF);
    
    addAndMakeVisible (&title);
    title.setTitle (String("AustrianAudio"),String("AmbiCreator"));
    title.setFont (mainLaF.aaMedium, mainLaF.aaRegular);
    title.showAlertSymbol(false);
    title.setAlertMessage(wrongBusConfigMessageShort, wrongBusConfigMessageLong);
    cbAttOutChOrder.reset(new ComboBoxAttachment (valueTreeState, "channelOrder", *title.getOutputWidgetPtr()->getCbOutChOrder()));
    title.getOutputWidgetPtr()->getCbOutChOrder()->addListener(this);
    
    addAndMakeVisible (&footer);
    tooltipWindow.setLookAndFeel (&mainLaF);
    tooltipWindow.setMillisecondsBeforeTipAppears(500);

    legacyModeImageArray = ImageCache::getFromMemory (legacyPNGArray, legacyPNGArraySize);
    fourChannelModeImageArray = ImageCache::getFromMemory (fourChannelPNGArray, fourChannelPNGArraySize);

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
    slOutGain.setColour (Slider::thumbColourId, mainLaF.AARed);
    slOutGain.addListener (this);
    
    addAndMakeVisible (&slHorizontalRotation);
    slAttHorizontalRotation.reset(new ReverseSlider::SliderAttachment (valueTreeState, "horRotation", slHorizontalRotation));
    slHorizontalRotation.setSliderStyle (Slider::LinearHorizontal);
    slHorizontalRotation.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    slHorizontalRotation.setColour (Slider::thumbColourId, mainLaF.AARed);
    slHorizontalRotation.addListener (this);
    
    addAndMakeVisible (&slZGain);
    slAttZGain.reset(new ReverseSlider::SliderAttachment (valueTreeState, "zGainDb", slZGain));
    slZGain.setSliderStyle (Slider::LinearHorizontal);
    slZGain.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    slZGain.setColour (Slider::thumbColourId, mainLaF.AARed);
    slZGain.addListener (this);
    
    for (int i = 0; i < 4; ++i)
    {
        addAndMakeVisible(&inputMeter[i]);
        inputMeter[i].setColour(mainLaF.AARed);
        inputMeter[i].setLabelText(inMeterLabelText[i]);
        
        addAndMakeVisible(&outputMeter[i]);
        outputMeter[i].setColour(mainLaF.AARed);
    }
    updateOutputMeterLabelTexts();
    
    startTimer (100);

    // ------------------new AmbiCreator Layout components----------------
    // ugly but simple solution
    title.getOutputWidgetPtr()->setEnabled(false);
    title.getOutputWidgetPtr()->setVisible(false);

    addAndMakeVisible(&cbOutChannelOrder);
    cbAttOutChannelOrder.reset(new ComboBoxAttachment (valueTreeState, "channelOrder", cbOutChannelOrder));
    cbOutChannelOrder.addItem("AmbiX", 1);
    cbOutChannelOrder.addItem("FUMA", 2);
    cbOutChannelOrder.setEditableText(false);
    cbOutChannelOrder.setJustificationType (Justification::centred);
    cbOutChannelOrder.setSelectedId (1);
    cbOutChannelOrder.addListener(this);

#if 0 // !J!
    addAndMakeVisible(&slRotZGain);
    slAttRotZGain.reset(new SliderAttachment (valueTreeState, "zGainDb", slRotZGain));
//    slRotZGain.setSliderStyle(Slider::Rotary); // !J!
    slRotZGain.setColour(Slider::rotarySliderOutlineColourId, mainLaF.AARed);
    slRotZGain.setTextValueSuffix(" dB");
    slRotZGain.addListener(this);

    addAndMakeVisible(&slRotOutGain);
    slAttRotOutGain.reset(new SliderAttachment (valueTreeState, "outGainDb", slRotOutGain));
//    slRotOutGain.setSliderStyle(Slider::Rotary); // !J!
    slRotOutGain.setColour(Slider::rotarySliderOutlineColourId, mainLaF.AARed);
    slRotOutGain.setTextValueSuffix(" dB");
    slRotOutGain.addListener(this);

    addAndMakeVisible (&lbSlRotOutGain);
    lbSlRotOutGain.setText("Output Gain");

    addAndMakeVisible (&lbSlRotZGain);
    lbSlRotZGain.setText("Z Gain");
#endif

    addAndMakeVisible(&lbOutConfig);
    lbOutConfig.setText("Output Config");

    addAndMakeVisible (&tbLegacyMode);
    tbAttLegacyMode.reset(new ButtonAttachment (valueTreeState, "legacyMode", tbLegacyMode));
    tbLegacyMode.addListener(this);
    tbLegacyMode.setButtonText("legacy mode");
    tbLegacyMode.setToggleState(processor.isLegacyModeActive(), NotificationType::sendNotification);

    addAndMakeVisible(&tbAbLayer[0]);
    tbAbLayer[0].setButtonText("A");
    tbAbLayer[0].addListener(this);
    tbAbLayer[0].setClickingTogglesState(true);
    tbAbLayer[0].setRadioGroupId(1);
    tbAbLayer[0].setToggleState(true, NotificationType::dontSendNotification);

    addAndMakeVisible(&tbAbLayer[1]);
    tbAbLayer[1].setButtonText("B");
    tbAbLayer[1].addListener(this);
    tbAbLayer[1].setClickingTogglesState(true);
    tbAbLayer[1].setRadioGroupId(1);
    tbAbLayer[1].setToggleState(false, NotificationType::dontSendNotification);

    setAbButtonAlphaFromLayerState(eCurrentActiveLayer::layerA);

    // help tooltip
    addAndMakeVisible(&helpToolTip);
    helpToolTip.setText("help", NotificationType::dontSendNotification);
    helpToolTip.setTextColour(Colours::white.withAlpha(0.5f));

    setModeDisplay(processor.isLegacyModeActive());
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
    
    g.fillAll (mainLaF.ClBackground);
//    g.drawImage(fourChannelModeImageArray, arrayImageArea, RectanglePlacement::centred);
    {
        if (processor.isLegacyModeActive())
        {
            g.drawImage(legacyModeImageArray, -40, 0, (int)(arrayImageArea.getWidth() + 100.0f), currHeight + 40, 0, 0, legacyModeImageArray.getWidth(), legacyModeImageArray.getHeight());
            helpToolTip.setTooltip(helpTextLegacy);
        }
        else
        {
            g.drawImageWithin(fourChannelModeImageArray, 12, 1, fourChannelModeImageArray.getWidth() / 2, fourChannelModeImageArray.getHeight() / 2, RectanglePlacement::onlyReduceInSize);
            helpToolTip.setTooltip(helpText);
        }
    }

    // background logo
    aaLogoBgPath.applyTransform (aaLogoBgPath.getTransformToScaleToFit ((0.4f * currWidth), (0.25f * currHeight),
                                                                        (0.7f * currWidth), (0.7f * currWidth), true, Justification::centred));
    g.setColour (Colours::white.withAlpha(0.1f));
    g.strokePath (aaLogoBgPath, PathStrokeType (0.1f));
    g.fillPath (aaLogoBgPath);
}

void AmbiCreatorAudioProcessorEditor::resized()
{
    const float currentWidth = getWidth();
    const float currentHeight = getHeight();
//    processor.setEditorWidth(currentWidth);
//    processor.setEditorHeight(currentHeight);
    
    const float leftRightMargin = 0.046f * currentWidth;
    const float topMargin = 0.01f * currentHeight;
    const int headerHeight = 60;
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
    
//    const float rotarySliderWidth = 0.123f * currentWidth;
    const float rotarySliderHeight = 0.12f * currentHeight;
    const float rotarySliderVerticalSpacing = 0.05f * currentHeight;
    const float buttonHeight = 0.051f * currentHeight;
    const float legacyButtonWidth = 0.13f * currentWidth;
    const float horizontalButtonSpacing = 0.006f * currentWidth;
//    const float comboBoxWidth = 0.2f * currentWidth;
    const float comboBoxHeight = 0.038f * currentHeight;

    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom((int)footerHeight));
    helpToolTip.setBounds(5, getHeight() - 30, 40, 15);
    footer.setBounds (footerArea);
    
    area.removeFromLeft((int)leftRightMargin);
    area.removeFromRight((int)leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    title.setBounds (headerArea);
//    title.toFront(false);
    
    if (processor.isLegacyModeActive())
    {
        title.setLineBounds(false, 0.0f, (int)(0.116f * currentWidth), (int)(0.186f * currentWidth));
    }
    else
    {
        title.setLineBounds(false, 0, 65, 138);
    }

    Rectangle<int> headerButtonArea = headerArea;
    headerButtonArea.removeFromRight((int)(leftRightMargin/8.0f));
    headerButtonArea.removeFromTop((int)(headerHeight/2.0f - buttonHeight/2.0f));
    headerButtonArea.removeFromBottom((int)(headerHeight/2.0f - buttonHeight/2.0f));
    tbLegacyMode.setBounds(headerButtonArea.removeFromRight((int)legacyButtonWidth));
    headerButtonArea.removeFromRight((int)(2.0f * horizontalButtonSpacing));
    tbAbLayer[1].setBounds(headerButtonArea.removeFromRight((int)buttonHeight));
    headerButtonArea.removeFromRight((int)(2.0f * horizontalButtonSpacing));
    tbAbLayer[0].setBounds(headerButtonArea.removeFromRight((int)buttonHeight));

    area.removeFromTop((int)topMargin);
    arrayImageArea = area.removeFromLeft((int)arrayWidth).toFloat();
    
    // -------- MAIN AREA ---------
    const float contentWidth = 8 * meterWidth + 2 * meterToSliderSpacing + 8 * meterSpacing + linearSliderWidth;
    const float mainMarginLeft = ((int)area.getWidth() - contentWidth) / 2.0f;
    
    area.removeFromLeft(mainMarginLeft * 1.0f);
    
    Rectangle<int> inMeterArea = area.removeFromLeft((int)(4.0f * meterWidth + 4.0f * meterSpacing)).withHeight((int)meterHeight);
    inMeterArea = inMeterArea.withCentre(Point<int> (inMeterArea.getCentreX(), (int)(area.getHeight() * 0.6f)));
    
    for (int i = 0; i < 4; ++i)
    {
        inputMeter[i].setBounds(inMeterArea.removeFromLeft((int)meterWidth));
        inMeterArea.removeFromLeft((int)meterSpacing);
    }
    area.removeFromLeft((int)meterToSliderSpacing);
    
    Rectangle<int> sliderArea = area.removeFromLeft((int)linearSliderWidth).withHeight((int)(3.0f * linearSliderVerticalSpacing + 3.0f * labelHeight + 3.0f * linearSliderHeight));
    sliderArea = sliderArea.withCentre(Point<int> (sliderArea.getCentreX(), (int)(area.getHeight() * 0.55f)));
    
    Rectangle<int> rotSliderArea = sliderArea;

#if 0
    rotSliderArea.removeFromTop((int)rotarySliderVerticalSpacing);
    lbSlRotOutGain.setBounds(rotSliderArea.removeFromTop((int)labelHeight));
    slRotOutGain.setTextBoxStyle(Slider::TextBoxBelow, false, (int)slTbWidth, slTbHeight);
    slRotOutGain.setBounds(rotSliderArea.removeFromTop(rotarySliderHeight));

    rotSliderArea.removeFromTop(rotarySliderVerticalSpacing);
    lbSlRotZGain.setBounds(rotSliderArea.removeFromTop(labelHeight));
    slRotZGain.setTextBoxStyle(Slider::TextBoxBelow, false, slTbWidth, slTbHeight);
    slRotZGain.setBounds(rotSliderArea.removeFromTop(rotarySliderHeight));
#endif

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
    
    lbOutConfig.setBounds(outMeterArea.getX() - 7 * meterSpacing, lbSlOutGain.getY() - 12, 4 * meterWidth + 14 * meterSpacing, labelHeight);
    cbOutChannelOrder.setBounds(outMeterArea.getX() - 7 * meterSpacing, outMeterArea.getY() - 2 * comboBoxHeight, 4 * meterWidth + 14 * meterSpacing, comboBoxHeight);

    for (int i = 0; i < 4; ++i)
    {
        outputMeter[i].setBounds(outMeterArea.removeFromLeft(meterWidth).withHeight(meterHeight));
        outMeterArea.removeFromLeft(meterSpacing);
    }
}

void AmbiCreatorAudioProcessorEditor::sliderValueChanged (Slider* slider) {
    
}

void AmbiCreatorAudioProcessorEditor::buttonClicked (Button* button) {
    if (button == &tbLegacyMode)
    {
        bool isToggled = button->getToggleState();
        button->setToggleState(!isToggled, NotificationType::dontSendNotification);

        setModeDisplay(button->getToggleState());
    }
    else if (button == &tbAbLayer[0])
    {
        bool isToggled = button->getToggleState();
        if (isToggled < 0.5f)
        {
            processor.setAbLayer(eCurrentActiveLayer::layerB);
            setAbButtonAlphaFromLayerState(eCurrentActiveLayer::layerB);
        }
        tbLegacyMode.setToggleState(processor.isLegacyModeActive(), NotificationType::dontSendNotification);
        setModeDisplay(processor.isLegacyModeActive());
    }
    else if (button == &tbAbLayer[1])
    {
        bool isToggled = button->getToggleState();
        if (isToggled < 0.5f)
        {
            processor.setAbLayer(eCurrentActiveLayer::layerA);
            setAbButtonAlphaFromLayerState(eCurrentActiveLayer::layerA);
        }
        tbLegacyMode.setToggleState(processor.isLegacyModeActive(), NotificationType::dontSendNotification);
        setModeDisplay(processor.isLegacyModeActive());
    }
    repaint();
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

void AmbiCreatorAudioProcessorEditor::setModeDisplay(bool legacyModeActive)
{
#if 0
    slZGain.setEnabled(legacyModeActive);
    slZGain.setVisible(legacyModeActive);
    slOutGain.setEnabled(legacyModeActive);
    slOutGain.setVisible(legacyModeActive);
    lbSlOutGain.setVisible(legacyModeActive);
    lbSlZGain.setVisible(legacyModeActive);

    slRotZGain.setVisible(!legacyModeActive);
    slRotZGain.setEnabled(!legacyModeActive);
    slRotOutGain.setVisible(!legacyModeActive);
    slRotOutGain.setEnabled(!legacyModeActive);
    lbSlRotZGain.setVisible(!legacyModeActive);
    lbSlRotOutGain.setVisible(!legacyModeActive);
#endif

    if (legacyModeActive)
    {
        for (int i = 0; i < 4; ++i)
            inputMeter[i].setLabelText(inMeterLabelTextLegacy[i]);

        title.setLineBounds(false, 0.0f, 0.116f * getLocalBounds().getWidth(), 0.186f * getLocalBounds().getWidth());
    }
    else
    {
        for (int i = 0; i < 4; ++i)
            inputMeter[i].setLabelText(inMeterLabelText[i]);

        title.setLineBounds(false, 0, 65, 138);
    }

}
void AmbiCreatorAudioProcessorEditor::setAbButtonAlphaFromLayerState(int layerState)
{
    if (layerState == eCurrentActiveLayer::layerA)
    {
        tbAbLayer[0].setAlpha(1.0f);
        tbAbLayer[1].setAlpha(0.3f);
    }
    else
    {
        tbAbLayer[0].setAlpha(0.3f);
        tbAbLayer[1].setAlpha(1.0f);
    }
}
