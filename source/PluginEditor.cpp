#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"
#include "PluginProcessor.h"

#define AA_SUPPORT_URL "https://austrian.audio/support-downloads/"

//==============================================================================
AmbiCreatorAudioProcessorEditor::AmbiCreatorAudioProcessorEditor (
    AmbiCreatorAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (&p), processor (p), valueTreeState (vts)
{
    using namespace juce;

    //    setResizable (true, true);
    //    fixedAspectRatioConstrainer.setFixedAspectRatio (double(processor.EDITOR_DEFAULT_WIDTH) / processor.EDITOR_DEFAULT_HEIGHT);
    //    fixedAspectRatioConstrainer.setSizeLimits (processor.EDITOR_DEFAULT_WIDTH, processor.EDITOR_DEFAULT_HEIGHT, 2 * processor.EDITOR_DEFAULT_WIDTH, 2 * processor.EDITOR_DEFAULT_HEIGHT);
    //    setConstrainer (&fixedAspectRatioConstrainer);

#ifdef AA_MELATONIN
    inspector.setVisible (true);
    inspector.toggle (true);
#endif

    valueTreeState.addParameterListener ("outGainDb", this);
    valueTreeState.addParameterListener ("horRotation", this);
    valueTreeState.addParameterListener ("zGainDb", this);
    valueTreeState.addParameterListener ("legacyMode", this);

    setLookAndFeel (&ambiCreatorLookAndFeel);

    addAndMakeVisible (&title);
    title.setTitle (String ("AustrianAudio"), String ("AmbiCreator"));
    //    title.setFont (alternativeLookAndFeel.aaMedium, alternativeLookAndFeel.aaRegular);
    title.setFont (ambiCreatorLookAndFeel.terminatorBoldFont, ambiCreatorLookAndFeel.normalFont);

    title.showAlertSymbol (false);

    title.setAlertMessage (wrongBusConfigMessageShort, wrongBusConfigMessageLong);
    //    cbAttOutChOrder.reset(new ButtonAttachment (valueTreeState, "channelOrder", *title.getOutputWidgetPtr()->getCbOutChOrder()));

    title.getOutputWidgetPtr()->getCbOutChOrder()->addListener (this);

    addAndMakeVisible (&footer);
    tooltipWindow.setLookAndFeel (&ambiCreatorLookAndFeel);
    tooltipWindow.setMillisecondsBeforeTipAppears (500);

    legacyModeImage = ImageCache::getFromMemory (legacyPNGArray, legacyPNGArraySize);
    fourChannelModeImage = ImageCache::getFromMemory (fourChannelPNGArray, fourChannelPNGArraySize);

    aaLogoBgPath.loadPathFromData (aaLogoData, sizeof (aaLogoData));

    //    outGainSlider.setLookAndFeel(&alternativeLookAndFeel);
    //    horizontalRotationSlider.setLookAndFeel(&alternativeLookAndFeel);
    //    zGainSlider.setLookAndFeel(&alternativeLookAndFeel);

    addAndMakeVisible (&outGainGroup);
    outGainGroup.setName ("grpOutputGain");
    outGainGroup.setText ("Output Gain");

    outGainGroup.setEnabled (true);
    addAndMakeVisible (&outGainSlider);
    outGainAttachment.reset (
        new ReverseSlider::SliderAttachment (valueTreeState, "outGainDb", outGainSlider));
    outGainSlider.setSliderStyle (Slider::LinearHorizontal);
    outGainSlider.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    outGainSlider.setColour (Slider::thumbColourId, ambiCreatorLookAndFeel.AARed);
    outGainSlider.addListener (this);

    addAndMakeVisible (&horizontalRotationGroup);
    horizontalRotationGroup.setName ("horizontalRotationGroup");
    horizontalRotationGroup.setText ("Horizontal Rotation");
    horizontalRotationGroup.setEnabled (true);
    addAndMakeVisible (&horizontalRotationSlider);
    horizontalRotationAttachment.reset (
        new ReverseSlider::SliderAttachment (valueTreeState,
                                             "horRotation",
                                             horizontalRotationSlider));
    horizontalRotationSlider.setSliderStyle (Slider::LinearHorizontal);
    horizontalRotationSlider.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    horizontalRotationSlider.setColour (Slider::thumbColourId, ambiCreatorLookAndFeel.AARed);
    horizontalRotationSlider.addListener (this);

    addAndMakeVisible (&zGainGroup);
    zGainGroup.setName ("zGainGroup");
    zGainGroup.setText ("Z Gain");
    zGainGroup.setEnabled (true);
    addAndMakeVisible (&zGainSlider);
    zGainAttachment.reset (
        new ReverseSlider::SliderAttachment (valueTreeState, "zGainDb", zGainSlider));
    zGainSlider.setSliderStyle (Slider::LinearHorizontal);
    zGainSlider.setColour (Slider::rotarySliderOutlineColourId, Colours::black);
    zGainSlider.setColour (Slider::thumbColourId, ambiCreatorLookAndFeel.AARed);
    zGainSlider.addListener (this);

    for (int i = 0; i < 4; ++i)
    {
        addAndMakeVisible (&inputMeter[i]);
        inputMeter[i].setColour (ambiCreatorLookAndFeel.AARed);
        inputMeter[i].setLabelText (inMeterLabelText[i]);

        addAndMakeVisible (&outputMeter[i]);
        outputMeter[i].setColour (ambiCreatorLookAndFeel.AARed);
    }

    // ------------------new AmbiCreator Layout components----------------
    // ugly but simple solution
    title.getOutputWidgetPtr()->setEnabled (false);
    title.getOutputWidgetPtr()->setVisible (false);

    addAndMakeVisible (&tmbOutChannelOrder);
    tmbOutChannelOrder.setButtonsNumber (2);
    tmbOutChannelOrder.setInterceptsMouseClicks (true, true);

    tmbOutChannelOrder[0].setClickingTogglesState (true);
    tmbOutChannelOrder[0].setRadioGroupId (8880);
    tmbOutChannelOrder[0].setButtonText ("AmbiX");
    tmbOutChannelOrder[0].addListener (this);
    //    tmbOutChannelOrder[0].setToggleState(polarDesignerProcessor.abLayerState, NotificationType::dontSendNotification);

    tmbOutChannelOrder[1].setClickingTogglesState (true);
    tmbOutChannelOrder[1].setRadioGroupId (8880);
    tmbOutChannelOrder[1].setButtonText ("FUMA");
    tmbOutChannelOrder[1].addListener (this);

    updateOutputMeterLabelTexts();

#ifdef AA_CONFIG_COMBOBOX
    addAndMakeVisible (&cbOutChannelOrder);
    cbAttOutChannelOrder.reset (
        new ComboBoxAttachment (valueTreeState, "channelOrder", cbOutChannelOrder));
    cbOutChannelOrder.addItem ("AmbiX", 1);
    cbOutChannelOrder.addItem ("FUMA", 2);
    cbOutChannelOrder.setEditableText (false);
    cbOutChannelOrder.setJustificationType (Justification::centred);
    cbOutChannelOrder.setSelectedId (1);
    cbOutChannelOrder.addListener (this);
#endif

    addAndMakeVisible (&outputConfigLabel);
    outputConfigLabel.setText ("Output Config");

    addAndMakeVisible (&tbLegacyMode);
    tbLegacyMode.setButtonText ("legacy mode");
    tbLegacyMode.setClickingTogglesState (true);
    tbLegacyMode.addListener (this);
    tbAttLegacyMode.reset (new ButtonAttachment (valueTreeState, "legacyMode", tbLegacyMode));
    tbLegacyMode.setToggleState (processor.isNormalLRFBMode(),
                                 NotificationType::dontSendNotification);

    addAndMakeVisible (&tbAbLayer[0]);
    tbAbLayer[0].setButtonText ("A");
    tbAbLayer[0].addListener (this);
    tbAbLayer[0].setClickingTogglesState (true);
    tbAbLayer[0].setRadioGroupId (1);
    tbAbLayer[0].setToggleState (true, NotificationType::dontSendNotification);

    addAndMakeVisible (&tbAbLayer[1]);
    tbAbLayer[1].setButtonText ("B");
    tbAbLayer[1].addListener (this);
    tbAbLayer[1].setClickingTogglesState (true);
    tbAbLayer[1].setRadioGroupId (1);
    tbAbLayer[1].setToggleState (false, NotificationType::dontSendNotification);

    setAbButtonAlphaFromLayerState (eCurrentActiveLayer::layerA);

    if (processor.abLayerState == eCurrentActiveLayer::layerB)
    {
        tbAbLayer[0].setToggleState (false, NotificationType::dontSendNotification);
        tbAbLayer[1].setToggleState (true, NotificationType::dontSendNotification);
        setAbButtonAlphaFromLayerState (eCurrentActiveLayer::layerB);
    }
    else
    {
        tbAbLayer[0].setToggleState (true, NotificationType::dontSendNotification);
        tbAbLayer[1].setToggleState (false, NotificationType::dontSendNotification);
        setAbButtonAlphaFromLayerState (eCurrentActiveLayer::layerA);
    }

    // help tooltip
    addAndMakeVisible (&helpToolTip);
    helpToolTip.setText ("help", NotificationType::dontSendNotification);
    helpToolTip.setTextColour (Colours::white.withAlpha (0.5f));
    helpToolTip.setInterceptsMouseClicks (true, false); // Enable mouse clicks
    helpToolTip.addMouseListener (this, false); // Listen for clicks

    setModeDisplay (processor.isNormalLRFBMode());

    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);

    startTimer (100);
}

