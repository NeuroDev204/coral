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
#include "BinaryData.h"

#include "FxTheme.h"

namespace
{
    Typeface::Ptr linuxUiTypeface(int style, const void* fallback, int fallback_size)
    {
        auto names = Font::findAllTypefaceNames();
        String family;
        for (auto* candidate : { "Inter", "Ubuntu", "Cantarell", "Noto Sans" })
        {
            if (names.contains(String(candidate), true))
            {
                family = candidate;
                break;
            }
        }

        if (family.isNotEmpty())
            return Font(family, 14.0f, style).getTypefacePtr();

        return Typeface::createSystemTypefaceFor(fallback, fallback_size);
    }

    constexpr char kCoralMarkDark[] =
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'>"
        "<g fill='#7eb8ea'>"
        "<rect x='8' y='24' width='9' height='22' rx='4.5'/>"
        "<rect x='21' y='8' width='9' height='48' rx='4.5'/>"
        "<rect x='34' y='16' width='9' height='32' rx='4.5'/>"
        "<rect x='47' y='28' width='9' height='16' rx='4.5'/>"
        "</g></svg>";

    constexpr char kCoralMarkLight[] =
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'>"
        "<g fill='#5aa6d6'>"
        "<rect x='8' y='24' width='9' height='22' rx='4.5'/>"
        "<rect x='21' y='8' width='9' height='48' rx='4.5'/>"
        "<rect x='34' y='16' width='9' height='32' rx='4.5'/>"
        "<rect x='47' y='28' width='9' height='16' rx='4.5'/>"
        "</g></svg>";
}

const uint32 FxTheme::theme_colors_[FxThemeMode::NumModes][FxColor::NumColors] =
{ { 0x2b2a2e, 0x35343a, 0x3c3b42, 0x4a4950, 0xeceaf0, 0x3c3b42, 0xffffff, 0x7eb8ea, 0xeceaf0,
	0x403f46, 0x7eb8ea, 0xd8d6de, 0xa8a6b0, 0x7eb8ea, 0xd0727a, 0x35343a, 0x7eb8ea, 0xb5d9f5,
	0x7eb8ea, 0xb5d9f5, 0x7eb8ea, 0xc5e4f8, 0xeceaf0, 0x4a4952, 0x35343a, 0xa8a6b0, 0x7eb8ea },

  { 0xf6f3ef, 0xfffcf8, 0xfffcf8, 0xe4ddd4, 0x3a3732, 0xfffcf8, 0x3a3732, 0x5aa6d6, 0x3a3732,
	0xfffcf8, 0x5aa6d6, 0x6b675f, 0x7a756c, 0x5aa6d6, 0xc45b63, 0xfffcf8, 0x5aa6d6, 0x8ec8ee,
	0x5aa6d6, 0x8ec8ee, 0x5aa6d6, 0xc5e4f8, 0x3a3732, 0xe8e2da, 0xfffcf8, 0x7a756c, 0x5aa6d6} };

const char* FxTheme::theme_images_[FxThemeMode::NumModes][FxImage::NumImages] =
{ { kCoralMarkDark, kCoralMarkDark, kCoralMarkDark,
	BinaryData::power_on_svg, BinaryData::power_off_svg, BinaryData::donate_svg, BinaryData::donate_hover_svg, BinaryData::menu_svg, BinaryData::menu_hover_svg,
	BinaryData::minimize_svg, BinaryData::minimize_hover_svg, BinaryData::maximize_svg, BinaryData::maximize_hover_svg, BinaryData::min_window_svg, BinaryData::min_window_hover_svg,
    BinaryData::flip_white_svg, BinaryData::flip_svg, BinaryData::restore_defaults_white_svg, BinaryData::restore_defaults_svg,
	BinaryData::arrow_next_svg, BinaryData::arrow_next_bw_svg, BinaryData::arrow_prev_svg, BinaryData::arrow_prev_bw_svg, BinaryData::arrow_up_svg, BinaryData::arrow_up_white_svg, BinaryData::arrow_down_svg,  BinaryData::arrow_down_white_svg, BinaryData::dropdown_arrow_bw_svg, BinaryData::dropdown_arrow_hover_svg,
	BinaryData::Slider_Thumb_svg, BinaryData::Slider_Thumb_bw_svg },

  { kCoralMarkLight, kCoralMarkLight, kCoralMarkLight,
	BinaryData::power_on_blue_svg, BinaryData::power_off_black_svg, BinaryData::donate_blue_svg, BinaryData::donate_hover_blue_svg, BinaryData::menu_black_svg, BinaryData::menu_hover_blue_svg,
	BinaryData::minimize_black_svg, BinaryData::minimize_hover_blue_svg, BinaryData::maximize_black_svg, BinaryData::maximize_hover_blue_svg, BinaryData::min_window_black_svg, BinaryData::min_window_hover_blue_svg,
    BinaryData::flip_black_svg, BinaryData::flip_blue_svg, BinaryData::restore_defaults_black_svg, BinaryData::restore_defaults_blue_svg,
	BinaryData::arrow_next_blue_svg, BinaryData::arrow_next_bw_svg, BinaryData::arrow_prev_blue_svg, BinaryData::arrow_prev_bw_svg, BinaryData::arrow_up_blue_svg, BinaryData::arrow_up_black_svg, BinaryData::arrow_down_blue_svg, BinaryData::arrow_down_black_svg, BinaryData::dropdown_arrow_bw_svg, BinaryData::dropdown_arrow_hover_blue_svg,
	BinaryData::Slider_Thumb_blue_svg, BinaryData::Slider_Thumb_bw_svg } };

