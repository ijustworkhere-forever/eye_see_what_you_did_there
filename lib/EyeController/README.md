# EyeController

**Purpose:** The sole class allowed to convert eye "intent" into servo
positions. No other module ever touches `IServoOutput` directly.

**Responsibilities:** Hold the current `EyePose`; convert poses to
`ServoOutput` via `CalibrationManager`; expose named convenience methods
(`look`, `blink`, `winkLeft`, `winkRight`, `sleep`, `wake`, `setExpression`,
`setIdle`).

**Planned features:** None — this class is deliberately "dumb" (see
docs/architecture.md invariant 1). All animation and behavior sit above it.

**Future work:** Real `toServoOutput()` calibration math (pulse scaling,
invert, mirror, mechanical offset) — docs/ROADMAP.md v0.2.
