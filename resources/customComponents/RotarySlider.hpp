/*
 This file is based on IEM_LaF.h of the IEM plug-in suite.
 Modifications by Sebastian Grill.
*/

/*
 ==============================================================================
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

/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#include "Colours.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{

struct RotarySlider : public juce::Slider
{
    RotarySlider()
    {
        setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void paint (juce::Graphics& g) override { paintInBounds (g, getLocalBounds()); }

    void paintInBounds (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        using namespace juce;

        auto width = bounds.getWidth();
        auto height = bounds.getHeight();

        const auto tbPosition = getTextBoxPosition();

        if (tbPosition == TextEntryBoxPosition::TextBoxAbove
            || tbPosition == TextEntryBoxPosition::TextBoxBelow)
        {
            height -= getTextBoxHeight() + 2;
        }
        else if (tbPosition == TextEntryBoxPosition::TextBoxLeft
                 || tbPosition == TextEntryBoxPosition::TextBoxRight)
        {
            width -= getTextBoxWidth() + 2;
        }

        const auto x = bounds.getX();
        const auto y = bounds.getY();

        const bool enabled = isEnabled();
        const float alpha = enabled ? 1.0f : 0.4f;
        const auto radius = static_cast<const float> (jmin (width / 2, height / 2));
        const float centreX = static_cast<float> (x) + static_cast<float> (width) * 0.5f;
        const float centreY = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
        const float rx = centreX - radius;
        const float ry = centreY - radius;
        const float rw = radius * 2.0f;

        const auto rotaryParams = getRotaryParameters();
        const auto rotaryStartAngle = rotaryParams.startAngleRadians;
        const auto rotaryEndAngle = rotaryParams.endAngleRadians;
        const auto sliderPos = static_cast<float> (valueToProportionOfLength (getValue()));

        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        const float bedThickness = 2.0f;
        const float bedOutline = 1.4f;
        const float statusOutline = 1.6f;
        const float extraMargin = 1.0f;

        const float pointerThickness = 1.2f;
        const float pointerLength =
            (radius - extraMargin - statusOutline - bedOutline - bedThickness - 1.0f) * 0.8f;

        Path p, q, a;
        Rectangle<float> r = Rectangle<float> (rx, ry, rw, rw);

        const bool isMouseOver = isMouseOverOrDragging() && isEnabled();

        const Colour statusColour = findColour (Slider::rotarySliderOutlineColourId);
        //status ring
        g.setColour (statusColour.withMultipliedAlpha (alpha));

        a.addCentredArc (centreX,
                         centreY,
                         radius - extraMargin,
                         radius - extraMargin,
                         0.0f,
                         rotaryStartAngle,
                         angle,
                         true);
        // if (isDual)
        //     a.addCentredArc (centreX,
        //                      centreY,
        //                      radius - extraMargin,
        //                      radius - extraMargin,
        //                      0.0f,
        //                      negAngle,
        //                      zeroAngle,
        //                      true);

        g.strokePath (a, PathStrokeType (statusOutline));

        //bed ellipse
        g.setColour (Colours::ClFaceShadow);
        g.fillEllipse (r.reduced (extraMargin + statusOutline));

        //(isMouseOver)?g.setColour(ClFaceShadowOutlineActive) : g.setColour (ClFaceShadowOutline);
        (isMouseOver) ? g.setColour (statusColour.withMultipliedAlpha (0.4f))
                      : g.setColour (Colours::ClFaceShadowOutline);
        g.drawEllipse (r.reduced (extraMargin + statusOutline), bedOutline);

        //knob
        g.setColour (Colours::ClFace.withMultipliedAlpha (alpha));
        g.fillEllipse (r.reduced (extraMargin + statusOutline + bedOutline + bedThickness));
        g.setColour (statusColour.withMultipliedAlpha (alpha));
        g.drawEllipse (r.reduced (extraMargin + statusOutline + bedOutline + bedThickness),
                       statusOutline);

        g.setColour (Colours::ClRotSliderArrowShadow.withMultipliedAlpha (alpha));
        g.drawEllipse (r.reduced (extraMargin + statusOutline + bedOutline + bedThickness + 1.0f),
                       1.0f);

        q.addRectangle (pointerThickness * 0.3f, -radius + 6.0f, pointerThickness, pointerLength);
        q.applyTransform (AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (Colours::ClRotSliderArrowShadow.withMultipliedAlpha (alpha));
        g.fillPath (q);

        p.addRectangle (-pointerThickness * 0.5f, -radius + 6.0f, pointerThickness, pointerLength);
        p.applyTransform (AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (Colours::ClRotSliderArrow.withMultipliedAlpha (alpha));
        g.fillPath (p);
    }
};
} // namespace AAGuiComponents
