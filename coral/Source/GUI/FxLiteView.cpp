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
#include "FxLiteView.h"
#include "FxController.h"
#include "FxTheme.h"

//==============================================================================
FxLiteView::FxLiteView()
{	
	setOpaque(false);

	auto& theme = dynamic_cast<FxTheme&>(LookAndFeel::getDefaultLookAndFeel());
	playback_label_.setFont(theme.getTitleFont());
	enhance_label_.setFont(theme.getTitleFont());
	playback_label_.setColour(Label::textColourId, Colour(FXCOLOR(DefaultText)).withAlpha(1.0f));
	enhance_label_.setColour(Label::textColourId, Colour(FXCOLOR(DefaultText)).withAlpha(1.0f));

	addAndMakeVisible(playback_label_);
	addAndMakeVisible(enhance_label_);
	addAndMakeVisible(effects_);
	addAndMakeVisible(visualizer_);
	effects_.update();
	effects_.showValues(true);
}

void FxLiteView::startVisualizer()
{
	visualizer_.start();
}

void FxLiteView::pauseVisualizer()
{
	visualizer_.pause();
}

void FxLiteView::resized()
{
	auto area = getLocalBounds().reduced(24, 20);
	visualizer_.setBounds(area.removeFromBottom(56).reduced(12, 8));
	area.removeFromBottom(16);

	auto playback = area.removeFromTop(116).reduced(20, 14);
	playback_label_.setBounds(playback.removeFromTop(20));
	playback.removeFromTop(6);
	auto row = playback.removeFromTop(36);
	preset_list_.setBounds(row.removeFromLeft((row.getWidth() - 8) / 2));
	row.removeFromLeft(8);
	endpoint_list_.setBounds(row);

	area.removeFromTop(16);
	auto enhance = area.reduced(16, 10);
	enhance_label_.setBounds(enhance.removeFromTop(20));
	enhance.removeFromTop(6);
	effects_.setBounds(enhance);
}

void FxLiteView::paint(Graphics& g)
{
	auto area = getLocalBounds().reduced(24, 20);
	auto vis = area.removeFromBottom(56);
	area.removeFromBottom(16);
	auto playback = area.removeFromTop(116);
	area.removeFromTop(16);
	auto enhance = area;
	const float r = (float) FxTheme::CARD_CORNER_RADIUS;

	FxTheme::paintGlassCard(g, playback.toFloat(), r);
	FxTheme::paintGlassCard(g, enhance.toFloat(), r);
	FxTheme::paintGlassCard(g, vis.toFloat(), r);

	g.setColour(Colour(FXCOLOR(HighlightedFill)));
	auto mark = [](Graphics& g, const Label& label)
	{
		auto b = label.getBounds().toFloat();
		g.fillRoundedRectangle(b.getX() - 10.0f, b.getY() + 3.0f, 3.5f, b.getHeight() - 5.0f, 1.6f);
	};
	mark(g, playback_label_);
	mark(g, enhance_label_);

	auto power_state = FxModel::getModel().getPowerState();
	preset_list_.setEnabled(true);
	endpoint_list_.setEnabled(true);
	effects_.setEnabled(power_state);
	visualizer_.setEnabled(power_state);
}