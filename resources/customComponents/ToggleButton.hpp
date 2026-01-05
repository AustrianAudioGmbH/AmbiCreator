
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

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{
struct ToggleButton : public juce::ToggleButton
{
    ToggleButton()
    {
        setToggleable (true);
        setClickingTogglesState (true);
    }

    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override
    {
        using namespace juce;

        ignoreUnused (shouldDrawButtonAsDown);

        constexpr float margin = 2.0f;

        Rectangle toggleButtonBounds (0.0f,
                                      0.0f,
                                      static_cast<float> (getWidth()),
                                      static_cast<float> (getHeight()));

        toggleButtonBounds.reduce (margin, margin);

        int w = static_cast<int> (toggleButtonBounds.getWidth());

        g.setColour (isEnabled() ? Colours::textButtonActiveRedFrameColor
                                 : Colours::textButtonActiveRedFrameColor.withAlpha (0.5f));
        Path outline;

        outline.addRoundedRectangle (toggleButtonBounds,
                                     static_cast<float> (toggleButtonBounds.getHeight()) * 0.5f,
                                     static_cast<float> (toggleButtonBounds.getHeight()) * 0.5f);

        g.strokePath (outline, juce::PathStrokeType (2.0f));

        if (getToggleState())
        {
            if (shouldDrawButtonAsHighlighted)
                g.setColour (Colours::textButtonHoverRedBackgroundColor);
            else
                g.setColour (Colours::textButtonPressedRedBackgroundColor);
        }
        else
        {
            if (shouldDrawButtonAsHighlighted)
            {
                g.setColour (Colours::textButtonHoverRedBackgroundColor);
            }
            else
            {
                g.setColour (Colours::textButtonPressedRedBackgroundColor);
            }
        }

        g.fillPath (outline);

        const float newDiameter = toggleButtonBounds.getHeight() * 0.8f;

        Path p;

        if (getToggleState())
            p.addEllipse (static_cast<float> (w) - newDiameter,
                          margin + toggleButtonBounds.getHeight() / 2 - newDiameter / 2,
                          newDiameter,
                          newDiameter);
        else
            p.addEllipse (margin + 2,
                          margin + toggleButtonBounds.getHeight() / 2 - newDiameter / 2,
                          newDiameter,
                          newDiameter);

        g.setColour (isEnabled() ? Colours::mainTextColor : Colours::mainTextDisabledColor);
        g.fillPath (p);
    }
};
} // namespace AAGuiComponents
