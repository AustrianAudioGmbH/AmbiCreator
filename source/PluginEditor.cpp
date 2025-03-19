#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"

#define AA_SUPPORT_URL "https://austrian.audio/support-downloads/"

//==============================================================================
AmbiCreatorAudioProcessorEditor::AmbiCreatorAudioProcessorEditor (AmbiCreatorAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
//    setResizable (true, true);
//    fixedAspectRatioConstrainer.setFixedAspectRatio (double(processor.EDITOR_DEFAULT_WIDTH) / processor.EDITOR_DEFAULT_HEIGHT);
//    fixedAspectRatioConstrainer.setSizeLimits (processor.EDITOR_DEFAULT_WIDTH, processor.EDITOR_DEFAULT_HEIGHT, 2 * processor.EDITOR_DEFAULT_WIDTH, 2 * processor.EDITOR_DEFAULT_HEIGHT);
//    setConstrainer (&fixedAspectRatioConstrainer);

#ifdef AA_MELATONIN
    inspector.setVisible(true);
    inspector.toggle(true);
#endif

    setLookAndFeel (&ambiCreatorLookAndFeel);
    
    addAndMakeVisible (&title);
    title.setTitle (String("AustrianAudio"),String("AmbiCreator"));
//    title.setFont (alternativeLookAndFeel.aaMedium, alternativeLookAndFeel.aaRegular);
    title.setFont (ambiCreatorLookAndFeel.terminatorBoldFont, ambiCreatorLookAndFeel.normalFont);

    title.showAlertSymbol(false);

    title.setAlertMessage(wrongBusConfigMessageShort, wrongBusConfigMessageLong);
    //    cbAttOutChOrder.reset(new ButtonAttachment (valueTreeState, "channelOrder", *title.getOutputWidgetPtr()->getCbOutChOrder()));

    title.getOutputWidgetPtr()->getCbOutChOrder()->addListener(this);
    
    addAndMakeVisible (&footer);
    tooltipWindow.setLookAndFeel (&ambiCreatorLookAndFeel);
    tooltipWindow.setMillisecondsBeforeTipAppears(500);

    legacyModeImage = ImageCache::getFromMemory (legacyPNGArray, legacyPNGArraySize);
    fourChannelModeImage = ImageCache::getFromMemory (fourChannelPNGArray, fourChannelPNGArraySize);

    aaLogoBgPath.loadPathFromData (aaLogoData, sizeof (aaLogoData));

//    outGainSlider.setLookAndFeel(&alternativeLookAndFeel);
//    horizontalRotationSlider.setLookAndFeel(&alternativeLookAndFeel);
//    zGainSlider.setLookAndFeel(&alternativeLookAndFeel);

    addAndMakeVisible(&outGainGroup);
    outGainGroup.setName("grpOutputGain");
    outGainGroup.setText("Output Gain");
    outGainGroup.setEnabled(true);
    addAndMakeVisible (&outGainSlider);
    outGainAttachment.reset(new ReverseSlider::SliderAttachment (valueTreeState, "outGainDb", outGainSlider));
    outGainSlider.setSliderStyle(Slider::LinearHorizontal);
    outGainSlider.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    outGainSlider.setColour (Slider::thumbColourId, ambiCreatorLookAndFeel.AARed);
    outGainSlider.addListener (this);

    addAndMakeVisible(&horizontalRotationGroup);
    horizontalRotationGroup.setName("horizontalRotationGroup");
    horizontalRotationGroup.setText("Horizontal Rotation");
    horizontalRotationGroup.setEnabled(true);
    addAndMakeVisible (&horizontalRotationSlider);
    horizontalRotationAttachment.reset(new ReverseSlider::SliderAttachment (valueTreeState, "horRotation", horizontalRotationSlider));
    horizontalRotationSlider.setSliderStyle (Slider::LinearHorizontal);
    horizontalRotationSlider.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    horizontalRotationSlider.setColour (Slider::thumbColourId, ambiCreatorLookAndFeel.AARed);
    horizontalRotationSlider.addListener (this);

    addAndMakeVisible(&zGainGroup);
    zGainGroup.setName("zGainGroup");
    zGainGroup.setText("Z Gain");
    zGainGroup.setEnabled(true);
    addAndMakeVisible (&zGainSlider);
    zGainAttachment.reset(new ReverseSlider::SliderAttachment (valueTreeState, "zGainDb", zGainSlider));
    zGainSlider.setSliderStyle (Slider::LinearHorizontal);
    zGainSlider.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    zGainSlider.setColour (Slider::thumbColourId, ambiCreatorLookAndFeel.AARed);
    zGainSlider.addListener (this);


    for (int i = 0; i < 4; ++i)
    {
        addAndMakeVisible(&inputMeter[i]);
        inputMeter[i].setColour(ambiCreatorLookAndFeel.AARed);
        inputMeter[i].setLabelText(inMeterLabelText[i]);
        
        addAndMakeVisible(&outputMeter[i]);
        outputMeter[i].setColour(ambiCreatorLookAndFeel.AARed);
    }


    // ------------------new AmbiCreator Layout components----------------
    // ugly but simple solution
    title.getOutputWidgetPtr()->setEnabled(false);
    title.getOutputWidgetPtr()->setVisible(false);


    addAndMakeVisible(&tmbOutChannelOrder);
    tmbOutChannelOrder.setButtonsNumber(2);
    tmbOutChannelOrder.setInterceptsMouseClicks(true, true);

    tmbOutChannelOrder[0].setClickingTogglesState(true);
    tmbOutChannelOrder[0].setRadioGroupId(8880);
    tmbOutChannelOrder[0].setButtonText("AmbiX");
    tmbOutChannelOrder[0].addListener(this);
//    tmbOutChannelOrder[0].setToggleState(polarDesignerProcessor.abLayerState, NotificationType::dontSendNotification);

    tmbOutChannelOrder[1].setClickingTogglesState(true);
    tmbOutChannelOrder[1].setRadioGroupId(8880);
    tmbOutChannelOrder[1].setButtonText("FUMA");
    tmbOutChannelOrder[1].addListener(this);

    updateOutputMeterLabelTexts();

#ifdef AA_CONFIG_COMBOBOX
    addAndMakeVisible(&cbOutChannelOrder);
    cbAttOutChannelOrder.reset(new ComboBoxAttachment (valueTreeState, "channelOrder", cbOutChannelOrder));
    cbOutChannelOrder.addItem("AmbiX", 1);
    cbOutChannelOrder.addItem("FUMA", 2);
    cbOutChannelOrder.setEditableText(false);
    cbOutChannelOrder.setJustificationType (Justification::centred);
    cbOutChannelOrder.setSelectedId (1);
    cbOutChannelOrder.addListener(this);
#endif

#ifdef AA_CONFIG_ROTARY_UI // !J!
    addAndMakeVisible(&slRotZGain);
    slAttRotZGain.reset(new SliderAttachment (valueTreeState, "zGainDb", slRotZGain));
//    slRotZGain.setSliderStyle(Slider::Rotary); // !J!
    slRotZGain.setColour(Slider::rotarySliderOutlineColourId, ambiCreatorLookAndFeel.AARed);
    slRotZGain.setTextValueSuffix(" dB");
    slRotZGain.addListener(this);

    addAndMakeVisible(&slRotOutGain);
    slAttRotOutGain.reset(new SliderAttachment (valueTreeState, "outGainDb", slRotOutGain));
//    slRotOutGain.setSliderStyle(Slider::Rotary); // !J!
    slRotOutGain.setColour(Slider::rotarySliderOutlineColourId, ambiCreatorLookAndFeel.AARed);
    slRotOutGain.setTextValueSuffix(" dB");
    slRotOutGain.addListener(this);

    addAndMakeVisible (&lbSlRotOutGain);
    lbSlRotOutGain.setText("Output Gain");

    addAndMakeVisible (&lbSlRotZGain);
    lbSlRotZGain.setText("Z Gain");
#endif

    addAndMakeVisible(&outputConfigLabel);
    outputConfigLabel.setText("Output Config");

    addAndMakeVisible (&tbLegacyMode);
    tbAttLegacyMode.reset(new ButtonAttachment (valueTreeState, "legacyMode", tbLegacyMode));
    tbLegacyMode.addListener(this);
    tbLegacyMode.setButtonText("legacy mode");

    auto legacyModeInit = valueTreeState.getParameter("legacyMode")->getValue();
    valueTreeState.getParameter("legacyMode")->setValueNotifyingHost(legacyModeInit);

    tbLegacyMode.setToggleState(processor.isNormalLRFBMode(), NotificationType::dontSendNotification);

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
    helpToolTip.setInterceptsMouseClicks(true, false); // Enable mouse clicks
    helpToolTip.addMouseListener(this, false); // Listen for clicks

    setModeDisplay(processor.isNormalLRFBMode());

    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);
    setResizable(true, true );
    setResizeLimits(600, 490, 1920, 1080);


    startTimer (100);

}