const int FxTheme::theme_image_sizes_[FxThemeMode::NumModes][FxImage::NumImages] =
{ { (int) sizeof(kCoralMarkDark) - 1, (int) sizeof(kCoralMarkDark) - 1, (int) sizeof(kCoralMarkDark) - 1,
   BinaryData::power_on_svgSize, BinaryData::power_off_svgSize, BinaryData::donate_svgSize, BinaryData::donate_hover_svgSize, BinaryData::menu_svgSize, BinaryData::menu_hover_svgSize,
   BinaryData::minimize_svgSize, BinaryData::minimize_hover_svgSize, BinaryData::maximize_svgSize, BinaryData::maximize_hover_svgSize, BinaryData::min_window_svgSize, BinaryData::min_window_hover_svgSize,
   BinaryData::flip_white_svgSize, BinaryData::flip_svgSize, BinaryData::restore_defaults_white_svgSize, BinaryData::restore_defaults_svgSize,
   BinaryData::arrow_next_svgSize, BinaryData::arrow_next_bw_svgSize, BinaryData::arrow_prev_svgSize, BinaryData::arrow_prev_bw_svgSize, BinaryData::arrow_up_svgSize, BinaryData::arrow_up_white_svgSize, BinaryData::arrow_down_svgSize,  BinaryData::arrow_down_white_svgSize, BinaryData::dropdown_arrow_bw_svgSize, BinaryData::dropdown_arrow_hover_svgSize,
   BinaryData::Slider_Thumb_svgSize, BinaryData::Slider_Thumb_bw_svgSize },

  { (int) sizeof(kCoralMarkLight) - 1, (int) sizeof(kCoralMarkLight) - 1, (int) sizeof(kCoralMarkLight) - 1,
	BinaryData::power_on_blue_svgSize, BinaryData::power_off_black_svgSize, BinaryData::donate_blue_svgSize, BinaryData::donate_hover_blue_svgSize, BinaryData::menu_black_svgSize, BinaryData::menu_hover_blue_svgSize,
	BinaryData::minimize_black_svgSize, BinaryData::minimize_hover_blue_svgSize, BinaryData::maximize_black_svgSize, BinaryData::maximize_hover_blue_svgSize, BinaryData::min_window_black_svgSize, BinaryData::min_window_hover_blue_svgSize,
    BinaryData::flip_black_svgSize, BinaryData::flip_blue_svgSize, BinaryData::restore_defaults_black_svgSize, BinaryData::restore_defaults_blue_svgSize,
	BinaryData::arrow_next_blue_svgSize, BinaryData::arrow_next_bw_svgSize, BinaryData::arrow_prev_blue_svgSize, BinaryData::arrow_prev_bw_svgSize, BinaryData::arrow_up_blue_svgSize, BinaryData::arrow_up_black_svgSize, BinaryData::arrow_down_blue_svgSize, BinaryData::arrow_down_black_svgSize, BinaryData::dropdown_arrow_bw_svgSize, BinaryData::dropdown_arrow_hover_blue_svgSize,
	BinaryData::Slider_Thumb_blue_svgSize, BinaryData::Slider_Thumb_bw_svgSize } };

FxThemeMode FxTheme::theme_mode_ = FxThemeMode::Dark;

//==============================================================================
FxTheme::FxTheme() : LookAndFeel_V4()
{
	init();
}

