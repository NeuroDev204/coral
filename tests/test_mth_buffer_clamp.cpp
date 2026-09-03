#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>

#include "codedefs.h"
#include "mth.h"

int main() {
    std::cout << "Running mthConvertRealtypeBufToIntBuf test..." << std::endl;
    std::vector<float> input = {
        0.0f, 0.5f, 0.999f, 1.0f, 1.05f, 1.5f, 2.0f,
        -0.5f, -0.999f, -1.0f, -1.05f, -1.5f, -2.0f
    };
    std::vector<short> output(input.size(), 0);

    int res = mthConvertRealtypeBufToIntBuf((int)input.size(), 16, 16, input.data(), output.data());
    assert(res == 0); // OKAY

    // Verify normal linear range mapping
    assert(output[0] == 0);
    assert(output[1] == 16384);
    assert(output[7] == -16384);

    // Check that positive peaks DO NOT wrap to negative numbers
    for (size_t i = 0; i < input.size(); ++i) {
        float in = input[i];
        short out = output[i];
        std::cout << "in=" << in << " -> out=" << out << std::endl;
        if (in >= 1.0f) {
            assert(out == 32767); // Must clamp to SHRT_MAX, never negative!
        } else if (in <= -1.0f) {
            assert(out == -32768); // Must clamp to SHRT_MIN, never positive!
        }
    }

    std::cout << "Running mthConvertRealtypeBufToIntBufSurroundPostProcess test..." << std::endl;
    // 6 channels per frame: length must be multiple of 6
    std::vector<float> surround_in = {
        1.5f, -1.5f,  // Front L, R
        2.0f, -2.0f,  // Rear L, R
        1.0f, -1.0f   // Center, Sub
    };
    std::vector<short> surround_out(6, 0);
    int res_surround = mthConvertRealtypeBufToIntBufSurroundPostProcess(6, 16, 16, surround_in.data(), surround_out.data());
    assert(res_surround == 0);

    for (size_t i = 0; i < surround_out.size(); ++i) {
        std::cout << "surround i=" << i << " -> out=" << surround_out[i] << std::endl;
        if (i % 2 == 0) {
            assert(surround_out[i] == 32767);
        } else {
            assert(surround_out[i] == -32768);
        }
    }

    std::cout << "All MthBuffer clamping tests PASSED!" << std::endl;
    return 0;
}