AmbiCreatorAudioProcessorEditor::~AmbiCreatorAudioProcessorEditor()
{
    valueTreeState.removeParameterListener ("outGainDb", this);
    valueTreeState.removeParameterListener ("horRotation", this);
    valueTreeState.removeParameterListener ("zGainDb", this);
    valueTreeState.removeParameterListener ("legacyMode", this);

    setLookAndFeel (nullptr);
}

void AmbiCreatorAudioProcessorEditor::parameterChanged (const juce::String& parameterID,
                                                        float newValue)
{
    using namespace juce;

    if (parameterID == "outGainDb")
    {
        DBG ("OutGainDb changed to: " << newValue);
    }
    else if (parameterID == "horRotation")
    {
        DBG ("HorRotation changed to: " << newValue);
    }
    else if (parameterID == "zGainDb")
    {
        DBG ("ZGainDb changed to: " << newValue);
    }
    else if (parameterID == "channelOrder")
    {
        eChannelOrder order = static_cast<eChannelOrder> (static_cast<int> (newValue));
        DBG ("ChannelOrder changed to: " << (order == eChannelOrder::ACN ? "AmbiX" : "FUMA"));
        tmbOutChannelOrder[order].setToggleState (true, NotificationType::dontSendNotification);
        updateOutputMeterLabelTexts();
    }
    else if (parameterID == "legacyMode")
    {
        bool legacyMode = newValue > 0.5f;
        DBG ("LegacyMode changed to: "
             << (legacyMode ? "ON" : "OFF")
             << ", Layer=" << (processor.abLayerState == eCurrentActiveLayer::layerA ? "A" : "B"));
        tbLegacyMode.setToggleState (legacyMode, NotificationType::dontSendNotification);
        setModeDisplay (legacyMode);
    }

    repaint();
}

