# Configuration

**Purpose:** Strongly-typed, dependency-free data structs shared across the
firmware.

**Responsibilities:** Define `ServoConfig`, `EyeConfig`, `NetworkConfig`,
`BehaviorConfig` with sane `constexpr` defaults. Also owns `CalibrationManager`
(see below), since calibration is just typed access to `EyeConfig`.

**CalibrationManager:** In-memory owner of `EyeConfig`, with explicit
`loadFromStorage(IStorage&)`/`saveToStorage(IStorage&) const` persistence
(v0.5) — not constructor-injected, so existing tests and call sites stay
unchanged whether or not persistence is used. `EyeChannel` selects which of
the six servos a getter/setter operates on. Thread-safe on ESP32 (see the
class's own doc comment in `CalibrationManager.h`) since `RestApi`'s config
route can mutate it from a different task than the one that reads it every
frame.

**Future work:** No REST-exposed way to change `EyeConfig::lookRangeDegrees`
at runtime yet — it persists correctly but only `CalibrationManager`'s
explicit-`EyeConfig` constructor can set it today.
