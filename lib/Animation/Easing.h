#pragma once

namespace eyesee {

/** Interpolation curve shape for an in-progress animation transition. */
enum class EasingType { Linear, EaseInOut, Cubic };

/**
 * Maps a linear progress fraction through the given easing curve. `t` is
 * clamped to [0, 1] before easing, so out-of-range input can't produce an
 * out-of-range result.
 */
float ease(EasingType type, float t);

}  // namespace eyesee
