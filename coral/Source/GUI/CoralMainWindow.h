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
#include "CoralWindow.h"
#include "CoralLiteView.h"
#include "CoralProView.h"
#include "CoralPowerButton.h"

//==============================================================================
/*
	This class implements the desktop window that contains an instance of
	our MainComponent class.
*/
class CoralMainWindow : public CoralWindow, private Button::Listener, public CoralModel::Listener
{
public:
    CoralMainWindow();
    ~CoralMainWindow();

    void show();
    void showLiteView();
    void showProView();
    void update();
    void startVisualizer();
    void pauseVisualizer();

	void setLookAndFeel();
    void setResizeImage();
    void setIcon(bool power, bool processing);
	void applyDockIcon();
    void enablePowerButton(bool enable);

    bool keyPressed(const KeyPress& key) override;
    void visibilityChanged() override;

private:
    static constexpr int BUTTON_WIDTH = 24;

    void showMenu();
    void buttonClicked(Button* button) override;
    void mouseEnter(const MouseEvent&) override;
    void modelChanged(CoralModel::Event model_event) override;
	void userTriedToCloseWindow() override;
	void closeButtonPressed() override;
    void moved() override;
	void lookAndFeelChanged() override;

	class ViewSwitcher : public Component, private Button::Listener
	{
	public:
		ViewSwitcher();
		void syncFromController();

	private:
		void resized() override;
		void paint(Graphics& g) override;
		void buttonClicked(Button* button) override;

		TextButton simple_{ "Simple" };
		TextButton pro_{ "Pro" };
	};

	CoralLiteView lite_view_;
	CoralProView  pro_view_;
    CoralPowerButton power_button_;
    CoralWindow::GlassGlyphButton menu_button_{ "menuButton", CoralWindow::GlassGlyphButton::Glyph::Menu };
    DrawableButton resize_button_;
    DrawableButton minimize_button_;
    DrawableButton donate_button_;
	ViewSwitcher view_switcher_;
    BubbleMessageComponent help_bubble_;

    std::unique_ptr<Drawable> menu_image_;
    std::unique_ptr<Drawable> menu_hover_image_;
    std::unique_ptr<Drawable> resize_image_;
    std::unique_ptr<Drawable> resize_hover_image_;
    std::unique_ptr<Drawable> minimize_image_;
    std::unique_ptr<Drawable> minimize_hover_image_;
	std::unique_ptr<Drawable> donate_image_;
	std::unique_ptr<Drawable> donate_hover_image_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoralMainWindow)
};
