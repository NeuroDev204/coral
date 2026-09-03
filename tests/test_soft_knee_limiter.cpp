#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

inline float applySoftKnee(float s) {
    float abs_s = std::fabs(s);
    if (abs_s <= 0.90f) {
        return s; // 100% bit-perfect linear
    }
    float sign = (s > 0.0f) ? 1.0f : -1.0f;
    if (abs_s < 1.098f) {
        float u = (abs_s - 0.90f) / 0.198f;
        float compressed = 0.90f + 0.198f * (u - 0.5f * u * u);
        return sign * compressed;
    } else {
        return sign * 0.999f;
    }
}

int main() {
    // 1. Linear region check
    assert(applySoftKnee(0.0f) == 0.0f);
    assert(applySoftKnee(0.5f) == 0.5f);
    assert(applySoftKnee(0.90f) == 0.90f);
    assert(applySoftKnee(-0.90f) == -0.90f);

    // 2. Over-amplitude compression check
    for (float in = 0.91f; in <= 3.0f; in += 0.01f) {
        float out = applySoftKnee(in);
        assert(out > 0.90f);
        assert(out <= 1.0f); // Never exceeds 1.0f!
    }
    for (float in = -0.91f; in >= -3.0f; in -= 0.01f) {
        float out = applySoftKnee(in);
        assert(out < -0.90f);
        assert(out >= -1.0f); // Never exceeds -1.0f!
    }

    // 3. Monotonicity check
    float prev = 0.0f;
    for (float in = 0.0f; in <= 3.0f; in += 0.005f) {
        float curr = applySoftKnee(in);
        assert(curr >= prev);
        prev = curr;
    }

    // 4. Extreme values and boundary checks
    assert(std::fabs(applySoftKnee(1.098f) - 0.999f) < 1e-4f);
    assert(std::fabs(applySoftKnee(-1.098f) - (-0.999f)) < 1e-4f);
    assert(applySoftKnee(10.0f) == 0.999f);
    assert(applySoftKnee(-10.0f) == -0.999f);
    assert(applySoftKnee(100.0f) == 0.999f);
    assert(applySoftKnee(-100.0f) == -0.999f);

    // 5. C1 continuity checks at knee (0.90f) and ceiling (1.098f)
    const float eps = 1e-4f;
    // Knee (0.90f): value continuity and slope continuity
    float v_below_knee = applySoftKnee(0.90f - eps);
    float v_at_knee = applySoftKnee(0.90f);
    float v_above_knee = applySoftKnee(0.90f + eps);
    assert(std::fabs(v_above_knee - v_below_knee) < 3.0f * eps);
    float slope_knee_left = (v_at_knee - v_below_knee) / eps;
    float slope_knee_right = (v_above_knee - v_at_knee) / eps;
    assert(std::fabs(slope_knee_left - 1.0f) < 2e-3f);
    assert(std::fabs(slope_knee_right - 1.0f) < 2e-3f);

    // Ceiling (1.098f): value continuity and slope continuity (reaches 0.0)
    float v_below_ceil = applySoftKnee(1.098f - eps);
    float v_at_ceil = applySoftKnee(1.098f);
    float v_above_ceil = applySoftKnee(1.098f + eps);
    assert(std::fabs(v_above_ceil - v_below_ceil) < 3.0f * eps);
    float slope_ceil_left = (v_at_ceil - v_below_ceil) / eps;
    float slope_ceil_right = (v_above_ceil - v_at_ceil) / eps;
    assert(std::fabs(slope_ceil_left) < 2e-3f);
    assert(std::fabs(slope_ceil_right) < 2e-3f);

    std::cout << "All C1-continuous soft-knee limiter tests PASSED!" << std::endl;
    return 0;
}
