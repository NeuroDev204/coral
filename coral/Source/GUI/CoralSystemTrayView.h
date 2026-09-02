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

#ifndef FXSYSTEMTRAYVIEW_H
#define FXSYSTEMTRAYVIEW_H

#include <JuceHeader.h>
#include "CoralModel.h"
#include "CoralController.h"
#include "CoralNotification.h"
#include "CoralSettingsDialog.h"

//==============================================================================
/*
*/

class CoralSystemTrayView : public Component, public CoralModel::Listener
#if !defined(_WIN32)
    , private Timer
#endif
{
public:
	CoralSystemTrayView();
    ~CoralSystemTrayView();

	void modelChanged(CoralModel::Event model_event) override;
	void setStatus(bool power, bool processing);
	Point<int> getSystemTrayWindowPosition(int width, int height);

private:
	static constexpr int MENU_ID_OPEN = 1;
	static constexpr int MENU_ID_POWER = 2;
	static constexpr int MENU_ID_SETTINGS = 3;
    static constexpr int MENU_ID_DONATE = 4;
	static constexpr int MENU_ID_EXIT = 5;
	static constexpr int PRESET_MENU_ID_START = 101;
	static constexpr int OUTPUT_MENU_ID_START = 201;
#if defined(_WIN32)
	static constexpr int WMAPP_FXTRAYICON = WM_APP + 1;
	static const GUID trayIconGuid_;

	static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

	void addIcon();
	void showContextMenu();
	void addOutputDeviceMenu(PopupMenu* context_menu);
	void showNotification();
	String getTruncatedText(const String& text, int max_length);
#if !defined(_WIN32)
	void timerCallback() override;
#endif

	bool custom_notification_;
	CoralNotification notification_;
#if defined(_WIN32)
	WNDPROC componentWndProc_;
	UINT taskbar_created_message_;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CoralSystemTrayView)
};

#endif