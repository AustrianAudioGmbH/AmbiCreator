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

#include "BinaryData.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{
struct AALogo : public juce::Component, public juce::SettableTooltipClient
{
    AALogo()
    {
        using namespace juce;

        setTooltip (String (AA_BUILD_TAG) + String ("-") + String (AA_BUILD_COMMIT_HASH)
                    + String::formatted (" (JUCE:%s)", AA_BUILD_JUCE_VERSION)
                    + String::formatted (" (%p)", this));
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;

        Rectangle area (0.0f,
                        0.0f,
                        static_cast<float> (getWidth()),
                        static_cast<float> (getHeight()));

        static auto img = juce::Drawable::createFromImageData (BinaryData::AmbiCreatorLogo_svg,
                                                               BinaryData::AmbiCreatorLogo_svgSize);
        img->drawWithin (g, area, juce::RectanglePlacement::xLeft, 1.0f);
    }
};
} // namespace AAGuiComponents