void FxTheme::init()
{
	setColourScheme(getFxColourScheme());

	setColour(ComboBox::ColourIds::arrowColourId, Colour(FXCOLOR(ImageButton)).withAlpha(1.0f));
	setColour(ComboBox::ColourIds::backgroundColourId, Colour(FXCOLOR(ComboBoxBackground)).withAlpha(1.0f));
	setColour(ComboBox::ColourIds::outlineColourId, Colour(FXCOLOR(ComboBoxBackground)).withAlpha(1.0f));
	setColour(ComboBox::ColourIds::focusedOutlineColourId, Colour(FXCOLOR(SliderHighlight)).withAlpha(0.2f));
	setColour(ComboBox::ColourIds::textColourId, Colour(FXCOLOR(DefaultText)).withAlpha(1.0f));
	setColour(TextEditor::ColourIds::backgroundColourId, Colour(FXCOLOR(DefaultFill)).withAlpha(1.0f));
	setColour(TextEditor::ColourIds::outlineColourId, Colour(FXCOLOR(DefaultFill)).withAlpha(1.0f));
	setColour(TextEditor::ColourIds::focusedOutlineColourId, Colour(FXCOLOR(DefaultFill)).withAlpha(1.0f));
	setColour(TextEditor::ColourIds::textColourId, Colour(FXCOLOR(DefaultText)).withAlpha(1.0f));
	setColour(TextEditor::ColourIds::highlightedTextColourId, Colour(FXCOLOR(HighlightedText)).withAlpha(1.0f));
	setColour(TextButton::ColourIds::buttonColourId, Colour(FXCOLOR(TextButtonBackground)).withAlpha(1.0f));
	setColour(TextButton::ColourIds::buttonOnColourId, Colour(FXCOLOR(TextButtonBackground)).withAlpha(1.0f));
	setColour(TextButton::ColourIds::textColourOffId, Colour(FXCOLOR(HighlightedText)).withAlpha(1.0f));
	setColour(TextButton::ColourIds::textColourOnId, Colour(FXCOLOR(HighlightedText)).withAlpha(1.0f));
	setColour(HyperlinkButton::ColourIds::textColourId, Colour(FXCOLOR(HighlightedText)).withAlpha(1.0f));
	setColour(CaretComponent::ColourIds::caretColourId, Colour(FXCOLOR(DefaultText)).withAlpha(1.0f));
	setColour(PopupMenu::ColourIds::backgroundColourId, Colour(FXCOLOR(DefaultFill)).withAlpha(0.55f));
	setColour(PopupMenu::ColourIds::highlightedBackgroundColourId, Colour(FXCOLOR(HighlightedFill)).withAlpha(0.28f));
	setColour(Slider::ColourIds::rotarySliderOutlineColourId, Colour(FXCOLOR(SliderTrack)).withAlpha(0.2f));
	setColour(Slider::ColourIds::rotarySliderFillColourId, Colour(FXCOLOR(SliderTrack)).withAlpha(1.0f));
	setColour(ScrollBar::thumbColourId, Colour(FXCOLOR(SliderTrack)).withAlpha(1.0f));

	font_400_ = linuxUiTypeface(Font::plain, BinaryData::GilroyRegular_ttf, BinaryData::GilroyRegular_ttfSize);
	font_600_ = linuxUiTypeface(Font::plain, BinaryData::GilroySemibold_ttf, BinaryData::GilroySemibold_ttfSize);
	font_700_ = linuxUiTypeface(Font::plain, BinaryData::GilroyBold_ttf, BinaryData::GilroyBold_ttfSize);

	drop_down_arrow_ = Drawable::createFromImageData(FXIMAGE(DropDownArrowHover), FXIMAGESIZE(DropDownArrowHover));
	drop_down_arrow_grey_ = Drawable::createFromImageData(FXIMAGE(DropDownArrow), FXIMAGESIZE(DropDownArrow));
}

LookAndFeel_V4::ColourScheme FxTheme::getFxColourScheme()
{
	return { Colour(FXCOLOR(WindowBackground)).withAlpha(WINDOW_FILL_ALPHA), Colour(FXCOLOR(WidgetBackground)).withAlpha(CARD_FILL_ALPHA), Colour(FXCOLOR(MenuBackground)).withAlpha(1.0f),
			 Colour(FXCOLOR(Outline)).withAlpha(1.0f), Colour(FXCOLOR(DefaultText)).withAlpha(1.0f), Colour(FXCOLOR(DefaultFill)).withAlpha(0.35f),
			 Colour(FXCOLOR(HighlightedText)).withAlpha(1.0f), Colour(FXCOLOR(HighlightedFill)).withAlpha(1.0f), Colour(FXCOLOR(MenuText)).withAlpha(1.0f) };
}

Label* FxTheme::createComboBoxTextBox(ComboBox& box)
{
	auto label = LookAndFeel_V4::createComboBoxTextBox(box);
	label->setMouseCursor(MouseCursor::PointingHandCursor);
	box.setMouseCursor(MouseCursor::PointingHandCursor);
	return label;
}

Font FxTheme::getComboBoxFont(ComboBox& box)
{
	ignoreUnused(box);
	return Font(font_600_).withHeight(14.0f);
}

void FxTheme::positionComboBoxText(ComboBox& box, Label& label)
{
	label.setMinimumHorizontalScale(1.0);
	LookAndFeel_V4::positionComboBoxText(box, label);
	label.setBounds(label.getBounds().withX(5).withRight(box.getWidth() - 37));
}

