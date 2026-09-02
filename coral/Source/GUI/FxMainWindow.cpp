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
#include "FxMainWindow.h"
#include "FxController.h"
#include "FxSettingsDialog.h"
#include "FxPresetExportDialog.h"
#include "FxPresetImportDialog.h"
#include "FxPresetNameEditor.h"

class FxPresetMenuItem : public PopupMenu::CustomComponent, private TextEditor::Listener
{
public:
	enum class Status { Empty = 0, Valid, Invalid };
	enum class Action { Save = 1, Rename };

	FxPresetMenuItem(Action action) : PopupMenu::CustomComponent(false)
	{
		auto& theme = dynamic_cast<FxTheme&>(getLookAndFeel());

		preset_status_ = Status::Empty;

		hint_text_.setName(L"hintLabel");
		hint_text_.setFont(theme.getNormalFont());
		hint_text_.setColour(Label::ColourIds::textColourId, Colour(FXCOLOR(HintText)).withAlpha(1.0f));
		hint_text_.setJustificationType(Justification::centredLeft);
		addAndMakeVisible(hint_text_);

		if (action == Action::Save)
		{
			hint_text_.setText(TRANS("Enter your preset name"), NotificationType::dontSendNotification);
			preset_editor_.setDescription(TRANS("Enter your preset name"));
		}
		else
		{
			hint_text_.setText(TRANS("Enter new preset name"), NotificationType::dontSendNotification);
			preset_editor_.setDescription(TRANS("Enter new preset name"));
		}
		preset_editor_.setName(L"presetName");
		preset_editor_.setFont(theme.getNormalFont());
		preset_editor_.setColour(TextEditor::ColourIds::backgroundColourId, Colour(FXCOLOR(DefaultFill)).withAlpha(0.0f));
		preset_editor_.setInputRestrictions(64);
		preset_editor_.setInputFilter(&preset_name_input_filter_, false);
		preset_editor_.addListener(this);
		addAndMakeVisible(preset_editor_);

		preset_editor_.onEscapeKey = [this]() {
			preset_status_ = Status::Empty;
			triggerMenuItem();
		};

		preset_editor_.onReturnKey = [this, action]() {
			if (preset_status_ == Status::Valid)
			{
				if (action == Action::Save)
				{
					FxController::getInstance().savePreset(preset_name_);
					triggerMenuItem();
				}
				else
				{
					FxController::getInstance().renamePreset(preset_name_);
					triggerMenuItem();
				}
			}
		};
	}
	~FxPresetMenuItem() = default;

	Status getStatus() { return preset_status_; }
	String getPresetName() { return preset_name_; }

	void getIdealSize(int& ideal_width, int& ideal_height)
	{
		ideal_width = WIDTH;
		ideal_height = HEIGHT;
	}

private:
	static constexpr int WIDTH = 200;
	static constexpr int HEIGHT = 30;

	void resized() override
	{
		auto bounds = getLocalBounds().reduced(2, 2);

		hint_text_.setBounds(bounds);
		preset_editor_.setBounds(bounds);
	}

	void paint(Graphics& g) override
	{
		auto bounds = getLocalBounds();
		Colour outline_colour;

		switch (preset_status_)
		{
		case Status::Valid:
			outline_colour = Colour(FXCOLOR(ValidTextBorder)).withAlpha(1.0f);
			break;

		case Status::Empty:
		case Status::Invalid:
			outline_colour = Colour(FXCOLOR(InvalidTextBorder)).withAlpha(1.0f);
		}

		g.setColour(findColour(TextEditor::backgroundColourId));
		g.fillRect(bounds.toFloat());

		g.setColour(outline_colour);
		g.drawRect(bounds.toFloat().reduced(0.5f, 0.5f), 2.0f);

		preset_editor_.grabKeyboardFocus();
	}

