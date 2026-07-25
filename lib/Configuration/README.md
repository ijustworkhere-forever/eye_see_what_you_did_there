# Configuration

**Purpose:** Strongly-typed, dependency-free data structs shared across the
firmware.

**Responsibilities:** Define `ServoConfig`, `EyeConfig`, `NetworkConfig`,
`BehaviorConfig` with sane `constexpr` defaults. Also owns `CalibrationManager`
(see below), since calibration is just typed access to `EyeConfig`.

**Planned features:** None — this module is data, not behavior.

**Future work:** `CalibrationManager` persistence via `IStorage` (see
`docs/ROADMAP.md` v0.5).

**CalibrationManager:** In-memory owner of `EyeConfig`. `EyeChannel` selects
which of the six servos a getter/setter operates on.
