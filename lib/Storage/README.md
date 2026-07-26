# Storage

**Purpose:** Swappable persistence backend behind `IStorage`.

**Responsibilities:** `PreferencesStore` wraps the ESP32 `Preferences` API
(flash-backed key/value store) for `uint16_t`, `int16_t`, `bool`, and
`float` values — everything `CalibrationManager`'s `EyeConfig` needs.

**Consumed by:** `CalibrationManager::loadFromStorage`/`saveToStorage`
(`lib/Configuration`), which persists every servo's calibration plus
`lookRangeDegrees` — see
`docs/superpowers/specs/2026-07-26-v0.5-persistence-ota-design.md` for the
exact key table (every key ≤15 characters, ESP32 NVS's hard limit).
