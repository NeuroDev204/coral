#include "u_AudioPassthru.h"
#include "codedefs.h"
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/pulseaudio.h>
#include <thread>
#include <atomic>
#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <cctype>

AudioPassthruCallback* AudioPassthruPrivate::s_callback_ = nullptr;

namespace
{
	constexpr const char* kCoralSink = "CoralSink";

	bool isAppVirtualSink(const std::string& name)
	{
		return name.find("CoralSink") != std::string::npos
			|| name.find("FxSoundSink") != std::string::npos;
	}

	std::string shellQuote(const std::string& value)
	{
		std::string out = "'";
		for (char c : value)
		{
			if (c == '\'')
				out += "'\\''";
			else
				out += c;
		}
		out += "'";
		return out;
	}

	void pactlRun(const std::string& cmd)
	{
		std::string full = cmd + " 2>/dev/null";
		system(full.c_str());
	}

	// Hardware sink volume is independent of CoralSink. GNOME's slider only
	// moves the default sink (CoralSink), so a leftover 50% on headphones
	// makes output quieter than the UI suggests and volume keys cannot raise it.
	void setSinkUnityGain(const std::string& sink)
	{
		if (sink.empty() || isAppVirtualSink(sink))
			return;
		const std::string quoted = shellQuote(sink);
		pactlRun("pactl set-sink-mute " + quoted + " 0");
		pactlRun("pactl set-sink-volume " + quoted + " 100%");
	}

	std::string getSinkVolumePercent(const std::string& sink)
	{
		if (sink.empty())
			return "";
		std::string cmd = "pactl get-sink-volume " + shellQuote(sink) + " 2>/dev/null";
		FILE* pipe = popen(cmd.c_str(), "r");
		if (!pipe)
			return "";
		char line[512];
		std::string out;
		if (fgets(line, sizeof(line), pipe))
			out = line;
		pclose(pipe);
		const auto pct = out.find('%');
		if (pct == std::string::npos)
			return "";
		auto start = out.rfind(' ', pct);
		if (start == std::string::npos)
			start = 0;
		else
			++start;
		return out.substr(start, pct - start + 1);
	}

	void copySinkVolume(const std::string& from_sink, const std::string& to_sink)
	{
		if (from_sink.empty() || to_sink.empty())
			return;
		const std::string percent = getSinkVolumePercent(from_sink);
		if (percent.empty())
			return;
		pactlRun("pactl set-sink-mute " + shellQuote(to_sink) + " 0");
		pactlRun("pactl set-sink-volume " + shellQuote(to_sink) + " " + percent);
	}
}

static std::wstring stringToWstring(const std::string& str) {
    if (str.empty()) return L"";
    std::vector<wchar_t> buf(str.size() + 1, 0);
    mbstowcs(buf.data(), str.c_str(), str.size());
    return std::wstring(buf.data());
}

static std::string wstringToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    std::vector<char> buf(wstr.size() * 4 + 1, 0);
    wcstombs(buf.data(), wstr.c_str(), buf.size());
    return std::string(buf.data());
}

struct LinuxSinkInfo {
    std::string name;
    std::string description;
    std::string active_port;
    bool is_running = false;
    bool port_available = true; // false when pactl says "not available" (unplugged jack)
    int priority = 0; // Higher = preferred
};

