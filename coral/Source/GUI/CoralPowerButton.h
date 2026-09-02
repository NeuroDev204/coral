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

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class CoralPowerButton : public DrawableButton
{
public:
	CoralPowerButton(const String &button_name);
	~CoralPowerButton() = default;

	bool getPowerState();
	void setPowerState(bool power_state);

	int getImageWidth();
	void setImageWidth(int image_width);

private:
	void paint(Graphics& g) override;
	bool keyPressed(const KeyPress& key) override;
	void lookAndFeelChanged() override;

	std::unique_ptr<Drawable> power_on_image_;
	std::unique_ptr<Drawable> power_off_image_;

	bool power_state_;
	int image_width_;
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoralPowerButton)
};
