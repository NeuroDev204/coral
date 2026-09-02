#pragma once
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
#if defined(_WIN32)
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#endif

#include "../JuceLibraryCode/JuceHeader.h"

class SysInfo
{
public:
	SysInfo() = default;
	~SysInfo() = default;

	static void enumAudioOutputs(StringArray& audioOutputNames);
#if defined(_WIN32)
    static bool isServiceRunning(LPCWSTR service_name);
#endif
    static bool canSupportHotkeys();
	static bool isRemoteSession();

private:
#if defined(_WIN32)
	static String getAudioOutputName(IMMDevice *pDevice);
#endif
};