static std::string toLowerCopy(std::string s) {
    for (auto& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

static bool sinkHaystackHas(const LinuxSinkInfo& s, const char* token) {
    const std::string hay = toLowerCopy(s.name + " " + s.description + " " + s.active_port);
    return hay.find(token) != std::string::npos;
}

static bool isHdmiDisplayPortSink(const LinuxSinkInfo& s) {
    return sinkHaystackHas(s, "hdmi") || sinkHaystackHas(s, "displayport");
}

static bool isAnalogLikeSink(const LinuxSinkInfo& s) {
    if (isHdmiDisplayPortSink(s)) return false;
    return sinkHaystackHas(s, "speaker") || sinkHaystackHas(s, "headphone") ||
           sinkHaystackHas(s, "analog") || sinkHaystackHas(s, "usb") ||
           sinkHaystackHas(s, "bluez") || sinkHaystackHas(s, "bluetooth");
}

static std::vector<LinuxSinkInfo> enumerateHardwareSinks() {
    std::vector<LinuxSinkInfo> sinks;
    FILE* pipe = popen("pactl list sinks 2>/dev/null", "r");
    if (!pipe) return sinks;

    char line[512];
    LinuxSinkInfo current;
    bool in_sink = false;

    while (fgets(line, sizeof(line), pipe)) {
        std::string s(line);
        size_t first = s.find_first_not_of(" \t\r\n");
        size_t last = s.find_last_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        std::string trimmed = s.substr(first, (last - first + 1));

        if (trimmed.rfind("Sink #", 0) == 0) {
            if (in_sink && !current.name.empty() && !isAppVirtualSink(current.name)) {
                sinks.push_back(current);
            }
            current = LinuxSinkInfo();
            in_sink = true;
        } else if (trimmed.rfind("Name: ", 0) == 0) {
            current.name = trimmed.substr(6);
        } else if (trimmed.rfind("Description: ", 0) == 0) {
            current.description = trimmed.substr(13);
        } else if (trimmed.rfind("Active Port: ", 0) == 0) {
            current.active_port = trimmed.substr(13);
        } else if (trimmed.rfind("State: RUNNING", 0) == 0) {
            current.is_running = true;
        } else if (trimmed.find("[Out]") != std::string::npos) {
            // PipeWire reports jack state on the port line, e.g.
            // "[Out] Headphones: Headphones (..., available)" vs "(..., not available)".
            if (trimmed.find("not available") != std::string::npos) {
                current.port_available = false;
            }
        }
    }
    if (in_sink && !current.name.empty() && !isAppVirtualSink(current.name)) {
        sinks.push_back(current);
    }
    pclose(pipe);

    // USB / Bluetooth > headphone jack > internal speakers. HDMI/DP is last-resort:
    // this laptop's HDMI3 sink is often a silent DisplayPort jack, and once we
    // write to it PipeWire marks it RUNNING which would otherwise keep winning.
    for (auto& s : sinks) {
        s.priority = 10;
        const bool hdmi = isHdmiDisplayPortSink(s);

        if (!s.port_available) {
            s.priority -= 1000;
        } else if (s.is_running && !hdmi) {
            s.priority += 50;
        }

        if (sinkHaystackHas(s, "usb")) {
            s.priority += 400;
        } else if (sinkHaystackHas(s, "bluez") || sinkHaystackHas(s, "bluetooth")) {
            s.priority += 350;
        } else if (sinkHaystackHas(s, "headphone")) {
            s.priority += 250;
        } else if (sinkHaystackHas(s, "speaker")) {
            s.priority += 150;
        }

        if (sinkHaystackHas(s, "analog") && !hdmi) {
            s.priority += 30;
        }
        if (hdmi) {
            s.priority -= 200;
        }
    }

    // Sort sinks by priority descending (best output device first)
    std::sort(sinks.begin(), sinks.end(), [](const LinuxSinkInfo& a, const LinuxSinkInfo& b) {
        return a.priority > b.priority;
    });

    return sinks;
}

static std::string analogSinkSignature(const std::vector<LinuxSinkInfo>& sinks) {
    std::string sig;
    for (const auto& s : sinks) {
        if (!s.port_available || isHdmiDisplayPortSink(s)) continue;
        sig += s.name;
        sig += '|';
    }
    return sig;
}

static std::string resolveBestHardwareSink(const std::string& preferred_guid) {
    auto sinks = enumerateHardwareSinks();
    if (sinks.empty()) return "";

    auto isUsable = [](const LinuxSinkInfo& s) {
        return s.port_available && !isAppVirtualSink(s.name);
    };

    // Always follow the live analog jack: plugged headphones beat speakers,
    // unplugging drops the headphones sink/port so speakers win. A saved GUID
    // that is gone or "not available" must not keep us writing to a dead jack.
    const LinuxSinkInfo* best_analog = nullptr;
    const LinuxSinkInfo* best_any = nullptr;
    for (const auto& s : sinks) {
        if (!isUsable(s)) continue;
        if (best_any == nullptr) best_any = &s;
        if (isAnalogLikeSink(s) && best_analog == nullptr) best_analog = &s;
    }

    const LinuxSinkInfo* auto_pick = best_analog != nullptr ? best_analog : best_any;
    if (auto_pick == nullptr) return "";

    if (!preferred_guid.empty() && preferred_guid != "default" && preferred_guid != "pulseaudio_default") {
        const LinuxSinkInfo* preferred = nullptr;
        for (const auto& s : sinks) {
            if (s.name == preferred_guid) {
                preferred = &s;
                break;
            }
        }
        if (preferred != nullptr && isUsable(*preferred) && !isHdmiDisplayPortSink(*preferred)) {
            // Keep an explicit USB/BT choice while it is still plugged in.
            // Headphone jack still wins over speakers/HDMI.
            if (isAnalogLikeSink(*preferred) && preferred->priority >= auto_pick->priority) {
                return preferred->name;
            }
        }
    }

    return auto_pick->name;
}

std::string getDefaultSinkName() {
    return resolveBestHardwareSink("");
}

static void unloadCoralModules()
{
	FILE* pipe = popen("pactl list modules short 2>/dev/null | grep -iE 'fxsound|coralsink' | awk '{print $1}'", "r");
	if (pipe) {
		char buffer[64];
		std::vector<std::string> modules_to_unload;
		while (fgets(buffer, sizeof(buffer), pipe)) {
			std::string line(buffer);
			while (!line.empty() && (line.back() == '\n' || line.back() == '\r' || line.back() == ' ' || line.back() == '\t')) line.pop_back();
			if (!line.empty()) {
				modules_to_unload.push_back(line);
			}
		}
		pclose(pipe);

		for (const auto& mod : modules_to_unload) {
			std::string cmd = "pactl unload-module " + mod + " 2>/dev/null";
			system(cmd.c_str());
		}
	}
}

AudioPassthruPrivate::AudioPassthruPrivate()
{
	hProcessingThread_ = NULL;
	ProcessingThreadID_ = (DWORD)0;
	i_kill_processing_thread_ = IS_FALSE;
	b_no_valid_snd_device_dialog_shown_ = false;
	debug_ = IS_TRUE;
	mute_ = false;
	p_dfx_dsp_ = nullptr;
	hw_sink_name_ = "";
	saved_default_sink_ = "";
	targeted_playback_device_guid_ = "";
	last_device_signature_ = "";
}

AudioPassthruPrivate::~AudioPassthruPrivate()
{
	int i_timed_out;
	killProcessingThread(&i_timed_out);
	restoreDefaultPlaybackDevice();
}

int AudioPassthruPrivate::init()
{
	unloadCoralModules();

	hw_sink_name_ = getDefaultSinkName();
	saved_default_sink_ = hw_sink_name_;

	std::string load_cmd = std::string("pactl load-module module-null-sink sink_name=") + kCoralSink
		+ " sink_properties=device.description=Coral 2>/dev/null";
	system(load_cmd.c_str());
	std::string default_cmd = std::string("pactl set-default-sink ") + kCoralSink + " 2>/dev/null";
	system(default_cmd.c_str());
	pactlRun(std::string("pactl set-sink-mute ") + kCoralSink + " 0");
	pactlRun(std::string("pactl set-sink-volume ") + kCoralSink + " 100%");
	setSinkUnityGain(hw_sink_name_);

	i_kill_processing_thread_ = IS_FALSE;
	std::thread processing_thread([this]() { this->threadWorker(); });
	processing_thread.detach();

	return OKAY;
}

void AudioPassthruPrivate::mute(bool mute)
{
	mute_ = mute;
}

std::vector<SoundDevice> AudioPassthruPrivate::getSoundDevices(bool active_devices)
{
	std::vector<SoundDevice> devices;
	auto sinks = enumerateHardwareSinks();
	std::string current_best = resolveBestHardwareSink(targeted_playback_device_guid_);

	for (const auto& s : sinks) {
		if (!s.port_available) continue;
		SoundDevice dev;
		std::string label = s.description.empty() ? s.name : s.description;
		dev.deviceFriendlyName = stringToWstring(label);
		dev.pwszID = stringToWstring(s.name);
		dev.isRealDevice = true;
		dev.isPlaybackDevice = true;
		dev.isActive = true;
		dev.isDefaultDevice = (s.name == current_best);
		dev.isTargetedRealPlaybackDevice = (s.name == current_best);
		dev.deviceNumChannel = 2;
		devices.push_back(dev);
	}

	if (devices.empty()) {
		SoundDevice default_device;
		default_device.deviceFriendlyName = L"System Audio (Default)";
		default_device.pwszID = L"pulseaudio_default";
		default_device.isRealDevice = true;
		default_device.isActive = true;
		default_device.isDefaultDevice = true;
		default_device.deviceNumChannel = 2;
		devices.push_back(default_device);
	}

	return devices;
}

int AudioPassthruPrivate::killProcessingThread(int *ip_timed_out)
{
	i_kill_processing_thread_ = IS_TRUE;
	*ip_timed_out = 0;
	return OKAY;
}

int AudioPassthruPrivate::setBufferLength(int i_buffer_length_msecs)
{
	return OKAY;
}

int AudioPassthruPrivate::processTimer()
{
	return OKAY;
}

void AudioPassthruPrivate::setDspProcessingModule(DfxDsp* p_dfx_dsp)
{
	p_dfx_dsp_ = p_dfx_dsp;
	if (p_dfx_dsp_)
	{
		p_dfx_dsp_->setSignalFormat(16, 2, 48000, 16);
	}
}

DWORD WINAPI AudioPassthruPrivate::processingThread(LPVOID lpParam)
{
	return 0;
}

DWORD AudioPassthruPrivate::threadWorker(void)
{
    // Use 16-bit 48kHz stereo format (matching PipeWire native sample rate)
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 48000,
        .channels = 2
    };

    // Buffer attributes for low latency and smooth streaming
    pa_buffer_attr buffer_attr;
    buffer_attr.maxlength = (uint32_t)-1;
    buffer_attr.tlength = pa_usec_to_bytes(20000, &ss); // 20ms target latency
    buffer_attr.prebuf = (uint32_t)-1;
    buffer_attr.minreq = pa_usec_to_bytes(5000, &ss);   // 5ms min request
    buffer_attr.fragsize = pa_usec_to_bytes(5000, &ss); // 5ms fragment size

    int error = 0;

    usleep(50000);

    std::string monitor = std::string(kCoralSink) + ".monitor";
    pa_simple *s_read = pa_simple_new(NULL, "Coral", PA_STREAM_RECORD, monitor.c_str(), "Capture", &ss, NULL, &buffer_attr, &error);
    if (!s_read) {
        std::cerr << "Failed to create read connection: " << pa_strerror(error) << std::endl;
        return 1;
    }

    const int num_samples = 512; // 512 sample sets = ~10.6ms at 48kHz
    const int buf_size = num_samples * 2 * sizeof(short);
    short *input_buffer = (short*)malloc(buf_size);
    short *output_buffer = (short*)malloc(buf_size);

    memset(input_buffer, 0, buf_size);
    memset(output_buffer, 0, buf_size);

    bool format_initialized = false;
    std::string current_hw_sink = "";
    pa_simple *s_write = nullptr;

    auto open_write_stream = [&](const std::string& target_sink_name) -> pa_simple* {
        int err = 0;
        const char* target = (!target_sink_name.empty()) ? target_sink_name.c_str() : NULL;
        pa_simple *w = pa_simple_new(NULL, "Coral", PA_STREAM_PLAYBACK, target, "Playback", &ss, NULL, &buffer_attr, &err);
        if (!w && target != NULL) {
            w = pa_simple_new(NULL, "Coral", PA_STREAM_PLAYBACK, NULL, "Playback", &ss, NULL, &buffer_attr, &err);
        }
        return w;
    };

    int check_counter = 0;

    while (!i_kill_processing_thread_) {
        // Automatically check for headphone/device hotplug or change every 50 chunks (~500ms)
        if (++check_counter % 50 == 0 || !s_write) {
            std::string best_sink = resolveBestHardwareSink(targeted_playback_device_guid_);
            if (best_sink != current_hw_sink || !s_write) {
                if (s_write) {
                    pa_simple_free(s_write);
                    s_write = nullptr;
                }
                s_write = open_write_stream(best_sink);
                if (s_write) {
                    current_hw_sink = best_sink;
                    hw_sink_name_ = best_sink;
                    setSinkUnityGain(best_sink);
                    std::cerr << "DSP: Output automatically routed to -> " << (current_hw_sink.empty() ? "Default" : current_hw_sink) << std::endl;
                }
            }
        }

        // Read audio from monitor (captured output)
        int read_result = pa_simple_read(s_read, input_buffer, buf_size, &error);
        if (read_result < 0) {
            usleep(5000);
            continue;
        }

        int dsp_res = -1;
        if (p_dfx_dsp_) {
            if (!format_initialized) {
                p_dfx_dsp_->setSignalFormat(16, 2, 48000, 16);
                format_initialized = true;
            }
            dsp_res = p_dfx_dsp_->processAudio(input_buffer, output_buffer, num_samples, 0);
            if (dsp_res != 0) {
                memcpy(output_buffer, input_buffer, buf_size);
            }
        } else {
            memcpy(output_buffer, input_buffer, buf_size);
        }

        // If DSP output was all zero despite non-zero input, passthrough directly
        short in_peak_curr = 0, out_peak_curr = 0;
        for (int k = 0; k < num_samples * 2; ++k) {
            if (abs(input_buffer[k]) > in_peak_curr) in_peak_curr = abs(input_buffer[k]);
            if (abs(output_buffer[k]) > out_peak_curr) out_peak_curr = abs(output_buffer[k]);
        }
        if (in_peak_curr > 0 && out_peak_curr == 0) {
            memcpy(output_buffer, input_buffer, buf_size);
        }

        // Mute if requested
        if (mute_) {
            memset(output_buffer, 0, buf_size);
        }

        // Write processed audio to hardware
        if (s_write) {
            if (pa_simple_write(s_write, output_buffer, buf_size, &error) < 0) {
                // If write fails (e.g. device disconnected / unplugged), trigger reconnect
                pa_simple_free(s_write);
                s_write = nullptr;
                current_hw_sink = "";
            }
        }
    }

    free(input_buffer);
    free(output_buffer);
    pa_simple_free(s_read);
    if (s_write) {
        pa_simple_free(s_write);
    }
    return 0;
}

int AudioPassthruPrivate::setTargetedRealPlaybackDevice(const std::wstring sound_device_guid)
{
	targeted_playback_device_guid_ = wstringToString(sound_device_guid);
	return OKAY;
}

void AudioPassthruPrivate::registerCallback(AudioPassthruCallback *callback)
{
	s_callback_ = callback;
}

bool AudioPassthruPrivate::isPlaybackDeviceAvailable()
{
	return true;
}

bool AudioPassthruPrivate::checkDeviceChanges()
{
	const auto sinks = enumerateHardwareSinks();
	const std::string sig = analogSinkSignature(sinks);
	if (sig == last_device_signature_) {
		return false;
	}
	last_device_signature_ = sig;
	return true;
}

void AudioPassthruPrivate::restoreDefaultPlaybackDevice()
{
	std::string hw = saved_default_sink_;
	if (hw.empty() || isAppVirtualSink(hw))
		hw = getDefaultSinkName();
	if (hw.empty() || isAppVirtualSink(hw))
		hw = hw_sink_name_;

	if (!hw.empty() && !isAppVirtualSink(hw))
	{
		copySinkVolume(kCoralSink, hw);
		std::string cmd = "pactl set-default-sink " + shellQuote(hw) + " 2>/dev/null";
		system(cmd.c_str());
	}

	unloadCoralModules();
}

void AudioPassthruPrivate::onDeviceChange()
{
}

