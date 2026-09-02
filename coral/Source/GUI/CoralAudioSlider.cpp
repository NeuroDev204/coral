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
#include "CoralController.h"
#include "CoralTheme.h"
#include "CoralAudioSlider.h"

//==============================================================================
CoralAudioSlider::CoralAudioSlider(String label_format, float default_value) : default_value_(default_value)
{
	label_format_ = label_format;

	setValue(default_value);
	setScrollWheelEnabled(false);
	setMouseCursor(MouseCursor::PointingHandCursor);

	auto& theme = dynamic_cast<CoralTheme&>(getLookAndFeel());

	value_label_.setFont(theme.getNormalFont().withHeight(12.0f));
	value_label_.setJustificationType(Justification::centredLeft);
	addAndMakeVisible(value_label_);

	setWantsKeyboardFocus(true);
}

void CoralAudioSlider::resized()
{
	Slider::resized();

	// Always update label position & text after layout
	updateLabel();
}

void CoralAudioSlider::valueChanged()
{
	updateLabel();
}

bool CoralAudioSlider::keyPressed(const KeyPress& key)
{
	if (isEnabled())
	{
		if (key.isKeyCode(KeyPress::upKey))
		{
			setValue(getValue() + getInterval());
			return true;
		}
		else if (key.isKeyCode(KeyPress::downKey))
		{
			setValue(getValue() - getInterval());
			return true;
		}
	}

	return false;
}

void CoralAudioSlider::mouseDown(const juce::MouseEvent& event)
{
	// Check if right mouse button is pressed
	if (event.mods.isRightButtonDown())
	{
		// Reset the slider to 0
		setValue(default_value_, NotificationType::sendNotification);
	}
	else
	{
		// Default behavior for other buttons
		Slider::mouseDown(event);
	}
}

void CoralAudioSlider::updateLabel()
{
	auto value = getValue();

	// Update label text
	auto text = String::formatted(label_format_, value);
	value_label_.setText(text, NotificationType::dontSendNotification);

	// Position label based on slider thumb position
	auto pos = getPositionOfValue(value);
	auto x = pos + CoralTheme::SLIDER_THUMB_RADIUS / 2 + 1;
	value_label_.setBounds(x, (getHeight() - LABEL_HEIGHT) / 2, LABEL_WIDTH, LABEL_HEIGHT);
}