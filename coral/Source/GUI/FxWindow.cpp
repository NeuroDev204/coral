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

#include <JuceHeader.h>
#include "FxWindow.h"

//==============================================================================
FxWindow::FxWindow(String name, bool desktop_controls) : title_bar_(name, desktop_controls), content_(nullptr)
{
    setLookAndFeel(&LookAndFeel::getDefaultLookAndFeel());
	setOpaque(false);

	if (desktop_controls)
		setSize(720, 560);
	else
		setSize(64, 64);

	title_bar_.setSize(64, FxTheme::TITLE_BAR_HEIGHT);
	addAndMakeVisible(title_bar_);

    draw_shadow_ = false;
    shadow_width_ = 0;

	constrainer_.setMinimumSize(560, 400);
	if (desktop_controls)
	{
		resizer_ = std::make_unique<ResizableCornerComponent>(this, &constrainer_);
		addAndMakeVisible(*resizer_);
	}
}

FxWindow::~FxWindow()
{
	setLookAndFeel(nullptr);
}

void FxWindow::setContent(Component* content)
{
	if (content != content_)
	{
		removeChildComponent(content_);
		content_ = content;
		addAndMakeVisible(content);
		if (resizer_ != nullptr)
			resizer_->toFront(false);
	}

	if (resizer_ == nullptr && content_ != nullptr)
	{
		setSize(content_->getWidth(), content_->getHeight() + title_bar_.getHeight());
		content_->setBounds(0, title_bar_.getBottom(), content_->getWidth(), content_->getHeight());
	}
	else
	{
		resized();
	}
}

void FxWindow::setHeaderCenter(Component* component)
{
	title_bar_.setCenterComponent(component);
}

void FxWindow::minimiseButtonPressed()
{
	closeButtonPressed();
}

void FxWindow::maximiseButtonPressed()
{
	if (maximised_)
	{
		setBounds(restored_bounds_);
		maximised_ = false;
		return;
	}

	restored_bounds_ = getBounds();
	if (auto* display = Desktop::getInstance().getDisplays().getDisplayForRect(restored_bounds_))
		setBounds(display->userArea);
	else if (auto* primary = Desktop::getInstance().getDisplays().getPrimaryDisplay())
		setBounds(primary->userArea);
	maximised_ = true;
}

void FxWindow::startLogoAnimation()
{
	title_bar_.startLogoAnimation();
}

void FxWindow::stopLogoAnimation()
{
	title_bar_.stopLogoAnimation();
}

void FxWindow::addToolbarButton(Button* toolbarButton, bool right_aligned)
{
	title_bar_.addToolbarButton(toolbarButton, right_aligned);
}

void FxWindow::enableShadow(bool enable)
{
    draw_shadow_ = enable;
    if (enable)
    {
        shadow_width_ = SHADOW_WIDTH;
    }
    else
    {
        shadow_width_ = 0;
    }
}

bool FxWindow::isShadowEnabled()
{
    return draw_shadow_;
}

void FxWindow::paint(Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();
	const float radius = (float)FxTheme::WINDOW_CORNER_RADIUS;

	g.setColour(Colour(FXCOLOR(WindowBackground)).withAlpha(FxTheme::WINDOW_FILL_ALPHA));
	g.fillRoundedRectangle(bounds, radius);

	ColourGradient sheen(Colours::white.withAlpha(0.12f), bounds.getX(), bounds.getY(),
		Colours::white.withAlpha(0.02f), bounds.getX(), bounds.getBottom(), false);
	g.setGradientFill(sheen);
	g.fillRoundedRectangle(bounds, radius);

	g.setColour(Colours::white.withAlpha(0.22f));
	g.drawRoundedRectangle(bounds.reduced(0.6f), radius, 1.2f);

	g.setColour(Colour(FXCOLOR(HighlightedFill)).withAlpha(0.40f));
	g.fillRect(8.0f, (float) title_bar_.getBottom() - 1.0f, (float) getWidth() - 16.0f, 1.5f);
}

void FxWindow::resized()
{
	title_bar_.setBounds(0, 0, getWidth(), FxTheme::TITLE_BAR_HEIGHT);
	if (content_ != nullptr)
		content_->setBounds(0, title_bar_.getBottom(), getWidth(), getHeight() - title_bar_.getHeight());

	if (resizer_ != nullptr)
		resizer_->setBounds(getWidth() - 16, getHeight() - 16, 16, 16);
}

