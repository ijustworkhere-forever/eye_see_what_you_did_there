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
`EyeCommand` to `IAnimationEngine`, then delegates per-frame `update` to the
active `IBehavior` (only `IdleBehaviorStub` exists this pass).
