#pragma once

#include "GlanceBehavior.h"

namespace eyesee {

/**
 * An alternative idle personality: wider, more deliberate glances than
 * IdleBehavior's micro-saccades, paired with a Curious expression. Built
 * and tested this pass but not registered in main.cpp's EyeState::Idle
 * slot yet (docs/superpowers/specs/2026-07-25-v0.3-behavior-design.md) —
 * a future config/REST choice, not a new EyeState.
 */
class CuriousBehavior : public GlanceBehavior {
public:
    explicit CuriousBehavior(IRandomSource& randomSource);

    void onEnter(IAnimationEngine& animation) override;
};

}  // namespace eyesee