void FxTheme::drawComboBox(Graphics& g, int width, int height, bool,
							int, int, int, int, ComboBox& box)
{
	const float cornerSize = jmin(14.0f, (float) height * 0.5f);
	auto boxBounds = Rectangle<float>(0.0f, 0.0f, (float) width, (float) height).reduced(0.5f);

	g.setColour(Colours::white.withAlpha(0.10f));
	g.fillRoundedRectangle(boxBounds, cornerSize);
	g.setColour(box.findColour(ComboBox::backgroundColourId).withAlpha(0.38f));
	g.fillRoundedRectangle(boxBounds, cornerSize);

	g.setColour(Colours::white.withAlpha(0.28f));
	g.drawRoundedRectangle(boxBounds, cornerSize, 1.1f);

	if (box.hasKeyboardFocus(true))
	{
		g.setColour(Colour(FXCOLOR(HighlightedFill)).withAlpha(0.7f));
		g.drawRoundedRectangle(boxBounds.reduced(1.0f), cornerSize - 1.0f, 1.6f);
	}
	
	int margin = 32;
	if (width <= 150)
        margin = 24;

	g.setColour(Colour(FXCOLOR(DefaultText)).withAlpha(box.isEnabled() ? 0.85f : 0.30f));
	const float ax = (float) width - (float) margin + 2.0f;
	const float ay = (float) height * 0.5f;
	Path chevron;
	chevron.startNewSubPath(ax, ay - 3.2f);
	chevron.lineTo(ax + 5.5f, ay + 1.4f);
	chevron.lineTo(ax + 11.0f, ay - 3.2f);
	g.strokePath(chevron, PathStrokeType(1.6f, PathStrokeType::curved, PathStrokeType::rounded));
}

void FxTheme::drawComboBoxTextWhenNothingSelected(Graphics& g, ComboBox& box, Label& label)
{
	g.setColour(findColour(ComboBox::textColourId).withMultipliedAlpha(0.5f));

	auto font = label.getLookAndFeel().getLabelFont(label);

	g.setFont(font);

	auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds().withX(10));

	g.drawFittedText(box.getTextWhenNothingSelected(), textArea, label.getJustificationType(),
		jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
		label.getMinimumHorizontalScale());
}

void FxTheme::drawLinearSlider(Graphics& g, int x, int y, int width, int height,
								float sliderPos, float minSliderPos, float maxSliderPos,
								const Slider::SliderStyle style, Slider& slider)
{
	if (style == Slider::LinearVertical)
	{
		const float track_w = 8.0f;
		const float track_x = x + (width - track_w) / 2.0f;

        Colour colour = Colour(FXCOLOR(SliderTrack)).withAlpha(0.22f);
        if (!slider.isEnabled())
            colour = colour.withSaturation(0.0f);

		g.setColour(colour);
		g.fillRoundedRectangle(track_x, (float) y, track_w, (float) height, track_w / 2.0f);

		auto fill = Colour(FXCOLOR(SliderTrack)).withAlpha(1.0f);
		if (!slider.isEnabled())
			fill = fill.withSaturation(0.0f);
		const float fill_h = jmax(0.0f, (float) y + (float) height - sliderPos);
		g.setColour(fill);
		g.fillRoundedRectangle(track_x, sliderPos, track_w, fill_h, track_w / 2.0f);

        if (slider.getThumbBeingDragged() >= 0 || slider.hasKeyboardFocus(true))
        {
            Colour colour = Colour(FXCOLOR(SliderHighlight)).withAlpha(0.1f);
            g.setFillType(colour);
            g.fillRoundedRectangle(juce::Rectangle<float>(x + (width - SLIDER_THUMB_RADIUS*4) / 2, y, SLIDER_THUMB_RADIUS * 4, height).expanded(0, SLIDER_THUMB_RADIUS), 20);
        }
	}
	else if (style == Slider::LinearHorizontal)
	{
        Colour colour = Colour(FXCOLOR(SliderTrack)).withAlpha(0.2f);
        if (!slider.isEnabled())
        {
            colour = colour.withSaturation(0.0);
        }

		const float track_h = 8.0f;
		const float track_y = y + (height - track_h) / 2.0f;
		g.setFillType(colour);
		g.fillRoundedRectangle((float) x, track_y, (float) width, track_h, track_h / 2.0f);

        colour = Colour(FXCOLOR(SliderTrack)).withAlpha(1.0f);
        if (!slider.isEnabled())
        {
            colour = colour.withSaturation(0.0);
        }

		g.setFillType(colour);
		g.fillRoundedRectangle((float) x, track_y, jmax(0.0f, sliderPos - (float) x), track_h, track_h / 2.0f);
	
		if (slider.hasKeyboardFocus(true))
		{
			Colour colour = Colour(FXCOLOR(SliderHighlight)).withAlpha(0.1f);
			g.setFillType(colour);
			g.fillRoundedRectangle(juce::Rectangle<float>(x, y, width, height).expanded(SLIDER_THUMB_RADIUS/2, SLIDER_THUMB_RADIUS/2), height+SLIDER_THUMB_RADIUS);
		}
	}
	else
	{
		LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
	}
}

