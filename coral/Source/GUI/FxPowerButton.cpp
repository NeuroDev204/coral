/*
FxSound
Copyright (C) 2025  FxSound LLC

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

#include "FxPowerButton.h"
#include "FxTheme.h"

FxPowerButton::FxPowerButton(const String &button_name) : DrawableButton(button_name, DrawableButton::ButtonStyle::ImageFitted)
{
	power_state_ = false;

	power_on_image_ = Drawable::createFromImageData(FXIMAGE(PowerOnButton), FXIMAGESIZE(PowerOnButton));
	power_off_image_ = Drawable::createFromImageData(FXIMAGE(PowerOffButton), FXIMAGESIZE(PowerOffButton));
}

void FxPowerButton::paint(Graphics& g)
{
	auto r = getLocalBounds().toFloat().reduced(1.5f);
	const bool on = power_state_ && isEnabled();

	g.setColour(on ? Colour(FXCOLOR(HighlightedFill)).withAlpha(0.88f)
		: Colours::white.withAlpha(isMouseOver(true) ? 0.14f : 0.08f));
	g.fillEllipse(r);
	g.setColour(on ? Colours::white.withAlpha(0.35f) : Colours::white.withAlpha(0.28f));
	g.drawEllipse(r.reduced(0.4f), 1.1f);

	auto c = r.getCentre();
	const float radius = r.getWidth() * 0.22f;
	Path stem;
	stem.startNewSubPath(c.x, c.y - radius - 3.0f);
	stem.lineTo(c.x, c.y - 1.0f);
	g.setColour(on ? Colours::white : Colour(FXCOLOR(DefaultText)).withAlpha(0.92f));
	g.strokePath(stem, PathStrokeType(2.0f, PathStrokeType::curved, PathStrokeType::rounded));

	Path arc;
	arc.addCentredArc(c.x, c.y + 0.6f, radius + 1.2f, radius + 1.2f, 0.0f,
		MathConstants<float>::pi * 0.72f, MathConstants<float>::pi * 2.28f, true);
	g.strokePath(arc, PathStrokeType(2.0f, PathStrokeType::curved, PathStrokeType::rounded));
}

bool FxPowerButton::keyPressed(const KeyPress& key)
{
	if (isEnabled() && key.isKeyCode(KeyPress::spaceKey))
	{
		triggerClick();
		return true;
	}

	return false;
}

void FxPowerButton::lookAndFeelChanged()
{
	power_on_image_ = Drawable::createFromImageData(FXIMAGE(PowerOnButton), FXIMAGESIZE(PowerOnButton));
	power_off_image_ = Drawable::createFromImageData(FXIMAGE(PowerOffButton), FXIMAGESIZE(PowerOffButton));
	repaint();
}

bool FxPowerButton::getPowerState()
{
	return power_state_;
}

void FxPowerButton::setPowerState(bool power_state)
{
	power_state_ = power_state;
	repaint();
}

int FxPowerButton::getImageWidth()
{
	return image_width_;
}

void FxPowerButton::setImageWidth(int image_width)
{
	image_width_ = image_width;
}