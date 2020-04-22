#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../resources/customComponents/ImgPaths.h"

//==============================================================================
AafoaCreatorAudioProcessorEditor::AafoaCreatorAudioProcessorEditor (AafoaCreatorAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (EDITOR_WIDTH, EDITOR_HEIGHT);
    setLookAndFeel (&globalLaF);
    
    addAndMakeVisible (&title);
    title.setTitle (String("AustrianAudio"),String("FOACreator"));
    title.setFont (globalLaF.avenirMedium,globalLaF.avenirRegular);
    
    addAndMakeVisible (&footer);
    
    arrayImage = ImageCache::getFromMemory (arrayPng, arrayPngSize);
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
    
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom(footerHeight));
    footer.setBounds (footerArea);
    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    title.setTitleCentreX (headerArea.getCentreX());
    title.setBounds (headerArea);
    
    arrayImageArea = area.removeFromLeft(200).toFloat();
}