void FxTheme::drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
                                const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
    auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
    auto fill = slider.findColour(Slider::rotarySliderFillColourId);

    if (!slider.isEnabled())
    {
        outline = outline.withSaturation(0.0);
        fill = fill.withSaturation(0.0);
    }

    auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(1.5f);

    auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = 3.2f;
    auto arcRadius = radius - lineW * 0.5f - 1.0f;

    g.setColour(Colours::white.withAlpha(slider.isEnabled() ? 0.08f : 0.04f));
    g.fillEllipse(bounds.withSizeKeepingCentre(radius * 1.7f, radius * 1.7f));
    g.setColour(Colours::white.withAlpha(0.22f));
    g.drawEllipse(bounds.withSizeKeepingCentre(radius * 1.7f, radius * 1.7f), 1.0f);

    Path backgroundArc;
    backgroundArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        arcRadius,
        arcRadius,
        0.0f,
        rotaryStartAngle,
        rotaryEndAngle,
        true);

    g.setColour(outline);
    g.strokePath(backgroundArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));

    Path valueArc;
    valueArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        arcRadius,
        arcRadius,
        0.0f,
        rotaryStartAngle,
        toAngle,
        true);

    g.setColour(fill);
    g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
	if (slider.hasKeyboardFocus(true))
	{
		DropShadow shadow;
		shadow.colour = Colour(FXCOLOR(SliderHighlight)).withAlpha(0.1f);
		shadow.drawForPath(g, backgroundArc);
	}

    Point<float> thumbPoint(bounds.getCentreX() + arcRadius * std::cos(toAngle - MathConstants<float>::halfPi),
                            bounds.getCentreY() + arcRadius * std::sin(toAngle - MathConstants<float>::halfPi));
    g.setColour(fill);
    g.drawLine(bounds.getCentreX(), bounds.getCentreY(), thumbPoint.x, thumbPoint.y, 1.6f);
}

int FxTheme::getSliderThumbRadius(Slider& slider)
{
    if (slider.getSliderStyle() == Slider::Rotary)
    {
        return ROTARY_SLIDER_THUMB_RADIUS;
    }
    else
    {
        return SLIDER_THUMB_RADIUS;
    }
}

Slider::SliderLayout FxTheme::getSliderLayout(Slider& slider)
{
	auto layout = LookAndFeel_V4::getSliderLayout(slider);
	auto style = slider.getSliderStyle();

	if (style == Slider::LinearVertical)
	{
		auto y = layout.sliderBounds.getY() + (SLIDER_THUMB_RADIUS*2);
		auto height = layout.sliderBounds.getHeight() - (SLIDER_THUMB_RADIUS*2);
		layout.sliderBounds.setY(y);
		layout.sliderBounds.setHeight(height);
	}
	else if (style == Slider::LinearHorizontal)
	{
		auto width = layout.sliderBounds.getWidth() - (SLIDER_THUMB_RADIUS*4);
		layout.sliderBounds.setWidth(width);
	}

	return layout;
}

void FxTheme::drawPopupMenuItem(Graphics& g, const juce::Rectangle<int>& area, bool is_separator, bool is_active,
								bool is_highlighted, bool is_ticked, bool has_submenu, const String& text,
								const String& shortcut_key_text, const Drawable* icon, const Colour* text_colour)
{
	LookAndFeel_V4::drawPopupMenuItem(g, area, is_separator, is_active, is_highlighted||is_ticked, is_ticked,
										has_submenu, text, shortcut_key_text, icon, text_colour);
	
	if (is_ticked)
	{
		g.setColour(findColour(PopupMenu::textColourId).withAlpha(1.0f));
		g.drawRect(area.toFloat());
	}
}

Font FxTheme::getPopupMenuFont()
{
	return Font(font_600_).withHeight(17.0f);
}

void FxTheme::preparePopupMenuWindow(Component& new_window)
{
	new_window.setMouseCursor(MouseCursor::PointingHandCursor);
	auto children = new_window.getChildren();
	for (auto child : children)
	{
		child->setMouseCursor(MouseCursor::PointingHandCursor);
	}
}

