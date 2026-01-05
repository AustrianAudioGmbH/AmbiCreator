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

#include "../lookAndFeel/BinaryFonts.h"
#include "Colours.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{
struct ComboBox : public juce::ComboBox
{
    ComboBox()
    {
        using namespace juce;

        setColour (ComboBox::backgroundColourId, Colours::groupComponentBackgroundColor);
        setColour (ComboBox::outlineColourId, Colours::textButtonActiveRedFrameColor);
        setColour (ComboBox::textColourId, Colours::mainTextColor);
        setColour (ComboBox::arrowColourId, Colours::mainTextColor);
        auto& laF = getLookAndFeel();
        laF.setColour (PopupMenu::backgroundColourId, Colours::groupComponentBackgroundColor);
        laF.setColour (PopupMenu::highlightedBackgroundColourId,
                       Colours::textButtonHoverRedBackgroundColor);
        laF.setColour (PopupMenu::textColourId, Colours::mainTextColor);

        laF.setDefaultSansSerifTypeface (
            Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansRegular_ttf,
                                               BinaryFonts::NunitoSansRegular_ttfSize));

        // TODO: consistently style the popup menu (text size, outline color, etc.)
    }
};

} // namespace AAGuiComponents
