/*
 This file is based on DirectivityVisualizer.h of the IEM plug-in suite.
 Modifications by Thomas Deppisch.
 */

/*
 ==============================================================================
 Author: Thomas Deppisch
 
 Copyright (c) 2019 - Austrian Audio GmbH
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
#include <numbers>

//==============================================================================
/*
*/
//using namespace dsp;
class FirstOrderDirectivityVisualizer : public juce::Component
{
    const float deg2rad = std::numbers::pi_v<float> / 180.0f;
    const int degStep = 4;

public:
    FirstOrderDirectivityVisualizer()
    {
        using namespace juce;

        isActive = true;
        //        soloButton = nullptr;
        //        muteButton = nullptr;
        soloActive = false;
        rotation = 0.0f;
        patternAlpha = 1.0f;

        colour = Colour (0xFFD0011B);

        for (int phi = -180; phi <= 180; phi += degStep)
        {
            const auto angle = static_cast<float> (phi) * deg2rad;
            pointsOnCircle.add (Point<float> (std::cos (angle), std::sin (angle)));
        }

        Path circle;
        circle.addEllipse (-1.0f, -1.0f, 2.0f, 2.0f);
        Path line;
        line.startNewSubPath (0.0f, -1.0f);
        line.lineTo (0.0f, 1.0f);

        grid.clear();
        grid.addPath (circle);

        subGrid.clear();
        for (int i = 1; i < 5; i++)
            subGrid.addPath (circle, AffineTransform().scaled (static_cast<float> (i) / 4.0f));

        subGrid.addPath (line);
        subGrid.addPath (line, AffineTransform().rotation (0.25f * std::numbers::pi_v<float>));
        subGrid.addPath (line, AffineTransform().rotation (0.5f * std::numbers::pi_v<float>));
        subGrid.addPath (line, AffineTransform().rotation (0.75f * std::numbers::pi_v<float>));
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;

        Path path;

        if (shouldDrawGrid)
        {
            path = grid;
            path.applyTransform (transform);
            g.setColour (AAGuiComponents::Colours::mainBackground);
            g.fillPath (path);

            path = subGrid;
            path.applyTransform (transform);
            g.setColour (AAGuiComponents::Colours::polarVisualizerGrid);
            g.strokePath (path, PathStrokeType (1.0f));
        }

        // draw directivity
        g.setColour (colour.withMultipliedAlpha (! isActive ? 0.0f : patternAlpha));
        path.clear();

        int idx = 0;
        for (int phi = -180; phi <= 180; phi += degStep)
        {
            float phiInRad = (float) phi * deg2rad + rotation;
            float gainLin = std::abs ((1 - std::abs (dirWeight)) + dirWeight * std::cos (phiInRad));
            const float dbMin = 25;
            float gainDb =
                20
                * std::log10 (
                    std::max (gainLin, static_cast<float> (std::pow (10, -dbMin / 20.0f))));
            float effGain = std::max (std::abs ((gainDb + dbMin) / dbMin), 0.01f);
            Point<float> point = effGain * pointsOnCircle[idx];

            if (phi == -180)
                path.startNewSubPath (point);
            else
                path.lineTo (point);
            ++idx;
        }

        path.closeSubPath();
        path.applyTransform (transform);
        g.strokePath (path, PathStrokeType (2.0f));
    }

    void resized() override
    {
        using namespace juce;

        Rectangle<int> bounds = getLocalBounds();
        Point<int> centre = bounds.getCentre();

        bounds.reduce (10, 10);

        if (bounds.getWidth() > bounds.getHeight())
            bounds.setWidth (bounds.getHeight());
        else
            bounds.setHeight (bounds.getWidth());
        bounds.setCentre (centre);

        transform = AffineTransform::fromTargetPoints (static_cast<float> (centre.x),
                                                       static_cast<float> (centre.y),
                                                       static_cast<float> (centre.x),
                                                       static_cast<float> (bounds.getY()),
                                                       static_cast<float> (bounds.getX()),
                                                       static_cast<float> (centre.y));

        plotArea = bounds;
    }

    void setDirWeight (float weight)
    {
        dirWeight = weight;
        repaint();
    }

    void setActive (bool active)
    {
        if (isActive != active)
        {
            isActive = active;
            repaint();
        }
    }

    //    void setMuteSoloButtons(MuteSoloButton* solo, MuteSoloButton* mute)
    //    {
    //        soloButton = solo;
    //        muteButton = mute;
    //    }

    void setPatternAlpha (float newAlpha) { patternAlpha = newAlpha; }

    //    float calcAlpha()
    //    {
    //        if ((soloButton == nullptr || !soloActive) && (muteButton == nullptr || !muteButton->getToggleState()))
    //        {
    //            return 1.0f;
    //        }
    //        else if ((soloActive && soloButton->getToggleState()) || (!soloActive && !muteButton->getToggleState()))
    //        {
    //            return 1.0f;
    //        }
    //        else
    //        {
    //            return 0.4f;
    //        }
    //    }
    //
    //    void setSoloActive (bool set)
    //    {
    //        soloActive = set;
    //    }
    //
    void setColour (juce::Colour newColour) { colour = newColour; }

    void setPatternRotation (float degrees) { rotation = degrees * deg2rad; }

    void shouldDrawGridLines (bool draw) { shouldDrawGrid = draw; }

private:
    juce::Path grid;
    juce::Path subGrid;
    juce::AffineTransform transform;
    juce::Rectangle<int> plotArea;
    float dirWeight;
    float patternAlpha;
    bool isActive;
    bool shouldDrawGrid = true;
    //    MuteSoloButton* soloButton;
    //    MuteSoloButton* muteButton;
    bool soloActive;
    juce::Colour colour;

    float rotation;

    juce::Array<juce::Point<float>> pointsOnCircle;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderDirectivityVisualizer)
};
