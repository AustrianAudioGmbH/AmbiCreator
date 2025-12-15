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

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
*/
class SimpleLabel : public juce::Component, public juce::SettableTooltipClient
{
public:
    SimpleLabel()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
    }
    ~SimpleLabel() {}

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

    void enablementChanged() override { repaint(); };

    void paint (juce::Graphics& g) override
    {
        using namespace juce;

        Rectangle<int> bounds = getLocalBounds();
        paintSimpleLabel (g, bounds, text, isBold, justification);
    }

    virtual void paintSimpleLabel (juce::Graphics& g,
                                   juce::Rectangle<int> bounds,
                                   juce::String text,
                                   bool isBold,
                                   juce::Justification justification)
    {
        using namespace juce;

        g.setColour (colour.withMultipliedAlpha (this->isEnabled() ? 1.0f : 0.4f));
        g.setFont (getLookAndFeel().getTypefaceForFont (Font (bounds.getHeight(), isBold ? 1 : 0)));
        g.setFont (bounds.getHeight());
        g.drawText (text, bounds, justification, true);
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

    TripleLabel (juce::LeakedObjectDetector<TripleLabel> leakDetector212,
                 bool leftBold,
                 bool middleBold,
                 bool rightBold) :
        leftBold (leftBold), middleBold (middleBold), rightBold (rightBold)
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
                                   juce::String leftText,
                                   juce::String middleText,
                                   juce::String rightText,
                                   bool leftBold,
                                   bool middleBold,
                                   bool lrightBold)
    {
        using namespace juce;

        g.setColour (Colours::white);
        Font tempFont;
        tempFont.setHeight (bounds.getHeight());
        int height = bounds.getHeight();

        tempFont.setStyleFlags (leftBold ? 1 : 0);
        g.setFont (getLookAndFeel().getTypefaceForFont (tempFont));
        g.setFont (height);
        g.drawText (leftText, bounds, Justification::left, true);

        tempFont.setStyleFlags (middleBold ? 1 : 0);
        g.setFont (getLookAndFeel().getTypefaceForFont (tempFont));
        g.setFont (height + (middleBold ? 2 : 0));
        g.drawText (middleText, bounds, Justification::centred, true);

        tempFont.setStyleFlags (rightBold ? 1 : 0);
        g.setFont (getLookAndFeel().getTypefaceForFont (tempFont));
        g.setFont (height);
        g.drawText (rightText, bounds, Justification::right, true);
    }

    void resized() override {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TripleLabel)
    juce::String leftText = "";
    juce::String middleText = "";
    juce::String rightText = "";
    bool leftBold, middleBold, rightBold;
};
