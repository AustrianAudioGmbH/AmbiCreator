/*
 This file is based on ReverseSlider.h of the IEM plug-in suite.
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

/* Parts of this code originate from Yair Chuchem's AudioProcessorParameterSlider class:
 https://gist.github.com/yairchu */

#pragma once
#include "ImgPaths.h"
#include "RotarySlider.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

#define RS_FLT_EPSILON 1.19209290E-07F
class DirSlider : public AAGuiComponents::RotarySlider
{
public:
    DirSlider() :
        RotarySlider(),
        dirStripTop (this),
        dirStripBottom (this),
        lastDistanceFromDragStart (0),
        patternStripSize (12)
    {
        setTextBoxStyle (RotarySlider::NoTextBox, false, 60, 20); // use tooltipValueBox instead
        setSliderStyle (RotarySlider::RotaryHorizontalVerticalDrag);
        addAndMakeVisible (&dirStripTop);
        addAndMakeVisible (&dirStripBottom);
    }

    class DirPatternStrip : public Component
    {
    public:
        DirPatternStrip (DirSlider* newSlider) :
            Component(), dirImgSize (10), activePatternPath (-1.0f), slider (newSlider)
        {
            bCardPath.loadPathFromData (bCardData, sizeof (bCardData));
            cardPath.loadPathFromData (cardData, sizeof (cardData));
            sCardPath.loadPathFromData (sCardData, sizeof (sCardData));
            hCardPath.loadPathFromData (hCardData, sizeof (hCardData));
            eightPath.loadPathFromData (eightData, sizeof (eightData));
            omniPath.loadPathFromData (omniData, sizeof (omniData));
            //            revCardPath.loadPathFromData (cardData, sizeof (cardData));
            //            revCardPath.applyTransform (AffineTransform::rotation(M_PI));
            //            dirStripTop.setPatternPathsAndFactors(bCardPath, hCardPath, bCardFact, hCardFact);
        }

        void paint (juce::Graphics& g) override
        {
            using namespace juce;

            Rectangle<int> bounds = getLocalBounds();
            constexpr auto lrMargin = 7.f;
            constexpr auto topMargin = 1.0f;
            const auto boundsX = static_cast<float> (bounds.getX()) + lrMargin;
            const auto boundsY = static_cast<float> (bounds.getY()) + topMargin;
            const auto width = static_cast<float> (bounds.getWidth()) - 2 * lrMargin;

            leftPath.applyTransform (leftPath.getTransformToScaleToFit (boundsX,
                                                                        boundsY,
                                                                        dirImgSize,
                                                                        dirImgSize,
                                                                        true,
                                                                        Justification::centred));
            (slider->isEnabled()) ? g.setColour (Colours::white)
                                  : g.setColour (Colours::white.withMultipliedAlpha (0.5f));
            g.strokePath (
                leftPath,
                PathStrokeType (exactlyEqual (activePatternPath, patternFactorL) ? 2.0f : 1.0f));

            rightPath.applyTransform (
                rightPath.getTransformToScaleToFit (boundsX + width - dirImgSize,
                                                    boundsY,
                                                    dirImgSize,
                                                    dirImgSize,
                                                    true,
                                                    Justification::centred));
            g.strokePath (
                rightPath,
                PathStrokeType (exactlyEqual (activePatternPath, patternFactorR) ? 2.0f : 1.0f));
        }

        void setPatternPathsAndFactors (juce::Path pathL,
                                        juce::Path pathR,
                                        float factorL,
                                        float factorR)
        {
            leftPath = pathL;
            rightPath = pathR;

            patternFactorL = factorL;
            patternFactorR = factorR;
        }

        void mouseMove (const juce::MouseEvent& e) override
        {
            using namespace juce;

            if (! slider->isEnabled())
                return;

            Point<float> posf = e.getPosition().toFloat();
            float oldActivePath = activePatternPath;
            activePatternPath = -1;

            // highlight active polar pattern path
            if (leftPath.getBounds().contains (posf))
                activePatternPath = patternFactorL;
            else if (rightPath.getBounds().contains (posf))
                activePatternPath = patternFactorR;
            //            else if (cardPath.getBounds().contains(posf)) activePatternPath = cardFact;
            //            else if (revCardPath.getBounds().contains(posf)) activePatternPath = revCardFact;

            if (! exactlyEqual (oldActivePath, activePatternPath))
                repaint();
        }

        void mouseUp (const juce::MouseEvent& e) override
        {
            using namespace juce;

            ignoreUnused (e);

            if (! slider->isEnabled())
                return;

            if (! exactlyEqual (activePatternPath, -1.0f))
            {
                slider->setValue (activePatternPath, NotificationType::sendNotification);
            }
        }

        void mouseExit (const juce::MouseEvent& e) override
        {
            ignoreUnused (e);
            activePatternPath = -1;
            repaint();
        }

    private:
        float dirImgSize;
        float activePatternPath;

        float patternFactorL;
        float patternFactorR;

        juce::Path bCardPath;
        juce::Path cardPath;
        juce::Path sCardPath;
        juce::Path hCardPath;
        juce::Path eightPath;
        juce::Path omniPath;

        juce::Path leftPath;
        juce::Path rightPath;

        DirSlider* slider;
    };

    void paint (juce::Graphics& g) override { RotarySlider::paintInBounds (g, sliderRect); }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.eventComponent != this)
            return; // mouseEvent started from tooltipValueBox

        lastDistanceFromDragStart = 0;
        Slider::mouseDown (e);
    }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (e.eventComponent != this)
            return;

        Slider::mouseDrag (e);
    }

    void resized() override
    {
        sliderRect = getLocalBounds();
        sliderRect.removeFromTop (static_cast<int> (patternStripSize));
        sliderRect.removeFromBottom (patternStripSize);

        dirPatternTopBounds = getLocalBounds().removeFromTop (patternStripSize);
        dirStripTop.setBounds (dirPatternTopBounds);

        dirPatternBottomBounds = getLocalBounds().removeFromBottom (patternStripSize);
        dirStripBottom.setBounds (dirPatternBottomBounds);
    }

    double snapValueToRange (double attemptedValue)
    {
        return attemptedValue < getMinimum()
                   ? getMinimum()
                   : (attemptedValue > getMaximum() ? getMaximum() : attemptedValue);
    }

    DirPatternStrip dirStripTop;
    DirPatternStrip dirStripBottom;

private:
    int lastDistanceFromDragStart;
    juce::Rectangle<int> sliderRect;
    juce::Rectangle<int> dirPatternTopBounds;
    juce::Rectangle<int> dirPatternBottomBounds;

    int patternStripSize;
};
