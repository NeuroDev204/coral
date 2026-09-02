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
#include "CoralBalanceSlider.h"

//==============================================================================
CoralBalanceSlider::CoralBalanceSlider(float default_value) : default_value_(default_value)
{
	setScrollWheelEnabled(false);
	setMouseCursor(MouseCursor::PointingHandCursor);

	auto& controller = CoralController::getInstance();
	auto& theme = dynamic_cast<CoralTheme&>(getLookAndFeel());

	setSliderStyle(Slider::LinearHorizontal);
	setRange(-20, 20, 2);
	setValue(controller.getBalance());
	setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
	setWantsKeyboardFocus(true);

	value_label_.setFont(theme.getNormalFont().withHeight(12.0f));
	value_label_.setJustificationType(Justification::centredLeft);

	addAndMakeVisible(value_label_);
}

void CoralBalanceSlider::valueChanged()
{
	auto value = getValue();
	auto& controller = CoralController::getInstance();

	if (controller.getBalance() != value)
		controller.setBalance((float)value);

	updateValueLabel();
}

void CoralBalanceSlider::resized()
{
	Slider::resized();

	updateValueLabel();
}

void CoralBalanceSlider::paint(Graphics& g)
{
	auto& theme = dynamic_cast<CoralTheme&>(getLookAndFeel());
	auto layout = theme.getSliderLayout(*this);

	auto bounds = layout.sliderBounds;
	auto x = bounds.getX();
	auto y = bounds.getY();
	auto width = bounds.getWidth();
	auto height = bounds.getHeight();

	auto value = getValue();
	float scaled_value = (value - (-20.0f)) / (20.0f - (-20.0f));

	Colour left_colour = Colour(FXCOLOR(SliderTrack)).withAlpha(1.0f - scaled_value);
	Colour right_colour = Colour(FXCOLOR(SliderTrack)).withAlpha(scaled_value);
	if (!isEnabled())
	{
		left_colour = left_colour.withSaturation(0.0);
		right_colour = right_colour.withSaturation(0.0);
	}

	const float track_h = 8.0f;
	const float track_y = y + (height - track_h) / 2.0f;
	ColourGradient gradient = ColourGradient::horizontal(left_colour, (float) x, right_colour, (float) (x + width));
	g.setGradientFill(gradient);
	g.fillRoundedRectangle((float) x, track_y, (float) width, track_h, track_h / 2.0f);

	if (hasKeyboardFocus(true))
	{
		Colour colour = Colour(FXCOLOR(SliderHighlight)).withAlpha(0.1f);
		g.setFillType(colour);
		g.fillRoundedRectangle(juce::Rectangle<float>(x, y, width, height).expanded(CoralTheme::SLIDER_THUMB_RADIUS / 2, CoralTheme::SLIDER_THUMB_RADIUS / 2), height + CoralTheme::SLIDER_THUMB_RADIUS);
	}
}

bool CoralBalanceSlider::keyPressed(const KeyPress& key)
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

void CoralBalanceSlider::mouseDown(const juce::MouseEvent& event)
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

void CoralBalanceSlider::updateValueLabel()
{
	auto value = getValue();

	// Update value label text
	auto text = String::formatted("%0.0f dB", std::fabs(value));
	value_label_.setText(text, NotificationType::dontSendNotification);

	// Position label based on slider thumb position
	auto pos = getPositionOfValue(value);
	auto x = pos + CoralTheme::SLIDER_THUMB_RADIUS / 2 + 1;
	value_label_.setBounds(x, (getHeight() - LABEL_HEIGHT) / 2, LABEL_WIDTH, LABEL_HEIGHT);
}