	void textEditorTextChanged(TextEditor& textEditor) override
	{
		auto prevStatus = preset_status_;

		preset_name_ = textEditor.getText();
		if (preset_name_.isEmpty())
		{
			preset_status_ = Status::Empty;
			hint_text_.setAlpha(1.0);
		}
		else
		{
			hint_text_.setAlpha(0.0);

			preset_status_ = Status::Valid;
			auto& model = FxModel::getModel();
			for (auto i = 0; i < model.getPresetCount(); i++)
			{
				if (model.getPreset(i).name.equalsIgnoreCase(preset_name_))
				{
					preset_status_ = Status::Invalid;
					break;
				}
			}
		}

		if (prevStatus != preset_status_)
		{
			sendLookAndFeelChange();
		}
	}

	void lookAndFeelChanged() override
	{
		repaint();
	}

	TextEditor preset_editor_;
	PresetNameInputFilter preset_name_input_filter_;
	Label hint_text_;
	Status preset_status_;
	String preset_name_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FxPresetMenuItem)
};

//==============================================================================
FxMainWindow::FxMainWindow() : FxWindow("Coral", true), power_button_(L"powerButton"), resize_button_(L"resizeButton", DrawableButton::ButtonStyle::ImageFitted), donate_button_(L"donateButton", DrawableButton::ButtonStyle::ImageFitted), minimize_button_(L"minimizeButton", DrawableButton::ButtonStyle::ImageFitted)
{
	setName("Coral");
	setOpaque(false);
    enableShadow(false);

	setWantsKeyboardFocus(true);

	FxModel::getModel().addListener(this);

	power_button_.setPowerState(FxModel::getModel().getPowerState());

	power_button_.setMouseCursor(MouseCursor::PointingHandCursor);
	power_button_.setSize(32, 32);
	power_button_.setImageWidth(26);
	power_button_.setHelpText(TRANS("Power"));
	power_button_.setWantsKeyboardFocus(true);
	power_button_.addListener(this);

	menu_button_.setMouseCursor(MouseCursor::PointingHandCursor);
	menu_button_.setSize(32, 32);
	menu_button_.setHelpText(TRANS("Menu"));
	menu_button_.setWantsKeyboardFocus(true);
	menu_button_.addListener(this);

	resize_button_.setMouseCursor(MouseCursor::PointingHandCursor);	
	resize_button_.setHelpText(TRANS("Resize Button"));
	resize_button_.setWantsKeyboardFocus(true);
	resize_button_.addListener(this);	

	donate_button_.setMouseCursor(MouseCursor::PointingHandCursor);
	donate_button_.setSize(BUTTON_WIDTH + 2, BUTTON_WIDTH + 6);
	donate_button_.setHelpText(TRANS("Donate"));
	donate_button_.setTooltip(TRANS("Donate"));
	donate_button_.setWantsKeyboardFocus(true);
	donate_button_.onClick = [this]() {
		URL url("https://www.paypal.com/donate/?hosted_button_id=JVNQGYXCQ2GPG");
		url.launchInDefaultBrowser();
	};

	minimize_button_.setMouseCursor(MouseCursor::PointingHandCursor);
	minimize_button_.setSize(BUTTON_WIDTH + 2, BUTTON_WIDTH + 6);
	minimize_button_.setHelpText(TRANS("Minimize Button"));
	minimize_button_.setWantsKeyboardFocus(true);
	minimize_button_.addListener(this);

	help_bubble_.setAlwaysOnTop(true);

	setLookAndFeel();

	view_switcher_.setSize(184, 32);
	setHeaderCenter(&view_switcher_);
	addToolbarButton(&menu_button_, true);
	addToolbarButton(&power_button_, true);
}

FxMainWindow::~FxMainWindow()
{
	help_bubble_.removeFromDesktop();
	FxModel::getModel().removeListener(this);
}

