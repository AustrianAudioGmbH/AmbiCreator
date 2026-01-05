
#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"
#include "PluginProcessor.h"
#include "juce_core/juce_core.h"
#include "juce_events/juce_events.h"
#include <memory>

#define AA_SUPPORT_URL "https://austrian.audio/support-downloads/"

//==============================================================================
AmbiCreatorAudioProcessorEditor::AmbiCreatorAudioProcessorEditor (
    AmbiCreatorAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts) :
    juce::AudioProcessorEditor (p), ambiCreatorProcessor (p), valueTreeState (vts)
{
    using namespace juce;

#ifdef AA_MELATONIN
    inspector.setVisible (true);
    inspector.toggle (true);
#endif

    valueTreeState.addParameterListener ("outGainDb", this);
    valueTreeState.addParameterListener ("horRotation", this);
    valueTreeState.addParameterListener ("zGainDb", this);
    valueTreeState.addParameterListener ("legacyMode", this);

    addAndMakeVisible (logo);
    addAndMakeVisible (abComponent);

    abComponent[0].addListener (this);
    abComponent[1].addListener (this);

    addAndMakeVisible (footer);
    tooltipWindow.setMillisecondsBeforeTipAppears (500);

    legacyModeImage = ImageCache::getFromMemory (legacyPNGArray, legacyPNGArraySize);
    fourChannelModeImage = ImageCache::getFromMemory (fourChannelPNGArray, fourChannelPNGArraySize);

    aaLogoBgPath.loadPathFromData (aaLogoData, sizeof (aaLogoData));

    addAndMakeVisible (outGainGroup);
    outGainGroup.setName ("grpOutputGain");
    outGainGroup.setText ("Output Gain");
    outGainGroup.slider.setTextValueSuffix (" dB");
    outGainGroup.slider.setColour (Slider::rotarySliderOutlineColourId,
                                   AAGuiComponents::Colours::polarVisualizerRed);
    outGainGroup.slider.addListener (this);
    outGainAttachment =
        std::make_unique<SliderAttachment> (valueTreeState, "outGainDb", outGainGroup.slider);

    addAndMakeVisible (horizontalRotationGroup);
    horizontalRotationGroup.setName ("horizontalRotationGroup");
    horizontalRotationGroup.setText ("Horizontal Rotation");
    horizontalRotationGroup.slider.setTextValueSuffix (std::string ("°"));
    horizontalRotationGroup.slider.setColour (Slider::rotarySliderOutlineColourId,
                                              AAGuiComponents::Colours::polarVisualizerRed);
    horizontalRotationGroup.slider.addListener (this);
    horizontalRotationAttachment =
        std::make_unique<SliderAttachment> (valueTreeState,
                                            "horRotation",
                                            horizontalRotationGroup.slider);

    addAndMakeVisible (zGainGroup);
    zGainGroup.setName ("zGainGroup");
    zGainGroup.setText ("Z Gain");
    zGainGroup.slider.setTextValueSuffix (" dB");
    zGainGroup.slider.setColour (Slider::rotarySliderOutlineColourId,
                                 AAGuiComponents::Colours::polarVisualizerRed);
    zGainGroup.slider.addListener (this);
    zGainAttachment =
        std::make_unique<SliderAttachment> (valueTreeState, "zGainDb", zGainGroup.slider);

    for (int i = 0; i < 4; ++i)
    {
        addAndMakeVisible (inputMeter[i]);
        inputMeter[i].setColour (AAGuiComponents::Colours::polarVisualizerRed);
        inputMeter[i].setLabelText (inMeterLabelText[i]);

        addAndMakeVisible (outputMeter[i]);
        outputMeter[i].setColour (AAGuiComponents::Colours::polarVisualizerRed);
    }

    addAndMakeVisible (outputConfigGroup);
    outputConfigGroup.comboBox.setSelectedId (eChannelOrder::ACN,
                                              NotificationType::dontSendNotification);
    cbAttOutChannelOrder = std::make_unique<ComboBoxAttachment> (valueTreeState,
                                                                 "channelOrder",
                                                                 outputConfigGroup.comboBox);
    outputConfigGroup.comboBox.addListener (this);

    updateOutputMeterLabelTexts();

    addAndMakeVisible (tbLegacyMode);
    tbLegacyMode.setButtonText ("legacy mode");
    tbLegacyMode.setClickingTogglesState (true);
    tbLegacyMode.addListener (this);
    tbAttLegacyMode =
        std::make_unique<ButtonAttachment> (valueTreeState, "legacyMode", tbLegacyMode);
    tbLegacyMode.setToggleState (ambiCreatorProcessor.isNormalLRFBMode(),
                                 NotificationType::dontSendNotification);

    abComponent[0].setToggleState (true, NotificationType::dontSendNotification);
    abComponent[1].setToggleState (false, NotificationType::dontSendNotification);

    if (ambiCreatorProcessor.abLayerState == eCurrentActiveLayer::layerB)
    {
        abComponent[0].setToggleState (false, NotificationType::dontSendNotification);
        abComponent[1].setToggleState (true, NotificationType::dontSendNotification);
    }
    else
    {
        abComponent[0].setToggleState (true, NotificationType::dontSendNotification);
        abComponent[1].setToggleState (false, NotificationType::dontSendNotification);
    }

    // help tooltip
    addAndMakeVisible (helpToolTip);
    helpToolTip.setText ("help", NotificationType::dontSendNotification);
    helpToolTip.setTextColour (Colours::white.withAlpha (0.5f));
    helpToolTip.setInterceptsMouseClicks (true, false); // Enable mouse clicks
    helpToolTip.addMouseListener (this, false); // Listen for clicks

    // TODO: reenable helpTooltip when the support website is updated
    helpToolTip.setVisible (false);
    helpToolTip.setEnabled (false);

    setModeDisplay (ambiCreatorProcessor.isNormalLRFBMode());

    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);

    startTimer (100);
}

