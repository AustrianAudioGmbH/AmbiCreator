/*
 ==============================================================================
 Author: Sebastian Grill
 
 Copyright (c) 2026 - Austrian Audio GmbH
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

#include "../../resources/customComponents/ComboBox.hpp"
#include "../../resources/customComponents/GroupComponent.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{
struct OutputConfig : public GroupComponent
{
    OutputConfig()
    {
        setText ("Output Config");

        addAndMakeVisible (comboBox);
        comboBox.addItemList ({ "AmbiX", "FUMA" }, 1);
        comboBox.setEditableText (false);
    }

    void resized() override
    {
        using namespace juce;

        auto area = getLocalBounds().reduced (10);
        area.removeFromTop (textMarginTop);
        comboBox.setBounds (area.removeFromTop (20));
    }

    ComboBox comboBox;
};
} // namespace AAGuiComponents