void FxMainWindow::show()
{
	if (!isVisible())
	{
		setVisible(true);
	}

	addToDesktop(ComponentPeer::windowAppearsOnTaskbar
		| ComponentPeer::windowIsResizable
		| ComponentPeer::windowIsSemiTransparent);

	int x = 0;
	int y = 0;
	FxController::getInstance().getWindowPosition(x, y);
	if (x != 0 || y != 0)
		setTopLeftPosition(x, y);
	else
		centreWithSize(getWidth(), getHeight());

	toFront(true);
	applyDockIcon();

	// Bring window to the top
	auto* peer = getPeer();
	if (peer)
	{
#if defined(_WIN32)
		HWND hwnd = (HWND)peer->getNativeHandle();
		if (IsIconic(hwnd)) // If minimized, restore first
			ShowWindow(hwnd, SW_RESTORE);

		SetForegroundWindow(hwnd); // Bring to front
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
#endif
	}
}

void FxMainWindow::showLiteView()
{
	setContent(&lite_view_);
    setResizeImage();
	view_switcher_.syncFromController();
	setAlwaysOnTop(FxController::getInstance().isAlwaysOnTop());
	lite_view_.startVisualizer();
}

void FxMainWindow::showProView()
{
    pro_view_.update();
	setContent(&pro_view_);
    setResizeImage();
	view_switcher_.syncFromController();
	setAlwaysOnTop(FxController::getInstance().isAlwaysOnTop());
}

void FxMainWindow::update()
{
	pro_view_.update();
}

void FxMainWindow::startVisualizer()
{
    pro_view_.startVisualizer();
    lite_view_.startVisualizer();
}

void FxMainWindow::pauseVisualizer()
{
    pro_view_.pauseVisualizer();
    lite_view_.pauseVisualizer();
}

void FxMainWindow::setLookAndFeel()
{
	auto& theme = dynamic_cast<FxTheme&>(LookAndFeel::getDefaultLookAndFeel());

	donate_image_ = Drawable::createFromImageData(FXIMAGE(DonateButton), FXIMAGESIZE(DonateButton));
	donate_hover_image_ = Drawable::createFromImageData(FXIMAGE(DonateButtonHover), FXIMAGESIZE(DonateButtonHover));
	donate_button_.setImages(donate_image_.get(), donate_hover_image_.get());

	minimize_image_ = Drawable::createFromImageData(FXIMAGE(MinimizeWindowButton), FXIMAGESIZE(MinimizeWindowButton));
	minimize_hover_image_ = Drawable::createFromImageData(FXIMAGE(MinimizeWindowButtonHover), FXIMAGESIZE(MinimizeWindowButtonHover));
	minimize_button_.setImages(minimize_image_.get(), minimize_hover_image_.get());

	help_bubble_.setColour(BubbleComponent::ColourIds::backgroundColourId, Colour(FXCOLOR(DefaultFill)).withAlpha(1.0f));
	help_bubble_.setColour(BubbleComponent::ColourIds::outlineColourId, theme.findColour(TextEditor::textColourId));

	pro_view_.setLookAndFeel();

	setResizeImage();
}

void FxMainWindow::setResizeImage()
{
	if (FxController::getInstance().getCurrentView() == ViewType::Pro)
	{
		resize_image_ = Drawable::createFromImageData(FXIMAGE(MinimizeButton), FXIMAGESIZE(MinimizeButton));
		resize_hover_image_ = Drawable::createFromImageData(FXIMAGE(MinimizeButtonHover), FXIMAGESIZE(MinimizeButtonHover));
		resize_button_.setSize(BUTTON_WIDTH+2, BUTTON_WIDTH+2);
	}
	else
	{
		resize_image_ = Drawable::createFromImageData(FXIMAGE(MaximizeButton), FXIMAGESIZE(MaximizeButton));
		resize_hover_image_ = Drawable::createFromImageData(FXIMAGE(MaximizeButtonHover), FXIMAGESIZE(MaximizeButtonHover));
		resize_button_.setSize(BUTTON_WIDTH, BUTTON_WIDTH); 	
	}

	resize_button_.setImages(resize_image_.get(), resize_hover_image_.get());
}