void FxWindow::WindowButton::paintButton(Graphics& g, bool over, bool down)
{
	auto r = getLocalBounds().toFloat().reduced(2.0f);
	const bool close_hot = (kind_ == Kind::Close && over);
	g.setColour(close_hot ? Colour(FXCOLOR(InvalidTextBorder)).withAlpha(0.88f)
		: Colours::white.withAlpha(down ? 0.18f : over ? 0.14f : 0.08f));
	g.fillEllipse(r);
	g.setColour(Colours::white.withAlpha(close_hot ? 0.95f : 0.30f));
	g.drawEllipse(r.reduced(0.4f), 1.05f);

	g.setColour(close_hot ? Colours::white : Colour(FXCOLOR(DefaultText)).withAlpha(0.92f));
	auto c = r.getCentre();
	if (kind_ == Kind::Close)
	{
		g.drawLine(c.x - 3.6f, c.y - 3.6f, c.x + 3.6f, c.y + 3.6f, 1.5f);
		g.drawLine(c.x + 3.6f, c.y - 3.6f, c.x - 3.6f, c.y + 3.6f, 1.5f);
	}
	else if (kind_ == Kind::Min)
	{
		g.drawLine(c.x - 4.2f, c.y, c.x + 4.2f, c.y, 1.5f);
	}
	else
	{
		g.drawRoundedRectangle(c.x - 4.0f, c.y - 4.0f, 8.0f, 8.0f, 1.4f, 1.4f);
	}
}

void FxWindow::GlassGlyphButton::paintButton(Graphics& g, bool over, bool down)
{
	auto r = getLocalBounds().toFloat().reduced(1.5f);
	g.setColour(Colours::white.withAlpha(down ? 0.18f : over ? 0.14f : 0.08f));
	g.fillEllipse(r);
	g.setColour(Colours::white.withAlpha(0.28f));
	g.drawEllipse(r.reduced(0.4f), 1.05f);

	if (glyph_ == Glyph::Menu)
	{
		auto c = r.getCentre();
		g.setColour(Colour(FXCOLOR(DefaultText)).withAlpha(0.95f));
		const float w = 11.0f;
		g.drawLine(c.x - w / 2, c.y - 5.0f, c.x + w / 2, c.y - 5.0f, 1.6f);
		g.drawLine(c.x - w / 2, c.y, c.x + w / 2, c.y, 1.6f);
		g.drawLine(c.x - w / 2, c.y + 5.0f, c.x + w / 2, c.y + 5.0f, 1.6f);
	}
}

FxWindow::TitleBar::TitleBar(String name, bool desktop_controls)
{
	name_ = name;
	desktop_controls_ = desktop_controls;
	dragging_ = false;

	addAndMakeVisible(title_);

	updateLogo();
	if (animation_icon_.get() != nullptr)
		animation_icon_->setAlpha(0.0f);

	setFocusContainer(true);
	setFocusContainerType(FocusContainerType::keyboardFocusContainer);

	close_button_.setMouseCursor(MouseCursor::PointingHandCursor);
	close_button_.setSize(CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_WIDTH);
	close_button_.setHelpText(TRANS("Close"));
	addAndMakeVisible(close_button_);
	close_button_.addListener(this);

	if (desktop_controls_)
	{
		min_button_.setMouseCursor(MouseCursor::PointingHandCursor);
		min_button_.setSize(CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_WIDTH);
		min_button_.setHelpText(TRANS("Minimize"));
		addAndMakeVisible(min_button_);
		min_button_.addListener(this);

		max_button_.setMouseCursor(MouseCursor::PointingHandCursor);
		max_button_.setSize(CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_WIDTH);
		max_button_.setHelpText(TRANS("Maximize"));
		addAndMakeVisible(max_button_);
		max_button_.addListener(this);
	}
}

void FxWindow::TitleBar::startLogoAnimation()
{
	if (animation_icon_.get() != nullptr)
	{
		Desktop::getInstance().getAnimator().fadeOut(icon_.get(), 600);
		Desktop::getInstance().getAnimator().fadeIn(animation_icon_.get(), 600);
	}
}
void FxWindow::TitleBar::stopLogoAnimation()
{
	if (animation_icon_.get() != nullptr)
	{
		Desktop::getInstance().getAnimator().fadeOut(animation_icon_.get(), 600);
		Desktop::getInstance().getAnimator().fadeIn(icon_.get(), 600);
	}
}

void FxWindow::TitleBar::addToolbarButton(Button* toolbarButton, bool right_aligned)
{
	if (toolbarButton != nullptr)
	{
		// If translated text is added to the text button in the toolbar, it doesn't change on language change
		// So, the text button is added to the toolbar with button name in English and then button text is translated from button name
		// On repaint, the translated button text is updated
		if (TextButton* text_button = dynamic_cast<TextButton*>(toolbarButton))
		{
			const auto& text = text_button->getName();
			if (text.isNotEmpty())
			{
				text_button->setButtonText(TRANS(text));
			}
		}

		if (!getWantsKeyboardFocus())
		{
			setWantsKeyboardFocus(true);
		}

		toolbar_buttons_.push_back({ toolbarButton, right_aligned });
		addAndMakeVisible(toolbarButton);
	}
}

