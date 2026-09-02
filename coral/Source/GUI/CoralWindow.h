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
#include "CoralTheme.h"

//==============================================================================
/*
*/
class CoralWindow : public Component
{
public:
	CoralWindow(String name = "", bool desktop_controls = false);
	~CoralWindow();

	void setContent(Component* content);
	void setHeaderCenter(Component* component);
	virtual void closeButtonPressed() = 0;
	virtual void minimiseButtonPressed();
	virtual void maximiseButtonPressed();

	void startLogoAnimation();
	void stopLogoAnimation();
	void addToolbarButton(Button* toolbarButton, bool right_aligned = true);

    void enableShadow(bool enable);
    bool isShadowEnabled();
	bool isMaximised() const { return maximised_; }

protected:
    static constexpr int SHADOW_WIDTH = 0;
	static constexpr int CLOSE_BUTTON_WIDTH = 24;

	void paint(Graphics&) override;
	void resized() override;

	class WindowButton : public Button
	{
	public:
		enum class Kind { Close, Min, Max };
		explicit WindowButton(Kind kind) : Button("fxWindowButton"), kind_(kind) {}

	private:
		void paintButton(Graphics& g, bool over, bool down) override;
		Kind kind_;
	};

	class GlassGlyphButton : public Button
	{
	public:
		enum class Glyph { Menu };
		GlassGlyphButton(const String& name, Glyph glyph) : Button(name), glyph_(glyph) {}

	private:
		void paintButton(Graphics& g, bool over, bool down) override;
		Glyph glyph_;
	};

	class TitleBar : public Component, private Button::Listener
	{
	public:
		TitleBar(String name, bool desktop_controls);
		~TitleBar() = default;

		void startLogoAnimation();
		void stopLogoAnimation();
		void addToolbarButton(Button* toolbarButton, bool right_aligned=true);
		void setCenterComponent(Component* component);

	private:
		static constexpr int ICON_WIDTH = 106;
		static constexpr int ICON_HEIGHT = 15;

		void paint(Graphics& g) override;
		void resized() override;
		void lookAndFeelChanged() override;

		void buttonClicked(Button* button) override;

		void mouseDown(const MouseEvent& e);
		void mouseDrag(const MouseEvent& e);
		void mouseUp(const MouseEvent&);

		void updateLogo();

		String name_;
		bool desktop_controls_;
		std::unique_ptr<Drawable> icon_;
		std::unique_ptr<Drawable> animation_icon_;
		Label title_;
		std::vector<std::pair<Button*, bool>> toolbar_buttons_;
		WindowButton close_button_{ WindowButton::Kind::Close };
		WindowButton min_button_{ WindowButton::Kind::Min };
		WindowButton max_button_{ WindowButton::Kind::Max };
		Component* center_ = nullptr;
		ComponentDragger dragger_;
		bool dragging_;
	};

	Component* content_;
	TitleBar title_bar_;
    
    bool draw_shadow_;
    int shadow_width_;
	bool maximised_ = false;
	Rectangle<int> restored_bounds_;
	ComponentBoundsConstrainer constrainer_;
	std::unique_ptr<ResizableCornerComponent> resizer_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CoralWindow)
};