void FxMainWindow::setIcon(bool power, bool processing)
{
#if defined(_WIN32)
	HINSTANCE hInst = GetModuleHandle(NULL);
	HWND hWnd = (HWND)getWindowHandle();

	HICON curr_icon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL, 0);
	HICON icon = NULL;

	if (power)
	{
		if (processing)
		{
			if (FxTheme::getThemeMode() == FxThemeMode::Dark)
				icon = LoadIcon(hInst, L"IDI_LOGO_RED");
			else
				icon = LoadIcon(hInst, L"IDI_LOGO_BLUE");
		}
		else
		{
			icon = LoadIcon(hInst, L"IDI_LOGO_WHITE");
		}
	}
	else
	{
		icon = LoadIcon(hInst, L"IDI_LOGO_GRAY");
	}

	if (icon != NULL)
	{
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
	}

	if (curr_icon != NULL)
	{
		DestroyIcon(curr_icon);
	}
#else
	ignoreUnused(power, processing);
	applyDockIcon();
#endif
}

void FxMainWindow::applyDockIcon()
{
	auto* peer = getPeer();
	if (peer == nullptr)
		return;

	File icon_file = File::getSpecialLocation(File::userHomeDirectory)
		.getChildFile(".local")
		.getChildFile("share")
		.getChildFile("icons")
		.getChildFile("hicolor")
		.getChildFile("256x256")
		.getChildFile("apps")
		.getChildFile("coral.png");

	if (!icon_file.existsAsFile())
		icon_file = File("/usr/share/icons/hicolor/256x256/apps/coral.png");
	if (!icon_file.existsAsFile())
		icon_file = File::getSpecialLocation(File::currentExecutableFile).getSiblingFile("coral.png");

	if (!icon_file.existsAsFile())
		return;

	const Image icon = ImageFileFormat::loadFrom(icon_file);
	if (icon.isValid())
		peer->setIcon(icon);
}

void FxMainWindow::enablePowerButton(bool enable)
{
	if (power_button_.isEnabled() != enable)
	{
		power_button_.setEnabled(enable);

		if (!enable)
		{
			power_button_.setTooltip(TRANS("Audio enhancements are not available over Remote Desktop"));
		}
		else
		{
			power_button_.setTooltip("");
		}
	}
}

bool FxMainWindow::keyPressed(const KeyPress& key)
{
	if (key.getModifiers().isAltDown() && key.getKeyCode() == juce::KeyPress::returnKey)
	{
		FxController::getInstance().setMenuClicked(true);
		showMenu();
		return true;
	}

	return Component::keyPressed(key);
}

void FxMainWindow::visibilityChanged()
{
	auto* peer = getPeer();
	if (peer)
	{
#if defined(_WIN32)
		HWND hwnd = (HWND)peer->getNativeHandle();
		LONG style = GetWindowLong(hwnd, GWL_STYLE);
		if ((style & WS_MINIMIZEBOX) == 0)
		{
			SetWindowLong(hwnd, GWL_STYLE, style | WS_MINIMIZEBOX);
		}
#endif
	}
}

void FxMainWindow::showMenu()
{
	auto settingsClicked = []() {
		FxSettingsDialog settings_dialog;
		settings_dialog.runModalLoop();
		FxController::getInstance().refreshOutputList();
	};

	PopupMenu popup_menu;
	popup_menu.addItem(TRANS("Settings"), settingsClicked);
	popup_menu.addSeparator();
	popup_menu.addItem(TRANS("Exit"), []() { FxController::getInstance().exit(); });
	popup_menu.showAt(&menu_button_);
}

void FxMainWindow::buttonClicked(Button* button)
{
	if (button == &power_button_)
	{
		auto power_state = !FxModel::getModel().getPowerState();

		FxController::getInstance().setPowerState(power_state);

		power_button_.setPowerState(FxModel::getModel().getPowerState());

		repaint();
	}
	else if (button == &menu_button_)
	{
		FxController::getInstance().setMenuClicked(true);
		showMenu();
	}
	else if (button == &resize_button_)
	{
		FxController::getInstance().switchView();
		setResizeImage();
	}
	else if (button == &minimize_button_)
	{
#if defined(_WIN32)
		if (isOnDesktop())
			ShowWindow((HWND)getWindowHandle(), SW_MINIMIZE);
#else
		FxController::getInstance().hideMainWindow();
#endif
	}
}