void FxWindow::TitleBar::setCenterComponent(Component* component)
{
	center_ = component;
	if (center_ != nullptr)
		addAndMakeVisible(center_);
	resized();
}

void FxWindow::TitleBar::paint(Graphics& g)
{
	g.setColour(Colours::white.withAlpha(0.06f));
	g.fillRect(getLocalBounds());

	auto& theme = dynamic_cast<FxTheme&>(getLookAndFeel());
	const auto caption = name_.isNotEmpty() ? TRANS(name_) : String("Coral");
	title_.setText(caption, NotificationType::dontSendNotification);
	title_.setFont(theme.getTitleFont());
	title_.setColour(Label::ColourIds::textColourId, Colour(FXCOLOR(DefaultText)).withAlpha(1.0f));
	FxTheme::paintCoralMark(g, { 12.0f, (float) (getHeight() - 16) * 0.5f, 20.0f, 16.0f },
		Colour(FXCOLOR(HighlightedFill)).withAlpha(1.0f));

	for (auto& item : toolbar_buttons_)
	{
		if (auto* text_button = dynamic_cast<TextButton*>(item.first))
		{
			const auto& text = text_button->getName();
			if (text.isNotEmpty())
				text_button->setButtonText(TRANS(text));
		}
	}
}

void FxWindow::TitleBar::resized()
{
	auto bounds = getLocalBounds().reduced(10, 0);
	const int btn = CLOSE_BUTTON_WIDTH;
	const int gap = 6;
	const int y = (bounds.getHeight() - btn) / 2;

	close_button_.setBounds(bounds.getRight() - btn, y, btn, btn);
	int right = bounds.getRight() - btn - gap;

	if (desktop_controls_)
	{
		max_button_.setBounds(right - btn, y, btn, btn);
		right -= btn + gap;
		min_button_.setBounds(right - btn, y, btn, btn);
		right -= btn + gap;
	}

	int left = bounds.getX();
	if (icon_ != nullptr)
		icon_->setVisible(false);
	if (animation_icon_ != nullptr)
		animation_icon_->setVisible(false);

	title_.setBounds(left + 26, 0, 96, bounds.getHeight());
	left += 126;

	for (auto& item : toolbar_buttons_)
	{
		auto* button = item.first;
		if (item.second)
		{
			right -= button->getWidth();
			button->setBounds(right, (bounds.getHeight() - button->getHeight()) / 2, button->getWidth(), button->getHeight());
			right -= gap;
		}
		else
		{
			button->setBounds(left, (bounds.getHeight() - button->getHeight()) / 2, button->getWidth(), button->getHeight());
			left += button->getWidth() + gap;
		}
	}

	if (center_ != nullptr)
	{
		auto w = center_->getWidth();
		auto h = center_->getHeight();
		center_->setBounds((getWidth() - w) / 2, (getHeight() - h) / 2, w, h);
	}
}

void FxWindow::TitleBar::lookAndFeelChanged()
{
	updateLogo();
	repaint();
}

void FxWindow::TitleBar::buttonClicked(Button* button)
{
	auto parent = dynamic_cast<FxWindow*>(getParentComponent());
	if (parent == nullptr)
		return;

	if (button == &close_button_)
		parent->closeButtonPressed();
	else if (button == &min_button_)
		parent->minimiseButtonPressed();
	else if (button == &max_button_)
		parent->maximiseButtonPressed();
}

void FxWindow::TitleBar::mouseDown(const MouseEvent& e)
{
	dragging_ = true;
	auto parent = dynamic_cast<FxWindow*>(getParentComponent());
	dragger_.startDraggingComponent(parent, e);
}

void FxWindow::TitleBar::mouseDrag(const MouseEvent& e)
{
	if (dragging_)
	{
		auto parent = dynamic_cast<FxWindow*>(getParentComponent());
		dragger_.dragComponent(parent, e, nullptr);
	}
}

void FxWindow::TitleBar::mouseUp(const MouseEvent&)
{
	dragging_ = false;
}

void FxWindow::TitleBar::updateLogo()
{
	icon_.reset();
	animation_icon_.reset();

	auto& theme = dynamic_cast<FxTheme&>(LookAndFeel::getDefaultLookAndFeel());
	auto font = theme.getTitleFont();
	title_.setText(name_.isNotEmpty() ? name_ : "Coral", NotificationType::dontSendNotification);
	title_.setColour(Label::ColourIds::textColourId, Colour(FXCOLOR(DefaultText)).withAlpha(1.0f));
	title_.setFont(font);
	title_.setJustificationType(Justification::centredLeft);
	title_.setSize(jmax(96, font.getStringWidth(title_.getText()) + 8), (int) font.getHeight());
	title_.setVisible(true);
	addAndMakeVisible(title_);
}