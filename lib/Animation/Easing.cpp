#include "Easing.h"

#include <cmath>

namespace eyesee {

namespace {
float clamp01(float t) {
    if (t < 0.0f) return 0.0f;
    if (t > 1.0f) return 1.0f;
    return t;
}
}  // namespace

float ease(EasingType type, float t) {
    t = clamp01(t);
    switch (type) {
        case EasingType::Linear:
            return t;
        case EasingType::EaseInOut:
            return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
        case EasingType::Cubic:
            return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }
    return t;
}

}  // namespace eyesee