void AmbiCreatorAudioProcessorEditor::mouseUp (const juce::MouseEvent& event)
{
    if (event.eventComponent == &helpToolTip)
    {
        if (! juce::URL (AA_SUPPORT_URL).launchInDefaultBrowser())
        {
            DBG ("Failed to open URL!");
        }
    }
}

//==============================================================================
void AmbiCreatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    const int currHeight = getHeight();
    const int currWidth = getWidth();
    {
        g.fillAll (ambiCreatorLookAndFeel.mainBackground);
        //        g.drawImage(fourChannelModeImage, arrayImageArea, RectanglePlacement::centred);

        if (processor.isNormalLRFBMode())
        {
            g.drawImage (legacyModeImage,
                         -40,
                         0,
                         (int) (arrayImageArea.getWidth() + 100.0f),
                         currHeight + 40,
                         0,
                         0,
                         legacyModeImage.getWidth(),
                         legacyModeImage.getHeight());
            helpToolTip.setTooltip (helpTextLegacy);
        }
        else
        {
            g.drawImage (fourChannelModeImage,
                         -40,
                         0,
                         (int) (arrayImageArea.getWidth() + 100.0f),
                         currHeight + 40,
                         0,
                         0,
                         fourChannelModeImage.getWidth(),
                         fourChannelModeImage.getHeight());
            //            g.drawImageWithin(fourChannelModeImage, 12, 1, fourChannelModeImage.getWidth() / 2, fourChannelModeImage.getHeight() / 2, RectanglePlacement::onlyReduceInSize);
            helpToolTip.setTooltip (helpText);
        }
    }

