# Behavior

**Purpose:** The single arbiter between the outside world and the eye. Every
input source pushes an `EyeCommand`; nothing else calls `IAnimationEngine`
or `EyeController` directly, so inputs never fight each other.

**Responsibilities (this file):** `EyeState` (system-level state),
`EyeCommand`/`CommandQueue` (fixed-capacity FIFO command buffer).

**Future work:** Command priority handling (currently stored but not acted
on — `CommandQueue` is strictly FIFO). Configurable/swappable idle
personality (choosing between `IdleBehavior`/`CuriousBehavior`/
`RandomBehavior` at runtime, e.g. via REST or persisted config) is v0.4+
work — see docs/ROADMAP.md.

**IBehaviorEngine / BehaviorEngine:** Drains `CommandQueue`, dispatches each
`EyeCommand` to `IAnimationEngine`, then delegates per-frame `update` to
whichever `IBehavior` is registered for the current `EyeState` (via
`registerBehavior()`), calling `onExit`/`onEnter` on transition.
`IdleBehavior`, `SleepBehavior`, and `TrackingBehavior` are registered in
`main.cpp`; `CuriousBehavior`/`RandomBehavior` are built and tested as
alternative idle personalities but not registered yet — see
docs/superpowers/specs/2026-07-25-v0.3-behavior-design.md.
