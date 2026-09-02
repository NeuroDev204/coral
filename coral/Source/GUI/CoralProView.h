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
#include "CoralView.h"
#include "CoralAudioControls.h"
#include "CoralEqualizer.h"
#include "CoralVisualizer.h"

//==============================================================================
/*
*/
class CoralProView  : public CoralView
{
public:
    CoralProView();
    ~CoralProView() = default;

	void startVisualizer();
	void pauseVisualizer();

	void update();

    void setLookAndFeel();

private:
	static constexpr int SIDEBAR_WIDTH = 248;
	static constexpr int SIDEBAR_CONTENT_HEIGHT = 580;

	void resized() override;
	void paint(Graphics& g) override;
	void comboBoxChanged(ComboBox* combobox) override;
	void modelChanged(CoralModel::Event model_event) override;

	void mouseEnter(const MouseEvent& mouse_event) override;
	void mouseExit(const MouseEvent& mouse_event) override;

	CoralAudioControls audio_controls_;
	Viewport sidebar_viewport_;
	CoralEqualizer equalizer_;
    TooltipWindow tool_tip_;
    CoralVisualizer visualizer_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoralProView)
};
