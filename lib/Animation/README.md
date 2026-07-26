# Animation

**Purpose:** Own the "how" of eye motion — timing and easing — so
`EyeController` never has to.

**Responsibilities:** `IAnimationEngine` converts `GazeTarget`/`Expression` +
duration into `EyePose` updates over time. `RealAnimationEngine` implements
it with two independent, composed transitions (gaze, eyelids), each eased
over its own duration — see `Easing.h` for the three curves and
`docs/superpowers/specs/2026-07-25-v0.2-real-motion-design.md` for the full
design. `animateExpression()` blends toward `ExpressionPose.h`'s
per-expression eyelid targets via the same eyelid-transition machinery as
any other eyelid move (EaseInOut easing), rather than snapping instantly.

**Planned features:** Micro-saccades and idle scanning motion generation
live in `Behavior` (docs/ROADMAP.md v0.3), not here — this module only
executes whatever gaze/eyelid targets it's given.

**Future work:** `animateWinkLeft()`/`animateWinkRight()`/`animateSleep()`
still only close — they never auto-reopen (only `animateBlink()` does,
symmetrically, over the same duration as the close phase). Caller-selectable
easing per call (currently fixed per intent — gaze uses Cubic, eyelid moves
use EaseInOut).