void FxTheme::loadFont(String language)
{
    if (language.startsWithIgnoreCase("en"))
    {
        font_400_ = Typeface::createSystemTypefaceFor(BinaryData::GilroyRegular_ttf, BinaryData::GilroyRegular_ttfSize);
        font_600_ = Typeface::createSystemTypefaceFor(BinaryData::GilroySemibold_ttf, BinaryData::GilroySemibold_ttfSize);
        font_700_ = Typeface::createSystemTypefaceFor(BinaryData::GilroyBold_ttf, BinaryData::GilroyBold_ttfSize);
    }
    else if (language.startsWithIgnoreCase("ko"))
    {
        font_400_ = loadTypeface("NotoSansKR-Regular.otf");
        font_600_ = loadTypeface("NotoSansKR-Medium.otf");
        font_700_ = loadTypeface("NotoSansKR-Medium.otf");
    }
	else if (language.startsWithIgnoreCase("zh-CN"))
	{
		font_400_ = loadTypeface("NotoSansSC-Regular.otf");
		font_600_ = loadTypeface("NotoSansSC-Medium.otf");
		font_700_ = loadTypeface("NotoSansSC-Medium.otf");
	}
    else if (language.startsWithIgnoreCase("zh-TW"))
    {
        font_400_ = loadTypeface("NotoSansTC-Regular.ttf");
        font_600_ = loadTypeface("NotoSansTC-Medium.ttf");
        font_700_ = loadTypeface("NotoSansTC-Medium.ttf");
    }
	else if (language.startsWithIgnoreCase("th"))
	{
		font_400_ = loadTypeface("NotoSansThai-Regular.ttf");
		font_600_ = loadTypeface("NotoSansThai-Medium.ttf");
		font_700_ = loadTypeface("NotoSansThai-Medium.ttf");
	}
    else if (language.startsWithIgnoreCase("vi"))
    {
        font_400_ = loadTypeface("MontserratAlternates-Regular.ttf");
        font_600_ = loadTypeface("MontserratAlternates-Medium.ttf");
        font_700_ = loadTypeface("MontserratAlternates-Bold.ttf");
    }
	else if (language.startsWithIgnoreCase("ja"))
	{
		font_400_ = loadTypeface("NotoSansJP-Regular.ttf");
		font_600_ = loadTypeface("NotoSansJP-Medium.ttf");
		font_700_ = loadTypeface("NotoSansJP-Bold.ttf");
	}
	else if (language.startsWithIgnoreCase("ar"))
	{
		font_400_ = loadTypeface("IBMPlexSansArabic-Regular.ttf");
		font_600_ = loadTypeface("IBMPlexSansArabic-Medium.ttf");
		font_700_ = loadTypeface("IBMPlexSansArabic-Bold.ttf");
	}
	else if (language.startsWithIgnoreCase("fa"))
	{
		font_400_ = loadTypeface("IBMPlexSansArabic-Regular.ttf");
		font_600_ = loadTypeface("IBMPlexSansArabic-Medium.ttf");
		font_700_ = loadTypeface("IBMPlexSansArabic-Bold.ttf");
	}
    else
    {
        font_400_ = linuxUiTypeface(Font::plain, BinaryData::GilroyRegular_ttf, BinaryData::GilroyRegular_ttfSize);
        font_600_ = linuxUiTypeface(Font::plain, BinaryData::GilroySemibold_ttf, BinaryData::GilroySemibold_ttfSize);
        font_700_ = linuxUiTypeface(Font::plain, BinaryData::GilroyBold_ttf, BinaryData::GilroyBold_ttfSize);
    }
	
    if (font_400_ == nullptr)
    {
        font_400_ = Typeface::createSystemTypefaceFor(BinaryData::GilroyRegular_ttf, BinaryData::GilroyRegular_ttfSize);
    }
    if (font_600_ == nullptr)
    {
        font_600_ = Typeface::createSystemTypefaceFor(BinaryData::GilroySemibold_ttf, BinaryData::GilroySemibold_ttfSize);
    }
    if (font_700_ == nullptr)
    {
        font_700_ = Typeface::createSystemTypefaceFor(BinaryData::GilroyBold_ttf, BinaryData::GilroyBold_ttfSize);
    }

    setDefaultSansSerifTypeface(font_600_);
}

Font FxTheme::getTextButtonFont(TextButton&, int button_height)
{
	return Font(font_400_.get()).withHeight(jmin(15.0f, (float)button_height * 0.55f + 4.0f));
}

void FxTheme::drawButtonBackground(Graphics& g, Button& button, const Colour& backgroundColour,
	bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
	ignoreUnused(button);
	if (backgroundColour.getAlpha() < 12)
		return;

	auto r = button.getLocalBounds().toFloat().reduced(0.5f);
	const float rad = jmin(16.0f, r.getHeight() * 0.5f);
	const float alpha = shouldDrawButtonAsDown ? 0.18f : shouldDrawButtonAsHighlighted ? 0.14f : 0.08f;
	g.setColour(Colours::white.withAlpha(alpha));
	g.fillRoundedRectangle(r, rad);
	g.setColour(Colours::white.withAlpha(0.26f));
	g.drawRoundedRectangle(r, rad, 1.05f);
}

Font FxTheme::getNormalFont()
{
	return Font(font_400_).withHeight(14.5f);
}

