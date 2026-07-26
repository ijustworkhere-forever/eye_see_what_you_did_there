# Behavior

**Purpose:** The single arbiter between the outside world and the eye. Every
input source pushes an `EyeCommand`; nothing else calls `IAnimationEngine`
or `EyeController` directly, so inputs never fight each other.

**Responsibilities (this file):** `EyeState` (system-level state),
`EyeCommand`/`CommandQueue` (fixed-capacity FIFO command buffer).

**Planned features:** `IBehavior`/`BehaviorEngine` — see below, added in the
next task.

**Future work:** Command priority handling (currently stored but not acted
on), `EyeState`-driven behavior switching (docs/ROADMAP.md v0.3).

**IBehaviorEngine / BehaviorEngine:** Drains `CommandQueue`, dispatches each
`EyeCommand` to `IAnimationEngine`, then delegates per-frame `update` to
whichever `IBehavior` is registered for the current `EyeState` (via
`registerBehavior()`), calling `onExit`/`onEnter` on transition.
`IdleBehavior`, `SleepBehavior`, and `TrackingBehavior` are registered in
`main.cpp`; `CuriousBehavior`/`RandomBehavior` are built and tested as
alternative idle personalities but not registered yet — see
docs/superpowers/specs/2026-07-25-v0.3-behavior-design.md.
