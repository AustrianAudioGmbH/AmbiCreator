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

#include "../../resources/customComponents/GroupComponent.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{

template <typename SliderType>
struct RotarySliderGroup : public GroupComponent
{
    RotarySliderGroup()
    {
        using namespace juce;
        addAndMakeVisible (slider);
        setTextLabelPosition (Justification::centred);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10);
        area.removeFromTop (textMarginTop);
        slider.setBounds (area);
    }

    SliderType slider;
};

} // namespace AAGuiComponents