AmbiCreatorAudioProcessorEditor::~AmbiCreatorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void AmbiCreatorAudioProcessorEditor::mouseUp(const juce::MouseEvent& event)
{
    if (event.eventComponent == &helpToolTip)
    {
        if (!juce::URL(AA_SUPPORT_URL).launchInDefaultBrowser())
        {
            DBG("Failed to open URL!");
        }
    }
}

//==============================================================================
void AmbiCreatorAudioProcessorEditor::paint (Graphics& g)
{
    const int currHeight = getHeight();
    const int currWidth = getWidth();
    {
        g.fillAll (ambiCreatorLookAndFeel.mainBackground);
//        g.drawImage(fourChannelModeImage, arrayImageArea, RectanglePlacement::centred);

        if (processor.isNormalLRFBMode())
        {
            g.drawImage(legacyModeImage, -40, 0, (int)(arrayImageArea.getWidth() + 100.0f), currHeight + 40, 0, 0, legacyModeImage.getWidth(), legacyModeImage.getHeight());
            helpToolTip.setTooltip(helpTextLegacy);
        }
        else
        {
            g.drawImageWithin(fourChannelModeImage, 12, 1, fourChannelModeImage.getWidth() / 2, fourChannelModeImage.getHeight() / 2, RectanglePlacement::onlyReduceInSize);
            helpToolTip.setTooltip(helpText);
        }

    }

#ifdef AA_CONFIG_BG_LOGO
    // background logo
    aaLogoBgPath.applyTransform (aaLogoBgPath.getTransformToScaleToFit ((0.4f * currWidth), (0.25f * currHeight),
                                                                        (0.7f * currWidth), (0.7f * currWidth), true, Justification::centred));
    g.setColour (Colours::white.withAlpha(0.1f));
    g.strokePath (aaLogoBgPath, PathStrokeType (0.1f));
    g.fillPath (aaLogoBgPath);
#endif

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
    const float linearSliderHeight = 0.06f * currentHeight;
    const float labelHeight = 0.07f * currentHeight;
    const float meterWidth = 0.023f * currentWidth;
    const float meterHeight = 0.32f * currentHeight;
    const float meterSpacing = 0.003f * currentWidth;
    const float meterToSliderSpacing = 0.046f * currentWidth;
    const float arrayWidth = 0.308f * currentWidth;
    const float slTbWidth = 0.08f * currentWidth;
    const float slTbHeight = 0.06f * currentHeight;

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
    
    if (processor.isNormalLRFBMode())
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

#ifdef AA_CONFIG_ROTARY_UI
    rotSliderArea.removeFromTop((int)rotarySliderVerticalSpacing);
    lbSlRotOutGain.setBounds(rotSliderArea.removeFromTop((int)labelHeight));
    slRotOutGain.setTextBoxStyle(Slider::TextBoxBelow, false, (int)slTbWidth, slTbHeight);
    slRotOutGain.setBounds(rotSliderArea.removeFromTop(rotarySliderHeight));

    rotSliderArea.removeFromTop(rotarySliderVerticalSpacing);
    lbSlRotZGain.setBounds(rotSliderArea.removeFromTop(labelHeight));
    slRotZGain.setTextBoxStyle(Slider::TextBoxBelow, false, slTbWidth, slTbHeight);
    slRotZGain.setBounds(rotSliderArea.removeFromTop(rotarySliderHeight));
#endif
    {
        int numGroups = 3;  // Total groups
        int groupSpacing = 10;  // Space between groups

        sliderArea.removeFromTop(linearSliderVerticalSpacing);

        // Height for each group including spacing
        int totalSpacing = (numGroups - 1) * groupSpacing;
        int groupHeight = (sliderArea.getHeight() - totalSpacing) / numGroups;


        // OUT GAIN GROUP
        outGainGroup.setBounds(sliderArea.removeFromTop(groupHeight).reduced(2)); // Assign a portion of sliderArea
        auto outGainArea = outGainGroup.getBounds().reduced(4); // Inner padding for components
        outGainSlider.setBounds(outGainArea.reduced(2)); // removeFromTop(linearSliderHeight));
        outGainSlider.setTextBoxStyle(Slider::TextBoxRight, false, (int)slTbWidth, slTbHeight);

        sliderArea.removeFromTop(groupSpacing); // Space before next group

        // Z GAIN GROUP
        zGainGroup.setBounds(sliderArea.removeFromTop(groupHeight).reduced(2));
        auto zGainArea = zGainGroup.getBounds().reduced(4);
        zGainSlider.setBounds(zGainArea.reduced(2));
        zGainSlider.setTextBoxStyle(Slider::TextBoxRight, false, (int)slTbWidth, slTbHeight);

        sliderArea.removeFromTop(groupSpacing); // Space before next group

        // HORIZONTAL ROTATION GROUP
        horizontalRotationGroup.setBounds(sliderArea.removeFromTop(groupHeight).reduced(2));
        auto horizontalRotationArea = horizontalRotationGroup.getBounds().reduced(4);
        horizontalRotationSlider.setBounds(horizontalRotationArea.reduced(2));
        horizontalRotationSlider.setTextBoxStyle(Slider::TextBoxRight, false, (int)slTbWidth, slTbHeight);

    }

    area.removeFromLeft(meterToSliderSpacing);

    Rectangle<int> outMeterArea = area.removeFromLeft(4 * meterWidth + 4 * meterSpacing).withHeight(meterHeight);

    outMeterArea = outMeterArea.withCentre(Point<int> (outMeterArea.getCentreX(), int(area.getHeight() * 0.6f)));

    outputConfigLabel.setBounds(outMeterArea.getX() - 7 * meterSpacing, outMeterArea.getY() - 4 * comboBoxHeight, 4 * meterWidth + 14 * meterSpacing, comboBoxHeight);
    tmbOutChannelOrder.setBounds(outMeterArea.getX() - 7 * meterSpacing, outGainSlider.getY(), 4 * meterWidth + 14 * meterSpacing, labelHeight);

#ifdef AA_CONFIG_COMBOBOX
    cbOutChannelOrder.setBounds(outMeterArea.getX() - 7 * meterSpacing, outMeterArea.getY() - 2 * comboBoxHeight, 4 * meterWidth + 14 * meterSpacing, comboBoxHeight);
#endif

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
        tbLegacyMode.setToggleState(processor.isNormalLRFBMode(), NotificationType::dontSendNotification);
        setModeDisplay(processor.isNormalLRFBMode());
    }
    else if (button == &tbAbLayer[1])
    {
        bool isToggled = button->getToggleState();
        if (isToggled < 0.5f)
        {
            processor.setAbLayer(eCurrentActiveLayer::layerA);
            setAbButtonAlphaFromLayerState(eCurrentActiveLayer::layerA);
        }
        tbLegacyMode.setToggleState(processor.isNormalLRFBMode(), NotificationType::dontSendNotification);
        setModeDisplay(processor.isNormalLRFBMode());
    }
    else if (button == &tmbOutChannelOrder[0])
    {
        bool isToggled = button->getToggleState();
        if (!isToggled)
        {
            for (int i = 0; i < 4; ++i)
                outputMeter[i].setLabelText(outMeterLabelTextACN[i]);
        }
        valueTreeState.getParameter("channelOrder")->setValueNotifyingHost(valueTreeState.getParameter("channelOrder")->convertTo0to1((eChannelOrder::ACN)));
    }
    else if (button == &tmbOutChannelOrder[1]) {
        bool isToggled = button->getToggleState();
        if (!isToggled) {
            for (int i = 0; i < 4; ++i)
                outputMeter[i].setLabelText(outMeterLabelTextFUMA[i]);
        }
        valueTreeState.getParameter("channelOrder")->setValueNotifyingHost(
                valueTreeState.getParameter("channelOrder")->convertTo0to1((eChannelOrder::FUMA)));
    }

    updateOutputMeterLabelTexts();

    repaint();
}

