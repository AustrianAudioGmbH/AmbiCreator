/*
 This file is based on SimpleLabel.h of the IEM plug-in suite.
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

#include "../lookAndFeel/BinaryFonts.h"

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
*/
class SimpleLabel : public juce::Component
{
public:
    SimpleLabel()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
    }
    ~SimpleLabel() override {}

    void setText (juce::String newText)
    {
        text = newText;
        repaint();
    }
    void setText (juce::String newText, bool newBold)
    {
        text = newText;
        isBold = newBold;
        repaint();
    }
    void setText (juce::String newText, bool newBold, juce::Justification newJustification)
    {
        text = newText;
        isBold = newBold;
        justification = newJustification;
        repaint();
    }

    void setJustification (juce::Justification newJustification)
    {
        justification = newJustification;
        repaint();
    }

    void setTextColour (const juce::Colour newColour)
    {
        if (colour != newColour)
        {
            colour = newColour;
            repaint();
        }
    }

    void enablementChanged() override { repaint(); }

    void paint (juce::Graphics& g) override
    {
        juce::Rectangle<int> bounds = getLocalBounds();
        paintSimpleLabel (g, bounds, text, isBold, justification);
    }

    virtual void paintSimpleLabel (juce::Graphics& g,
                                   juce::Rectangle<int> bounds,
                                   juce::String labelText,
                                   bool isLabelBold,
                                   juce::Justification labelJustification)
    {
        using namespace juce;

        g.setColour (colour.withMultipliedAlpha (this->isEnabled() ? 1.0f : 0.4f));
        g.setFont (static_cast<float> (bounds.getHeight()));

        Font font (FontOptions {});

        if (isLabelBold)
            font = Font (FontOptions (
                juce::Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansRegular_ttf,
                                                         BinaryFonts::NunitoSansRegular_ttfSize)));
        else
            font = Font (FontOptions (
                juce::Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansSemiBold_ttf,
                                                         BinaryFonts::NunitoSansSemiBold_ttfSize)));

        font.setHeight (static_cast<float> (bounds.getHeight()));
        g.setFont (font);
        g.drawText (labelText, bounds, labelJustification, true);
    }

    void resized() override {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleLabel)
    juce::String text = "";
    bool isBold = false;
    juce::Colour colour = juce::Colours::white;
    juce::Justification justification = juce::Justification::centred;
};

//==============================================================================
/*
 */
class TripleLabel : public juce::Component
{
public:
    TripleLabel()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
    }

    TripleLabel (bool newLeftBold, bool newMiddleBold, bool newRightBold) :
        leftBold (newLeftBold), middleBold (newMiddleBold), rightBold (newRightBold)
    {
    }
    ~TripleLabel() override {}

    void setText (juce::String newLeftText,
                  juce::String newMiddleText,
                  juce::String newRightText,
                  bool newLeftBold,
                  bool newMiddleBold,
                  bool newRightBold)
    {
        leftText = newLeftText;
        middleText = newMiddleText;
        rightText = newRightText;
        leftBold = newLeftBold;
        middleBold = newMiddleBold;
        rightBold = newRightBold;

        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;

        Rectangle<int> bounds = getLocalBounds();
        paintTripleLabel (g,
                          bounds,
                          leftText,
                          middleText,
                          rightText,
                          leftBold,
                          middleBold,
                          rightBold);
    }

    virtual void paintTripleLabel (juce::Graphics& g,
                                   juce::Rectangle<int> bounds,
                                   juce::String newLeftText,
                                   juce::String newMiddleText,
                                   juce::String newRightText,
                                   bool newLeftBold,
                                   bool newMiddleBold,
                                   bool newRightBold)
    {
        using namespace juce;

        ignoreUnused (newRightBold);

        g.setColour (Colours::white);
        Font tempFont (FontOptions {});

        tempFont.setHeight (static_cast<float> (bounds.getHeight()));
        auto height = static_cast<float> (bounds.getHeight());

        tempFont.setStyleFlags (newLeftBold ? 1 : 0);
        g.setFont (tempFont);
        g.setFont (height * 1.0f);
        g.drawText (newLeftText, bounds, Justification::left, true);

        tempFont.setStyleFlags (newMiddleBold ? 1 : 0);
        g.setFont (tempFont);
        g.setFont ((height * 1.0f) + (newMiddleBold ? 2.0f : 0.0f));
        g.drawText (newMiddleText, bounds, Justification::centred, true);

        tempFont.setStyleFlags (rightBold ? 1 : 0);
        g.setFont (tempFont);
        g.setFont (height * 1.0f);
        g.drawText (newRightText, bounds, Justification::right, true);
    }

    void resized() override {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TripleLabel)
    juce::String leftText = "";
    juce::String middleText = "";
    juce::String rightText = "";
    bool leftBold, middleBold, rightBold;
};
