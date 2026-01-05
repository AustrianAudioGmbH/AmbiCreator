/*
 ==============================================================================
 Author: Sebastian Grill
 
 Copyright (c) 2025 - Austrian Audio GmbH
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

#include "Colours.hpp"

#include "../lookAndFeel/BinaryFonts.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{
class GroupComponent : public juce::GroupComponent
{
protected:
    static constexpr auto textMarginLeft = 10;
    static constexpr auto textMarginTop = 20;
    static constexpr auto textSize = 18;

public:
    GroupComponent() {}

    void paint (juce::Graphics& g) override
    {
        using namespace juce;

        auto area = getLocalBounds().toFloat();
        g.setColour (Colours::groupComponentBackgroundColor);
        g.fillRoundedRectangle (area, 10.0f);

        const auto font = Font (FontOptions {
            Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansSemiBold_ttf,
                                               BinaryFonts::NunitoSansSemiBold_ttfSize) }
                                    .withHeight (textSize));

        g.setFont (font);
        g.setColour (isEnabled() ? Colours::mainTextColor : Colours::mainTextDisabledColor);
        const auto componentText = getText();
        const auto labelJustification = getTextLabelPosition();
        const auto textArea =
            getLocalBounds().removeFromTop (textMarginTop + textSize).reduced (textMarginLeft, 0);
        g.drawFittedText (componentText, textArea, labelJustification, 1);
    }
};
} // namespace AAGuiComponents
