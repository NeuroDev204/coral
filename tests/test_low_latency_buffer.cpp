#include <cassert>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <string>

// Test timing and byte calculations for low-latency audio passthru
constexpr uint32_t kSampleRate = 48000;
constexpr uint32_t kChannels = 2;
constexpr uint32_t kBytesPerSample = sizeof(float); // 4 bytes
constexpr uint32_t kNumSamples = 256;

inline size_t samplesToBytes(uint32_t samples) {
    return samples * kChannels * kBytesPerSample;
}

inline uint32_t usecToBytes(uint32_t usec) {
    return static_cast<uint32_t>((static_cast<uint64_t>(usec) * kSampleRate * kChannels * kBytesPerSample) / 1000000ULL);
}

inline bool shouldTriggerDriftFlush(const std::string& sink_name, uint64_t latency_usec, uint64_t current_iter, uint64_t last_flush_iter) {
    uint64_t threshold = (sink_name.find("bluez") != std::string::npos) ? 140000ULL : 50000ULL;
    bool cooldown_ok = (current_iter >= last_flush_iter + 600); // ~3.2 seconds cooldown at 256 samples/iter
    return (latency_usec > threshold) && cooldown_ok;
}

int main() {
    // 1. Verify buffer sizes
    size_t chunk_bytes_256 = samplesToBytes(256);
    assert(chunk_bytes_256 == 2048); // 256 * 2 * 4 = 2048 bytes
    size_t chunk_bytes_512 = samplesToBytes(512);
    assert(chunk_bytes_512 == 4096); // 512 * 2 * 4 = 4096 bytes

    // 2. Verify usec conversions
    // 5333 usec (~5.33ms) -> ~256 samples
    uint32_t frag_bytes_256 = usecToBytes(5333);
    uint32_t frag_samples_256 = frag_bytes_256 / (kChannels * kBytesPerSample);
    assert(frag_samples_256 >= 255 && frag_samples_256 <= 256);

    // 10666 usec (~10.67ms) -> ~512 samples
    uint32_t frag_bytes_512 = usecToBytes(10666);
    uint32_t frag_samples_512 = frag_bytes_512 / (kChannels * kBytesPerSample);
    assert(frag_samples_512 >= 511 && frag_samples_512 <= 512);

    // 21333 usec (~21.33ms) -> ~1024 samples
    uint32_t tlength_bytes_1024 = usecToBytes(21333);
    uint32_t tlength_samples_1024 = tlength_bytes_1024 / (kChannels * kBytesPerSample);
    assert(tlength_samples_1024 >= 1023 && tlength_samples_1024 <= 1024);

    // 60000 usec (~60ms) max buffer ceiling
    uint32_t max_bytes = usecToBytes(60000);
    assert(max_bytes == 23040); // 60ms * 48 samples/ms * 8 bytes = 23040 bytes

    // 3. Verify drift threshold logic for Bluetooth
    std::string bt_sink = "bluez_output.DC:E9:E0:97:BE:8C";
    assert(!shouldTriggerDriftFlush(bt_sink, 120000, 1000, 0)); // Normal BT latency (120ms < 140ms) -> No flush
    assert(shouldTriggerDriftFlush(bt_sink, 160000, 1000, 0));  // Excessive BT latency (160ms > 140ms, cooldown ok) -> Flush
    assert(!shouldTriggerDriftFlush(bt_sink, 160000, 1200, 1000)); // Excessive but within cooldown (200 < 600) -> No flush

    // 4. Verify drift threshold logic for ALSA
    std::string alsa_sink = "alsa_output.pci-0000_00_1f.3-platform-skl_hda_dsp_generic.HiFi__Speaker__sink";
    assert(!shouldTriggerDriftFlush(alsa_sink, 35000, 1000, 0)); // Normal ALSA latency (35ms < 50ms) -> No flush
    assert(shouldTriggerDriftFlush(alsa_sink, 65000, 1000, 0));  // Excessive ALSA latency (65ms > 50ms) -> Flush

    std::cout << "All low-latency buffer & drift guard tests PASSED!" << std::endl;
    return 0;
}
