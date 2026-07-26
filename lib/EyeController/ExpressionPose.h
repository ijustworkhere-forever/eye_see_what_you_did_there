#pragma once

#include "EyeTypes.h"

namespace eyesee {

/** Symmetric left/right eyelid aperture for a named expression. */
struct ExpressionEyelidTarget {
    float upperLid;
    float lowerLid;
};

/**
 * Looks up the eyelid aperture that represents the given expression. Gaze
 * is never affected — see docs/superpowers/specs/2026-07-25-v0.3-behavior-design.md
 * for the full table and rationale (a deliberately simple, documented
 * design choice, not claimed to be anatomically precise).
 */
ExpressionEyelidTarget expressionEyelidTarget(Expression expression);

}  // namespace eyesee