AmbiCreatorAudioProcessorEditor::~AmbiCreatorAudioProcessorEditor()
{
    valueTreeState.removeParameterListener ("outGainDb", this);
    valueTreeState.removeParameterListener ("horRotation", this);
    valueTreeState.removeParameterListener ("zGainDb", this);
    valueTreeState.removeParameterListener ("legacyMode", this);

    outGainGroup.slider.removeListener (this);
    horizontalRotationGroup.slider.removeListener (this);
    zGainGroup.slider.removeListener (this);

    cbAttOutChannelOrder = nullptr;
    outGainAttachment = nullptr;
    horizontalRotationAttachment = nullptr;
    zGainAttachment = nullptr;
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
        outputConfigGroup.comboBox.setSelectedId (order, NotificationType::dontSendNotification);
        updateOutputMeterLabelTexts();
    }
    else if (parameterID == "legacyMode")
    {
        bool legacyMode = newValue > 0.5f;
        DBG ("LegacyMode changed to: "
             << (legacyMode ? "ON" : "OFF") << ", Layer="
             << (ambiCreatorProcessor.abLayerState == eCurrentActiveLayer::layerA ? "A" : "B"));
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
    using namespace juce;

    g.fillAll (AAGuiComponents::Colours::mainBackground);
    //        g.drawImage(fourChannelModeImage, arrayImageArea, RectanglePlacement::centred);

    if (ambiCreatorProcessor.isNormalLRFBMode())
    {
        g.setOpacity (0.7f);
        g.drawImageWithin (legacyModeImage,
                           -40,
                           0,
                           arrayImageArea.getWidth() + 100,
                           getHeight(),
                           RectanglePlacement::onlyReduceInSize);
    }
    else
    {
        g.setOpacity (0.7f);
        g.drawImageWithin (fourChannelModeImage,
                           -40,
                           0,
                           arrayImageArea.getWidth() + 100,
                           getHeight(),
                           RectanglePlacement::onlyReduceInSize);
    }
}

