/*
 ==============================================================================
 Author: Thomas Deppisch
 
 Copyright (c) 2020 - Austrian Audio GmbH
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

//==============================================================================
/*
*/
class LevelMeter : public Component
{
public:
    LevelMeter()
    {
        colour = Colours::black;
    }
    ~LevelMeter()
    {
    }

    void paint (Graphics& g) override
    {
        auto bounds = getLocalBounds();
        float labelWidth = bounds.getWidth();
        float labelHeight = labelWidth;
        auto labelBounds = bounds.removeFromBottom(labelHeight);
        g.setColour(Colours::white);
        g.setFont(getLookAndFeel().getTypefaceForFont(Font(bounds.getHeight())));
        g.drawText(labelText, labelBounds, Justification::centred);
        
        float labelMargin = 6.0f;
        bounds.removeFromBottom(labelMargin);
        g.setColour(Colours::black);
        g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 2.0f);
        
        g.setColour(colour);
        auto innerBounds = bounds.reduced(1).toFloat();
        auto newHeight = innerBounds.getHeight() * (1.0f - normalizedMeterHeight);
        g.fillRoundedRectangle(innerBounds.withTop(newHeight), 2.0f);
    }

    void resized() override
    {
    }
    
    void setLevel(float newLevel)
    {
        float levelDb = Decibels::gainToDecibels(newLevel, minDb);
        normalizedMeterHeight = (minDb - levelDb) / minDb;
        repaint();
    }
    
    void setColour(Colour newColour)
    {
        colour = newColour;
    }
    
    void setLabelText(juce::String newText)
    {
        labelText = newText;
        repaint();
    }
    
private:
    float normalizedMeterHeight = 0.0f;
    Colour colour;
    juce::String labelText = "";
    const float minDb = -60.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
