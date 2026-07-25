#pragma once

namespace eyesee {

/** Named facial expressions. Pose blending per expression is future work (docs/ROADMAP.md v0.3). */
enum class Expression {
    Neutral,
    Happy,
    Curious,
    Sleepy,
    Angry,
    Surprised
};

/** Command-layer gaze intent: where to look, and how (speed/blink/hold) — consumed by
 * IAnimationEngine, not EyeController. */
struct GazeTarget {
    float x = 0.0f;
    float y = 0.0f;
    float speed = 1.0f;
    bool blinkOnArrival = false;
    bool hold = false;
};

/**
 * The complete instantaneous "desired shape" of both eyes — no timing, no
 * easing, just values. IAnimationEngine produces one of these per frame;
 * EyeController converts it to a ServoOutput.
 */
struct EyePose {
    float lookX = 0.0f;         // normalized -1..1
    float lookY = 0.0f;         // normalized -1..1
    float upperLeftLid = 1.0f;  // normalized 0 (closed) .. 1 (open)
    float lowerLeftLid = 1.0f;
    float upperRightLid = 1.0f;
    float lowerRightLid = 1.0f;
};

}  // namespace eyesee
