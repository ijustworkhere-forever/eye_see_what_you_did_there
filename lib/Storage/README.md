# Storage

**Purpose:** Swappable persistence backend behind `IStorage`.

**Responsibilities:** `PreferencesStore` wraps the ESP32 `Preferences` API
(flash-backed key/value store) for `uint16_t` values (pulse widths).

**Planned features:** None yet — not wired into `CalibrationManager`.

**Future work:** `CalibrationManager` persistence (save/load `EyeConfig` on
boot and on calibration change) — docs/ROADMAP.md v0.5.
