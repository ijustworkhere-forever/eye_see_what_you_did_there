# EyeController

**Purpose:** The sole class allowed to convert eye "intent" into servo
positions. No other module ever touches `IServoOutput` directly.

**Responsibilities:** Hold the current `EyePose`; convert poses to
`ServoOutput` via `CalibrationManager`; expose named convenience methods
(`look`, `blink`, `winkLeft`, `winkRight`, `sleep`, `wake`, `setExpression`,
`setIdle`).

**Planned features:** None — this class is deliberately "dumb" (see
docs/architecture.md invariant 1). All animation and behavior sit above it.

**Future work:** A calibration validation/bounds-check API, so `ServoConfig`
values arriving from outside the firmware (v0.4 REST endpoints, v0.5 persisted
config) are rejected before they reach the servos — docs/ROADMAP.md.
