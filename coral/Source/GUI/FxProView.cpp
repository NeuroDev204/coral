/*
FxSound
Copyright (C) 2025  FxSound LLC

Contributors:
	www.theremino.com (2025)

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
#include "FxProView.h"
#include "FxController.h"
#include "FxTheme.h"

//==============================================================================
FxProView::FxProView() : tool_tip_(this)
{
	sidebar_viewport_.setViewedComponent(&audio_controls_, false);
	sidebar_viewport_.setScrollBarsShown(true, false);
	sidebar_viewport_.setScrollBarThickness(8);
	sidebar_viewport_.setSingleStepSizes(0, 28);
	sidebar_viewport_.setOpaque(false);
	addAndMakeVisible(sidebar_viewport_);
	addAndMakeVisible(equalizer_);
    addChildComponent(visualizer_);

	equalizer_.addMouseListener(this, true);
	audio_controls_.addMouseListener(this, true);

    auto& theme = dynamic_cast<LookAndFeel_V4&>(getLookAndFeel());

    tool_tip_.setColour(TooltipWindow::ColourIds::textColourId, theme.getCurrentColourScheme().getUIColour(LookAndFeel_V4::ColourScheme::defaultText));
    tool_tip_.setOpaque(false);

	setOpaque(false);
}

void FxProView::startVisualizer()
{
	visualizer_.start();
}

void FxProView::pauseVisualizer()
{
	visualizer_.pause();
}

void FxProView::update()
{
	audio_controls_.update();
	equalizer_.update();

	visualizer_.calcGradient();

    if (FxController::getInstance().isAudioProcessing())
    {
        visualizer_.reset();
    }

	visualizer_.setVisible(true);
	equalizer_.showValues(true);
	audio_controls_.showValues(true);
}

void FxProView::setLookAndFeel()
{
	audio_controls_.setLookAndFeel();
}

void FxProView::resized()
{
	auto area = getLocalBounds().reduced(22, 16);
	visualizer_.setBounds(area.removeFromBottom(68).reduced(10, 6));
	area.removeFromBottom(14);

	auto top = area.removeFromTop(36);
	preset_list_.setBounds(top.removeFromLeft((top.getWidth() - 8) / 2));
	top.removeFromLeft(8);
	endpoint_list_.setBounds(top);

	area.removeFromTop(12);
	auto sidebar = area.removeFromRight(SIDEBAR_WIDTH);
	area.removeFromRight(14);
	equalizer_.setBounds(area);
	sidebar_viewport_.setBounds(sidebar.reduced(4, 8));
	const int inner_w = jmax(180, sidebar_viewport_.getWidth() - sidebar_viewport_.getScrollBarThickness());
	audio_controls_.setSize(inner_w, SIDEBAR_CONTENT_HEIGHT);
}

void FxProView::paint(Graphics& g)
{
	auto area = getLocalBounds().reduced(22, 16);
	auto vis = area.removeFromBottom(68);
	area.removeFromBottom(14);
	area.removeFromTop(36 + 12);
	auto sidebar = area.removeFromRight(SIDEBAR_WIDTH);
	area.removeFromRight(14);
	const float r = (float) FxTheme::CARD_CORNER_RADIUS;

	FxTheme::paintGlassCard(g, area.toFloat(), r);
	FxTheme::paintGlassCard(g, sidebar.toFloat(), r);
	FxTheme::paintGlassCard(g, vis.toFloat(), r);

	auto enable_controls = FxModel::getModel().getPowerState();
	preset_list_.setEnabled(true);
	endpoint_list_.setEnabled(true);
	audio_controls_.setEnabled(enable_controls);
	equalizer_.setEnabled(enable_controls);
	visualizer_.setEnabled(enable_controls);
}

void FxProView::comboBoxChanged(ComboBox* combobox)
{
	FxView::comboBoxChanged(combobox);
}

void FxProView::modelChanged(FxModel::Event model_event)
{
	FxView::modelChanged(model_event);

	if (model_event == FxModel::Event::PresetSelected)
	{
		update();
	}
}

void FxProView::mouseEnter(const MouseEvent& mouse_event)
{
	FxView::mouseEnter(mouse_event);
}

void FxProView::mouseExit(const MouseEvent& mouse_event)
{
	FxView::mouseEnter(mouse_event);
}