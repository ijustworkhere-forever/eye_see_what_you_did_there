# Animation

**Purpose:** Own the "how" of eye motion — timing and easing — so
`EyeController` never has to.

**Responsibilities:** `IAnimationEngine` converts `GazeTarget`/`Expression` +
duration into `EyePose` updates over time.

**Planned features:** Linear, ease-in/out, cubic, and eventually spline
interpolation (docs/ROADMAP.md v0.2); animated blink/wink with real
open/close timing (v0.3).

**Future work:** Micro-saccades, idle scanning motion, expression blending.