void AmbiCreatorAudioProcessorEditor::comboBoxChanged (ComboBox* cb) {
    if (cb == title.getOutputWidgetPtr()->getCbOutChOrder())
        updateOutputMeterLabelTexts();
}

void AmbiCreatorAudioProcessorEditor::updateOutputMeterLabelTexts()
{
    if (processor.getChannelOrder() == eChannelOrder::ACN)
    {
        for (int i = 0; i < 4; ++i)
            outputMeter[i].setLabelText(outMeterLabelTextACN[i]);
    }
    else if (processor.getChannelOrder() == eChannelOrder::FUMA)
    {
        for (int i = 0; i < 4; ++i)
            outputMeter[i].setLabelText(outMeterLabelTextFUMA[i]);
    }
}

void AmbiCreatorAudioProcessorEditor::timerCallback()
{
//    setModeDisplay(processor.isNormalLRFBMode());

//DBG("EDITOR TIMER: LegacyMode:" << String(processor.isNormalLRFBMode() ? "TRUE" : "FALSE"));

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

    if (processor.getChannelOrder() == eChannelOrder::ACN) {
        tmbOutChannelOrder[0].setToggleState(true, NotificationType::dontSendNotification);
        tmbOutChannelOrder[1].setToggleState(false, NotificationType::dontSendNotification);
    }
    else {
        tmbOutChannelOrder[0].setToggleState(false, NotificationType::dontSendNotification);
        tmbOutChannelOrder[1].setToggleState(true, NotificationType::dontSendNotification);
    }


    title.showAlertSymbol(false);
}

// implement this for AAX automation shortchut
int AmbiCreatorAudioProcessorEditor::getControlParameterIndex (Component& control)
{
    if (&control == &outGainSlider)
        return 1;
    else if (&control == &horizontalRotationSlider)
        return 2;
    else if (&control == &zGainSlider)
        return 3;
    
    return -1;
}

void AmbiCreatorAudioProcessorEditor::setModeDisplay(bool legacyModeActive)
{
#ifdef AA_CONFIG_ROTARY_UI
    zGainSlider.setEnabled(legacyModeActive);
    zGainSlider.setVisible(legacyModeActive);
    outGainSlider.setEnabled(legacyModeActive);
    outGainSlider.setVisible(legacyModeActive);
    outGainLabel.setVisible(legacyModeActive);
    zGainLabel.setVisible(legacyModeActive);

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