void AmbiCreatorAudioProcessorEditor::resized()
{
    using namespace juce;

    constexpr auto leftRightMargin = 20;
    constexpr auto topMargin = 10;
    constexpr auto headerHeight = 50;
    constexpr auto footerHeight = 25;
    constexpr auto headerBottomMargin = 20;
    constexpr auto sliderWidth = 166;
    constexpr auto linearSliderVerticalSpacing = 40;
    constexpr auto meterWidth = 15;
    constexpr auto meterHeight = 180;
    constexpr auto meterSpacing = 2;
    constexpr auto meterToSliderSpacing = 30;
    constexpr auto arrayWidth = 200;

    constexpr auto buttonHeight = 25;
    constexpr auto legacyButtonWidth = 85;
    constexpr auto comboBoxHeight = 20;

    auto area = getLocalBounds();
    area.reduce (leftRightMargin, topMargin);

    Rectangle<int> footerArea (getLocalBounds().removeFromBottom ((int) footerHeight));
    helpToolTip.setBounds (5, getHeight() - 30, 40, 15);
    footer.setBounds (footerArea);

    Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    logo.setBounds (headerArea.removeFromLeft (140));
    headerArea.removeFromLeft (100);

    abComponent.setBounds (headerArea.removeFromLeft (160));

    headerArea.removeFromRight (15);

    tbLegacyMode.setBounds (headerArea.removeFromRight (legacyButtonWidth)
                                .withSizeKeepingCentre (legacyButtonWidth, buttonHeight));

    arrayImageArea = area.removeFromLeft (arrayWidth);

    // -------- MAIN AREA ---------
    area.removeFromTop (headerBottomMargin);

    constexpr auto contentWidth =
        8 * meterWidth + 2 * meterToSliderSpacing + 8 * meterSpacing + sliderWidth;
    const auto mainMarginLeft = (area.getWidth() - contentWidth) / 2;

    area.removeFromLeft (mainMarginLeft);

    auto inMeterArea =
        area.removeFromLeft ((4 * meterWidth + 4 * meterSpacing)).withHeight (meterHeight);

    inMeterArea = inMeterArea.withCentre (
        Point<int> (inMeterArea.getCentreX(), getLocalBounds().getHeight() / 2));

    for (int i = 0; i < 4; ++i)
    {
        inputMeter[i].setBounds (inMeterArea.removeFromLeft (meterWidth));
        inMeterArea.removeFromLeft (meterSpacing);
    }

    area.removeFromLeft (meterToSliderSpacing);

    auto sliderArea = area.removeFromLeft (sliderWidth);
    sliderArea = sliderArea.withCentre (
        Point<int> (sliderArea.getCentreX(), getLocalBounds().getHeight() / 2));
    sliderArea.reduce (0, 10);

    {
        constexpr int numGroups = 3; // Total groups
        constexpr int groupSpacing = 10; // Space between groups

        sliderArea.removeFromTop (linearSliderVerticalSpacing);

        // Height for each group including spacing
        const int totalSpacing = (numGroups - 1) * groupSpacing;
        const int groupHeight = (sliderArea.getHeight() - totalSpacing) / numGroups;

        // OUT GAIN GROUP
        outGainGroup.setBounds (sliderArea.removeFromTop (groupHeight).reduced (2));

        sliderArea.removeFromTop (groupSpacing); // Space before next group

        // Z GAIN GROUP
        zGainGroup.setBounds (sliderArea.removeFromTop (groupHeight).reduced (2));

        sliderArea.removeFromTop (groupSpacing); // Space before next group

        // HORIZONTAL ROTATION GROUP
        horizontalRotationGroup.setBounds (sliderArea.removeFromTop (groupHeight).reduced (2));
    }

    area.removeFromLeft (meterToSliderSpacing);

    Rectangle<int> outMeterArea =
        area.removeFromLeft (4 * meterWidth + 4 * meterSpacing).withHeight (meterHeight);

    outMeterArea = outMeterArea.withCentre (
        Point<int> (outMeterArea.getCentreX(), getLocalBounds().getHeight() / 2));

    outputConfigGroup.setBounds (outMeterArea.getX() - 7 * meterSpacing,
                                 outMeterArea.getY() - 4 * comboBoxHeight,
                                 4 * meterWidth + 20 * meterSpacing,
                                 60);

    for (int i = 0; i < 4; ++i)
    {
        outputMeter[i].setBounds (
            outMeterArea.removeFromLeft (meterWidth).withHeight (meterHeight));
        outMeterArea.removeFromLeft (meterSpacing);
    }
}

void AmbiCreatorAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    juce::ignoreUnused (slider);
}

void AmbiCreatorAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    using namespace juce;

    if (button == &tbLegacyMode)
    {
        bool isToggled = button->getToggleState();
        DBG ("tbLegacyMode clicked: Setting legacyMode to " << (isToggled ? "ON" : "OFF"));
        valueTreeState.getParameter ("legacyMode")->setValueNotifyingHost (isToggled ? 1.0f : 0.0f);
        setModeDisplay (ambiCreatorProcessor.isNormalLRFBMode());
    }
    else if (button == &abComponent[0] && ! button->getToggleState())
    {
        ambiCreatorProcessor.setAbLayer (eCurrentActiveLayer::layerB);
        setModeDisplay (ambiCreatorProcessor.isNormalLRFBMode());
    }
    else if (button == &abComponent[1] && ! button->getToggleState())
    {
        ambiCreatorProcessor.setAbLayer (eCurrentActiveLayer::layerA);
        setModeDisplay (ambiCreatorProcessor.isNormalLRFBMode());
    }
}

void AmbiCreatorAudioProcessorEditor::comboBoxChanged (juce::ComboBox* cb)
{
    if (cb == &outputConfigGroup.comboBox)
        updateOutputMeterLabelTexts();
}

void AmbiCreatorAudioProcessorEditor::updateOutputMeterLabelTexts()
{
    if (ambiCreatorProcessor.getChannelOrder() == eChannelOrder::ACN)
    {
        for (int i = 0; i < 4; ++i)
            outputMeter[i].setLabelText (outMeterLabelTextACN[i]);
    }
    else if (ambiCreatorProcessor.getChannelOrder() == eChannelOrder::FUMA)
    {
        for (int i = 0; i < 4; ++i)
            outputMeter[i].setLabelText (outMeterLabelTextFUMA[i]);
    }
}

void AmbiCreatorAudioProcessorEditor::timerCallback()
{
    using namespace juce;

    for (int i = 0; i < 4; ++i)
    {
        inputMeter[i].setLevel (ambiCreatorProcessor.inRms[i].get());
        outputMeter[i].setLevel (ambiCreatorProcessor.outRms[i].get());
    }

    tbLegacyMode.setToggleState (ambiCreatorProcessor.isNormalLRFBMode(),
                                 NotificationType::dontSendNotification);
    setModeDisplay (ambiCreatorProcessor.isNormalLRFBMode());
}

// implement this for AAX automation shortchut
int AmbiCreatorAudioProcessorEditor::getControlParameterIndex (Component& control)
{
    if (&control == &outGainGroup.slider)
        return 1;
    else if (&control == &horizontalRotationGroup.slider)
        return 2;
    else if (&control == &zGainGroup.slider)
        return 3;

    return -1;
}

void AmbiCreatorAudioProcessorEditor::setModeDisplay (bool legacyModeActive)
{
    if (legacyModeActive)
    {
        for (int i = 0; i < 4; ++i)
            inputMeter[i].setLabelText (inMeterLabelTextLegacy[i]);
    }
    else
    {
        for (int i = 0; i < 4; ++i)
            inputMeter[i].setLabelText (inMeterLabelText[i]);
    }
}
