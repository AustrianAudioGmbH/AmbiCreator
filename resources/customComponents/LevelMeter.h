/*
 ==============================================================================
 Author: Thomas Deppisch
 
 Copyright (c) 2020 - Austrian Audio GmbH
 www.austrian.audio
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
*/

#pragma once

#include "../lookAndFeel/BinaryFonts.h"
#include "Colours.hpp"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
*/
namespace AAGuiComponents
{
class LevelMeter : public juce::Component
{
public:
    LevelMeter()
    {
        using namespace juce;

        colour = juce::Colours::black;
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;

        auto bounds = getLocalBounds();
        auto labelWidth = bounds.getWidth();
        auto labelHeight = labelWidth;
        auto labelBounds = bounds.removeFromBottom (labelHeight);
        g.setColour (juce::Colours::white);
        auto fontOptions = FontOptions (Typeface::createSystemTypefaceFor (
                                            BinaryFonts::NunitoSansSemiBold_ttf,
                                            BinaryFonts::NunitoSansSemiBold_ttfSize))
                               .withHeight (static_cast<float> (labelHeight));
        g.setFont (fontOptions);
        g.setColour (AAGuiComponents::Colours::mainTextColor);

        g.drawText (labelText, labelBounds, Justification::centred);

        auto labelMargin = 6;
        bounds.removeFromBottom (labelMargin);
        g.setColour (AAGuiComponents::Colours::mainTextInactiveColor);
        g.drawRoundedRectangle (bounds.toFloat(), 4.0f, 2.0f);

        g.setColour (colour);
        auto innerBounds = bounds.reduced (1).toFloat();
        auto newHeight = innerBounds.getHeight() * (1.0f - normalizedMeterHeight);
        g.fillRoundedRectangle (innerBounds.withTop (newHeight), 2.0f);
    }

    void resized() override {}

    void setLevel (float newLevel)
    {
        using namespace juce;

        float levelDb = juce::Decibels::gainToDecibels (newLevel, minDb);
        normalizedMeterHeight = (minDb - levelDb) / minDb;
        repaint();
    }

    void setColour (juce::Colour newColour) { colour = newColour; }

    void setLabelText (juce::String newText)
    {
        labelText = newText;
        repaint();
    }

private:
    float normalizedMeterHeight = 0.0f;
    juce::Colour colour;
    juce::String labelText = "";
    const float minDb = -60.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
} // namespace AAGuiComponents