#ifdef AA_CONFIG_BG_LOGO
    // background logo
    aaLogoBgPath.applyTransform (aaLogoBgPath.getTransformToScaleToFit ((0.4f * currWidth),
                                                                        (0.25f * currHeight),
                                                                        (0.7f * currWidth),
                                                                        (0.7f * currWidth),
                                                                        true,
                                                                        Justification::centred));
    g.setColour (Colours::white.withAlpha (0.1f));
    g.strokePath (aaLogoBgPath, PathStrokeType (0.1f));
    g.fillPath (aaLogoBgPath);
#endif
}

void AmbiCreatorAudioProcessorEditor::resized()
{
    using namespace juce;

    const float currentWidth = getWidth();
    const float currentHeight = getHeight();

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

    const float rotarySliderHeight = 0.12f * currentHeight;
    const float rotarySliderVerticalSpacing = 0.05f * currentHeight;
    const float buttonHeight = 0.051f * currentHeight;
    const float legacyButtonWidth = 0.13f * currentWidth;
    const float horizontalButtonSpacing = 0.006f * currentWidth;
    const float comboBoxHeight = 0.038f * currentHeight;

    Rectangle<int> area (getLocalBounds());

    Rectangle<int> footerArea (area.removeFromBottom ((int) footerHeight));
    helpToolTip.setBounds (5, getHeight() - 30, 40, 15);
    footer.setBounds (footerArea);

    area.removeFromLeft ((int) leftRightMargin);
    area.removeFromRight ((int) leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);

    if (processor.isNormalLRFBMode())
    {
        title.setLineBounds (false,
                             0.0f,
                             (int) (0.116f * currentWidth),
                             (int) (0.186f * currentWidth));
    }
    else
    {
        title.setLineBounds (false, 0, 65, 138);
    }

    Rectangle<int> headerButtonArea = headerArea;
    headerButtonArea.removeFromRight ((int) (leftRightMargin / 8.0f));
    headerButtonArea.removeFromTop ((int) (headerHeight / 2.0f - buttonHeight / 2.0f));
    headerButtonArea.removeFromBottom ((int) (headerHeight / 2.0f - buttonHeight / 2.0f));
    tbLegacyMode.setBounds (headerButtonArea.removeFromRight ((int) legacyButtonWidth));
    headerButtonArea.removeFromRight ((int) (2.0f * horizontalButtonSpacing));
    tbAbLayer[1].setBounds (headerButtonArea.removeFromRight ((int) buttonHeight));
    headerButtonArea.removeFromRight ((int) (2.0f * horizontalButtonSpacing));
    tbAbLayer[0].setBounds (headerButtonArea.removeFromRight ((int) buttonHeight));

    area.removeFromTop ((int) topMargin);
    arrayImageArea = area.removeFromLeft ((int) arrayWidth).toFloat();

    // -------- MAIN AREA ---------
    const float contentWidth =
        8 * meterWidth + 2 * meterToSliderSpacing + 8 * meterSpacing + linearSliderWidth;
    const float mainMarginLeft = ((int) area.getWidth() - contentWidth) / 2.0f;

    area.removeFromLeft (mainMarginLeft * 1.0f);

    Rectangle<int> inMeterArea =
        area.removeFromLeft ((int) (4.0f * meterWidth + 4.0f * meterSpacing))
            .withHeight ((int) meterHeight);
    inMeterArea = inMeterArea.withCentre (
        Point<int> (inMeterArea.getCentreX(), (int) (area.getHeight() * 0.6f)));

    for (int i = 0; i < 4; ++i)
    {
        inputMeter[i].setBounds (inMeterArea.removeFromLeft ((int) meterWidth));
        inMeterArea.removeFromLeft ((int) meterSpacing);
    }
    area.removeFromLeft ((int) meterToSliderSpacing);

    Rectangle<int> sliderArea =
        area.removeFromLeft ((int) linearSliderWidth)
            .withHeight ((int) (3.0f * linearSliderVerticalSpacing + 3.0f * labelHeight
                                + 3.0f * linearSliderHeight));
    sliderArea = sliderArea.withCentre (
        Point<int> (sliderArea.getCentreX(), (int) (area.getHeight() * 0.55f)));

    Rectangle<int> rotSliderArea = sliderArea;

    {
        int numGroups = 3; // Total groups
        int groupSpacing = 10; // Space between groups

        sliderArea.removeFromTop (linearSliderVerticalSpacing);

        // Height for each group including spacing
        int totalSpacing = (numGroups - 1) * groupSpacing;
        int groupHeight = (sliderArea.getHeight() - totalSpacing) / numGroups;

        // OUT GAIN GROUP
        outGainGroup.setBounds (
            sliderArea.removeFromTop (groupHeight).reduced (2)); // Assign a portion of sliderArea
        auto outGainArea = outGainGroup.getBounds().reduced (4); // Inner padding for components
        outGainSlider.setBounds (outGainArea.reduced (2)); // removeFromTop(linearSliderHeight));
        outGainSlider.setTextBoxStyle (Slider::TextBoxRight, false, (int) slTbWidth, slTbHeight);

        sliderArea.removeFromTop (groupSpacing); // Space before next group

        // Z GAIN GROUP
        zGainGroup.setBounds (sliderArea.removeFromTop (groupHeight).reduced (2));
        auto zGainArea = zGainGroup.getBounds().reduced (4);
        zGainSlider.setBounds (zGainArea.reduced (2));
        zGainSlider.setTextBoxStyle (Slider::TextBoxRight, false, (int) slTbWidth, slTbHeight);

        sliderArea.removeFromTop (groupSpacing); // Space before next group

        // HORIZONTAL ROTATION GROUP
        horizontalRotationGroup.setBounds (sliderArea.removeFromTop (groupHeight).reduced (2));
        auto horizontalRotationArea = horizontalRotationGroup.getBounds().reduced (4);
        horizontalRotationSlider.setBounds (horizontalRotationArea.reduced (2));
        horizontalRotationSlider.setTextBoxStyle (Slider::TextBoxRight,
                                                  false,
                                                  (int) slTbWidth,
                                                  slTbHeight);
    }

    area.removeFromLeft (meterToSliderSpacing);

    Rectangle<int> outMeterArea =
        area.removeFromLeft (4 * meterWidth + 4 * meterSpacing).withHeight (meterHeight);

    outMeterArea = outMeterArea.withCentre (
        Point<int> (outMeterArea.getCentreX(), int (area.getHeight() * 0.6f)));

    outputConfigLabel.setBounds (outMeterArea.getX() - 7 * meterSpacing,
                                 outMeterArea.getY() - 4 * comboBoxHeight,
                                 4 * meterWidth + 14 * meterSpacing,
                                 comboBoxHeight);
    tmbOutChannelOrder.setBounds (outMeterArea.getX() - 7 * meterSpacing,
                                  outGainSlider.getY(),
                                  4 * meterWidth + 14 * meterSpacing,
                                  labelHeight);

    for (int i = 0; i < 4; ++i)
    {
        outputMeter[i].setBounds (
            outMeterArea.removeFromLeft (meterWidth).withHeight (meterHeight));
        outMeterArea.removeFromLeft (meterSpacing);
    }
}

void AmbiCreatorAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
}

void AmbiCreatorAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    using namespace juce;

    if (button == &tbLegacyMode)
    {
        bool isToggled = button->getToggleState();
        DBG ("tbLegacyMode clicked: Setting legacyMode to " << (isToggled ? "ON" : "OFF"));
        valueTreeState.getParameter ("legacyMode")->setValueNotifyingHost (isToggled ? 1.0f : 0.0f);
        setModeDisplay (processor.isNormalLRFBMode());
    }
    else if (button == &tbAbLayer[0] && button->getToggleState())
    {
        processor.setAbLayer (eCurrentActiveLayer::layerA);
        setAbButtonAlphaFromLayerState (eCurrentActiveLayer::layerA);
        tbAbLayer[0].setToggleState (true, NotificationType::dontSendNotification);
        tbAbLayer[1].setToggleState (false, NotificationType::dontSendNotification);
        setModeDisplay (processor.isNormalLRFBMode());
    }
    else if (button == &tbAbLayer[1] && button->getToggleState())
    {
        processor.setAbLayer (eCurrentActiveLayer::layerB);
        setAbButtonAlphaFromLayerState (eCurrentActiveLayer::layerB);
        tbAbLayer[0].setToggleState (false, NotificationType::dontSendNotification);
        tbAbLayer[1].setToggleState (true, NotificationType::dontSendNotification);
        setModeDisplay (processor.isNormalLRFBMode());
    }
    else if (button == &tmbOutChannelOrder[0] && button->getToggleState())
    {
        valueTreeState.getParameter ("channelOrder")
            ->setValueNotifyingHost (
                valueTreeState.getParameter ("channelOrder")->convertTo0to1 (eChannelOrder::ACN));
        updateOutputMeterLabelTexts();
    }
    else if (button == &tmbOutChannelOrder[1] && button->getToggleState())
    {
        valueTreeState.getParameter ("channelOrder")
            ->setValueNotifyingHost (
                valueTreeState.getParameter ("channelOrder")->convertTo0to1 (eChannelOrder::FUMA));
        updateOutputMeterLabelTexts();
    }

    repaint();
}