Font FxTheme::getSmallFont()
{
	return Font(font_400_).withHeight(12.5f);
}

Font FxTheme::getTitleFont()
{
	return Font(font_400_).withHeight(15.0f);
}

void FxTheme::paintCoralMark(Graphics& g, Rectangle<float> bounds, Colour colour)
{
	g.setColour(colour);
	const float gap = jmax(1.2f, bounds.getWidth() * 0.07f);
	const float bar_w = (bounds.getWidth() - 3.0f * gap) / 4.0f;
	const float rel[] = { 0.42f, 0.92f, 0.62f, 0.36f };
	const float cy = bounds.getCentreY();
	for (int i = 0; i < 4; ++i)
	{
		const float h = jmax(bar_w, bounds.getHeight() * rel[i]);
		const float x = bounds.getX() + (float) i * (bar_w + gap);
		const float y = cy - h * 0.5f;
		g.fillRoundedRectangle(x, y, bar_w, h, bar_w * 0.5f);
	}
}

void FxTheme::paintGlassCard(Graphics& g, Rectangle<float> bounds, float radius)
{
	g.setColour(Colour(FXCOLOR(PanelBackground)).withAlpha(CARD_FILL_ALPHA));
	g.fillRoundedRectangle(bounds, radius);

	ColourGradient sheen(Colours::white.withAlpha(0.18f), bounds.getX(), bounds.getY(),
		Colours::white.withAlpha(0.03f), bounds.getX(), bounds.getBottom(), false);
	g.setGradientFill(sheen);
	g.fillRoundedRectangle(bounds, radius);

	g.setColour(Colours::white.withAlpha(0.28f));
	g.drawRoundedRectangle(bounds.reduced(0.7f), jmax(1.0f, radius - 0.6f), 1.15f);

	g.setColour(Colours::white.withAlpha(0.08f));
	g.drawRoundedRectangle(bounds.reduced(2.2f), jmax(1.0f, radius - 2.0f), 1.0f);
}

Typeface::Ptr FxTheme::getDefaultTypeface()
{
    return font_400_;
}

FxThemeMode FxTheme::getThemeMode()
{
	return theme_mode_;
}

void FxTheme::setThemeMode(FxThemeMode theme_mode)
{
	auto& theme = dynamic_cast<FxTheme&>(LookAndFeel::getDefaultLookAndFeel());

	theme_mode_ = theme_mode;

	theme.init();
}

uint32 FxTheme::getColor(FxColor color)
{
	return theme_colors_[theme_mode_][color];
}

const char* FxTheme::getImage(FxImage image)
{
	return theme_images_[theme_mode_][image];
}

const int FxTheme::getImageSize(FxImage image)
{
	return theme_image_sizes_[theme_mode_][image];
}

Rectangle<int> FxTheme::getTooltipBounds(const String& tipText, Point<int> screenPos, Rectangle<int> parentArea)
{
    const TextLayout tl(layoutTooltipText(tipText, Colours::black));

    auto w = (int)(tl.getWidth() + 20.0f);
    auto h = (int)(tl.getHeight() + 12.0f);

    return Rectangle<int>(screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 18) : screenPos.x + 36,
        screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 12) : screenPos.y + 12,
        w, h)
        .constrainedWithin(parentArea);
}

void FxTheme::drawTooltip(Graphics& g, const String& text, int width, int height)
{
    Rectangle<int> bounds(width, height);
    auto cornerSize = 5.0f;

    g.setColour(findColour(TooltipWindow::backgroundColourId));
    g.fillRoundedRectangle(bounds.toFloat(), cornerSize);

    g.setColour(findColour(TooltipWindow::outlineColourId));
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);

    layoutTooltipText(text, findColour(TooltipWindow::textColourId))
        .draw(g, bounds.toFloat().reduced(10, 0));
}

