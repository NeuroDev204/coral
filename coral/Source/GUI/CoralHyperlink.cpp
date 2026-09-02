/*
Coral
Copyright (C) 2025  Coral

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <JuceHeader.h>
#include "CoralHyperlink.h"

//==============================================================================
CoralHyperlink::CoralHyperlink()
{
}

void CoralHyperlink::paintButton(Graphics& g, bool, bool)
{
    auto& theme = dynamic_cast<CoralTheme&>(LookAndFeel::getDefaultLookAndFeel());
    const Colour text_colour = theme.getCurrentColourScheme().getUIColour(LookAndFeel_V4::ColourScheme::defaultText);

	if (isEnabled())
		g.setColour(text_colour);
	else
		g.setColour(text_colour.withMultipliedAlpha(0.4f));
	
	auto text_font = theme.getNormalFont();
	text_font.setUnderline(true);
	g.setFont(text_font);

	g.drawText(getButtonText(), getLocalBounds(), getJustificationType(), true);
}

int CoralHyperlink::getTextWidth()
{
	auto& theme = dynamic_cast<CoralTheme&>(LookAndFeel::getDefaultLookAndFeel());
	auto font = theme.getNormalFont();
	font.setUnderline(true);
	return font.getStringWidth(getButtonText());
}