void AmbiCreatorAudioProcessorEditor::comboBoxChanged (juce::ComboBox* cb)
{
    if (cb == title.getOutputWidgetPtr()->getCbOutChOrder())
        updateOutputMeterLabelTexts();
}

void AmbiCreatorAudioProcessorEditor::updateOutputMeterLabelTexts()
{
    if (processor.getChannelOrder() == eChannelOrder::ACN)
    {
        for (int i = 0; i < 4; ++i)
            outputMeter[i].setLabelText (outMeterLabelTextACN[i]);
    }
    else if (processor.getChannelOrder() == eChannelOrder::FUMA)
    {
        for (int i = 0; i < 4; ++i)
            outputMeter[i].setLabelText (outMeterLabelTextFUMA[i]);
    }
}

void AmbiCreatorAudioProcessorEditor::timerCallback()
{
    using namespace juce;

    DBG ("Timer: LegacyMode=" << (processor.isNormalLRFBMode() ? "ON" : "OFF") << ", tbLegacyMode="
                              << (tbLegacyMode.getToggleState() ? "ON" : "OFF"));

    for (int i = 0; i < 4; ++i)
    {
        inputMeter[i].setLevel (processor.inRms[i].get());
        outputMeter[i].setLevel (processor.outRms[i].get());
    }

    // show alert message if bus configuration is wrong, i.e. there are
    // not 4 ins and outs
    if (processor.wrongBusConfiguration.get())
    {
        title.setAlertMessage (wrongBusConfigMessageShort, wrongBusConfigMessageLong);
        title.showAlertSymbol (true);
        return;
    }

    // also alert if the processor is playing, but some input channels remain silent
    if (processor.isPlaying.get())
    {
        for (auto& active : processor.channelActive)
        {
            if (! active.get())
            {
                title.setAlertMessage (inputInactiveMessageShort, inputInactiveMessageLong);
                title.showAlertSymbol (true);
                return;
            }
        }
    }

    if (processor.getChannelOrder() == eChannelOrder::ACN)
    {
        tmbOutChannelOrder[0].setToggleState (true, NotificationType::dontSendNotification);
        tmbOutChannelOrder[1].setToggleState (false, NotificationType::dontSendNotification);
    }
    else
    {
        tmbOutChannelOrder[0].setToggleState (false, NotificationType::dontSendNotification);
        tmbOutChannelOrder[1].setToggleState (true, NotificationType::dontSendNotification);
    }

    setAbButtonAlphaFromLayerState (processor.abLayerState);
    tbAbLayer[0].setToggleState (processor.abLayerState == eCurrentActiveLayer::layerA,
                                 NotificationType::dontSendNotification);
    tbAbLayer[1].setToggleState (processor.abLayerState == eCurrentActiveLayer::layerB,
                                 NotificationType::dontSendNotification);

    tbLegacyMode.setToggleState (processor.isNormalLRFBMode(),
                                 NotificationType::dontSendNotification);
    setModeDisplay (processor.isNormalLRFBMode());

    title.showAlertSymbol (false);
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

void AmbiCreatorAudioProcessorEditor::setModeDisplay (bool legacyModeActive)
{
#ifdef AA_CONFIG_ROTARY_UI
    zGainSlider.setEnabled (legacyModeActive);
    zGainSlider.setVisible (legacyModeActive);
    outGainSlider.setEnabled (legacyModeActive);
    outGainSlider.setVisible (legacyModeActive);
    outGainLabel.setVisible (legacyModeActive);
    zGainLabel.setVisible (legacyModeActive);

    slRotZGain.setVisible (! legacyModeActive);
    slRotZGain.setEnabled (! legacyModeActive);
    slRotOutGain.setVisible (! legacyModeActive);
    slRotOutGain.setEnabled (! legacyModeActive);
    lbSlRotZGain.setVisible (! legacyModeActive);
    lbSlRotOutGain.setVisible (! legacyModeActive);
#endif

    if (legacyModeActive)
    {
        for (int i = 0; i < 4; ++i)
            inputMeter[i].setLabelText (inMeterLabelTextLegacy[i]);

        title.setLineBounds (false,
                             0.0f,
                             0.116f * getLocalBounds().getWidth(),
                             0.186f * getLocalBounds().getWidth());
    }
    else
    {
        for (int i = 0; i < 4; ++i)
            inputMeter[i].setLabelText (inMeterLabelText[i]);

        title.setLineBounds (false,
                             0,
                             0.116f * getLocalBounds().getWidth(),
                             0.186f * getLocalBounds().getWidth()); // 65, 138);
    }
}
void AmbiCreatorAudioProcessorEditor::setAbButtonAlphaFromLayerState (int layerState)
{
    if (layerState == eCurrentActiveLayer::layerA)
    {
        tbAbLayer[0].setAlpha (1.0f);
        tbAbLayer[1].setAlpha (0.3f);
    }
    else
    {
        tbAbLayer[0].setAlpha (0.3f);
        tbAbLayer[1].setAlpha (1.0f);
    }
}