void FxTheme::drawDocumentWindowTitleBar(DocumentWindow& window, Graphics& g,
										int w, int h, int titleSpaceX, int titleSpaceW,
										const Image* icon, bool drawTitleTextOnLeft)
{
	if (w * h == 0)
		return;

	auto isActive = window.isActiveWindow();

	g.setColour(getCurrentColourScheme().getUIColour(ColourScheme::widgetBackground));
	g.fillAll();

	Font font(12.0f, Font::plain);
	g.setFont(font);

	auto textW = font.getStringWidth(window.getName());
	auto iconW = 0;
	auto iconH = 0;

	if (icon != nullptr)
	{
		iconH = static_cast<int> (font.getHeight());
		iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
	}

	textW = jmin(titleSpaceW, textW + iconW);
	auto textX = drawTitleTextOnLeft ? titleSpaceX
		: jmax(titleSpaceX, (w - textW) / 2);

	if (textX + textW > titleSpaceX + titleSpaceW)
		textX = titleSpaceX + titleSpaceW - textW;

	if (icon != nullptr)
	{
		g.setOpacity(isActive ? 1.0f : 0.6f);
		g.drawImageWithin(*icon, textX, (h - iconH) / 2, iconW, iconH,
			RectanglePlacement::centred, false);
		textX += iconW;
		textW -= iconW;
	}

	if (window.isColourSpecified(DocumentWindow::textColourId) || isColourSpecified(DocumentWindow::textColourId))
		g.setColour(window.findColour(DocumentWindow::textColourId));
	else
		g.setColour(getCurrentColourScheme().getUIColour(ColourScheme::defaultText));

	g.drawText(window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
}

class FxTheme_DocumentWindowButton : public Button
{
public:
	FxTheme_DocumentWindowButton(const String& name, Colour c, const Path& normal, const Path& toggled)
		: Button(name), colour(c), normalShape(normal), toggledShape(toggled)
	{
	}

	void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
	{
		auto background = Colours::grey;

		if (auto* rw = findParentComponentOfClass<ResizableWindow>())
			if (auto lf = dynamic_cast<LookAndFeel_V4*> (&rw->getLookAndFeel()))
				background = lf->getCurrentColourScheme().getUIColour(LookAndFeel_V4::ColourScheme::widgetBackground);

		g.fillAll(background);

		g.setColour((!isEnabled() || shouldDrawButtonAsDown) ? colour.withAlpha(0.6f)
			: colour);

		if (shouldDrawButtonAsHighlighted)
		{
			g.fillAll();
			g.setColour(background);
		}

		auto& p = getToggleState() ? toggledShape : normalShape;

		auto reducedRect = Justification(Justification::centred)
			.appliedToRectangle(Rectangle<int>(12, 12), getLocalBounds())
			.toFloat();

		g.fillPath(p, p.getTransformToScaleToFit(reducedRect, true));
	}

private:
	Colour colour;
	Path normalShape, toggledShape;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FxTheme_DocumentWindowButton)
};

Button* FxTheme::createDocumentWindowButton(int buttonType)
{
	Path shape;
	auto crossThickness = 0.15f;

	if (buttonType == DocumentWindow::closeButton)
	{
		shape.addLineSegment({ 0.0f, 0.0f, 1.0f, 1.0f }, crossThickness);
		shape.addLineSegment({ 1.0f, 0.0f, 0.0f, 1.0f }, crossThickness);

		return new FxTheme_DocumentWindowButton("close", Colour(FXCOLOR(DefaultFill)).withAlpha(1.0f), shape, shape);
	}

	if (buttonType == DocumentWindow::minimiseButton)
	{
		shape.addLineSegment({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

		return new FxTheme_DocumentWindowButton("minimise", Colour(FXCOLOR(DefaultFill)).withAlpha(1.0f), shape, shape);
	}

	if (buttonType == DocumentWindow::maximiseButton)
	{
		shape.addLineSegment({ 0.5f, 0.0f, 0.5f, 1.0f }, crossThickness);
		shape.addLineSegment({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

		Path fullscreenShape;
		fullscreenShape.startNewSubPath(45.0f, 100.0f);
		fullscreenShape.lineTo(0.0f, 100.0f);
		fullscreenShape.lineTo(0.0f, 0.0f);
		fullscreenShape.lineTo(100.0f, 0.0f);
		fullscreenShape.lineTo(100.0f, 45.0f);
		fullscreenShape.addRectangle(45.0f, 45.0f, 100.0f, 100.0f);
		PathStrokeType(30.0f).createStrokedPath(fullscreenShape, fullscreenShape);

		return new FxTheme_DocumentWindowButton("maximise", Colour(FXCOLOR(DefaultFill)).withAlpha(1.0f), shape, fullscreenShape);
	}

	jassertfalse;
	return nullptr;
}

TextLayout FxTheme::layoutTooltipText(const String& text, Colour colour) noexcept
{
    const float tooltipFontSize = 14.0f;
    const int maxToolTipWidth = 400;

    AttributedString s;
    s.setWordWrap(AttributedString::WordWrap::byWord);
    s.setJustification(Justification::centredLeft);
    s.append(text, getNormalFont().withHeight(tooltipFontSize), colour);

    TextLayout tl;
    tl.createLayout(s, (float)maxToolTipWidth);
    return tl;
}

Typeface::Ptr FxTheme::loadTypeface(String fileName)
{
    MemoryBlock fontBuffer;
    String filePath = File::addTrailingSeparator(File::getCurrentWorkingDirectory().getFullPathName());
    File fontFile = File(filePath+fileName);
    if (fontFile.exists())
    {
        if (fontFile.loadFileAsData(fontBuffer))
        {
            return Typeface::createSystemTypefaceFor(fontBuffer.getData(), fontBuffer.getSize());
        }
    }
        
    return nullptr;
}