/*
 This file is based on TitleBar.h of the IEM plug-in suite.
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

namespace AAGuiComponents
{
class Footer : public juce::Component
{
public:
    Footer() : Component() {}
    ~Footer() override {}

    void paint (juce::Graphics& g) override
    {
        using namespace juce;

        Rectangle<int> bounds = getLocalBounds();
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        const auto options = FontOptions().withTypeface (
            Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansRegular_ttf,
                                               BinaryFonts::NunitoSansRegular_ttfSize));
        g.setFont (options);
        g.setFont (14.0f);
        String versionString = "v";

#if JUCE_DEBUG
        versionString = "DEBUG - v";
#endif
        versionString.append (JucePlugin_VersionString, 6);

        g.drawText (versionString,
                    0,
                    0,
                    bounds.getWidth() - 8,
                    bounds.getHeight() - 2,
                    Justification::bottomRight);
    }

    void resized() override {}

private:
};
} // namespace AAGuiComponents