void FxMainWindow::mouseEnter(const MouseEvent&)
{
	if (menu_button_.isMouseOver(true) && !FxModel::getModel().isMenuClicked())
	{
		auto& theme = dynamic_cast<FxTheme&>(getLookAndFeel());

		AttributedString text(TRANS("Click here to save new presets, overwrite old ones, or reset your settings."));
		text.setColour(theme.findColour(TextEditor::textColourId));
		text.setJustification(Justification::centred);
		text.setFont(theme.getSmallFont());

		help_bubble_.showAt(&menu_button_, text, 0, true, false);
	}
}

void FxMainWindow::modelChanged(FxModel::Event)
{
	power_button_.setPowerState(FxModel::getModel().getPowerState());
}

void FxMainWindow::userTriedToCloseWindow()
{
	FxController::getInstance().hideMainWindow();
}

void FxMainWindow::closeButtonPressed()
{
	FxController::getInstance().hideMainWindow();
}

void FxMainWindow::moved()
{
	auto bounds = getBounds();
	auto desktop_bounds = Desktop::getInstance().getDisplays().getTotalBounds(true);
	if (desktop_bounds.contains(bounds))
		FxController::getInstance().saveWindowPosition(bounds.getX(), bounds.getY());
}

void FxMainWindow::lookAndFeelChanged()
{
	setLookAndFeel();
	view_switcher_.syncFromController();
	repaint();
}

FxMainWindow::ViewSwitcher::ViewSwitcher()
{
	simple_.setClickingTogglesState(true);
	pro_.setClickingTogglesState(true);
	simple_.setRadioGroupId(1001);
	pro_.setRadioGroupId(1001);
	simple_.setMouseCursor(MouseCursor::PointingHandCursor);
	pro_.setMouseCursor(MouseCursor::PointingHandCursor);
	simple_.setWantsKeyboardFocus(true);
	pro_.setWantsKeyboardFocus(true);
	const auto clear = Colours::transparentBlack;
	for (auto* b : { &simple_, &pro_ })
	{
		b->setColour(TextButton::buttonColourId, clear);
		b->setColour(TextButton::buttonOnColourId, clear);
		b->setColour(TextButton::textColourOffId, Colour(FXCOLOR(HintText)).withAlpha(1.0f));
		b->setColour(TextButton::textColourOnId, Colours::white);
	}
	simple_.addListener(this);
	pro_.addListener(this);
	addAndMakeVisible(simple_);
	addAndMakeVisible(pro_);
	syncFromController();
}

void FxMainWindow::ViewSwitcher::syncFromController()
{
	const bool pro = FxController::getInstance().getCurrentView() == ViewType::Pro;
	simple_.setToggleState(!pro, dontSendNotification);
	pro_.setToggleState(pro, dontSendNotification);
}

void FxMainWindow::ViewSwitcher::resized()
{
	auto r = getLocalBounds();
	simple_.setBounds(r.removeFromLeft(r.getWidth() / 2));
	pro_.setBounds(r);
}

void FxMainWindow::ViewSwitcher::paint(Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();
	g.setColour(Colours::white.withAlpha(0.10f));
	g.fillRoundedRectangle(bounds, 16.0f);
	g.setColour(Colours::white.withAlpha(0.22f));
	g.drawRoundedRectangle(bounds.reduced(0.5f), 16.0f, 1.0f);

	auto pill = bounds.reduced(3.0f);
	if (pro_.getToggleState())
		pill = pill.removeFromRight(pill.getWidth() * 0.5f);
	else
		pill = pill.removeFromLeft(pill.getWidth() * 0.5f);

	g.setColour(Colour(FXCOLOR(HighlightedFill)).withAlpha(1.0f));
	g.fillRoundedRectangle(pill, 13.0f);
}

void FxMainWindow::ViewSwitcher::buttonClicked(Button* button)
{
	auto& controller = FxController::getInstance();
	const bool want_pro = (button == &pro_);
	const bool is_pro = controller.getCurrentView() == ViewType::Pro;
	if (want_pro != is_pro)
		controller.switchView();
	syncFromController();
}