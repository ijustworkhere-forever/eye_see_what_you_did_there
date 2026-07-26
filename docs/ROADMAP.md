# Roadmap

## v0.1 — Firmware bootstrap (complete)
- PlatformIO project structure, `esp32dev` + `native` test environments
- MotionHardware (PCA9685) driver
- EyeController, CalibrationManager, Configuration
- Animation/Behavior interfaces and skeletons
- CI (build + native tests + formatting)

## v0.2 — Real motion (complete)
- `EyeController::toServoOutput()` calibration math (scaling, invert, mirror, offset)
- `IAnimationEngine` real interpolation: linear, ease-in/out, cubic
- `GazeTarget` speed honored (degrees/second, using `lookRangeDegrees`); `hold` remains unused, deferred to v0.6 (face-tracking bridge)
- Frame-rate pacing in `loop()`: holds the documented 100Hz target via a fixed-period gate

## v0.3 — Behavior (complete)
- `EyeState`-driven behavior switching in `BehaviorEngine` (registration table + onEnter/onExit lifecycle)
- `IdleBehavior` (registered), `SleepBehavior` (registered), `TrackingBehavior` (registered) — `CuriousBehavior`/`RandomBehavior` built and tested as alternative idle personalities, not yet registered (a future config/REST choice)
- Idle scanning / micro-saccades via `IdleBehavior` + injectable `IRandomSource`
- Animated blink (close + reopen, symmetric duration); wink/sleep remain one-way
- Expression pose blending via a shared `expressionEyelidTarget()` lookup table

For reference: [Will Cogley's EYEMECH ε3.2 adapted for ESP32 + PCA9685](https://github.com/GerNavBet/Will-cogley-s-EYEMECH-3.2-control-code-adapted-for-ESP32-with-PCA9685-servo-controller) is a MicroPython implementation with the same PCA9685 channel layout (0=LR, 1=UD, 2=TL, 3=BL, 4=TR, 5=BR) and a similar auto-mode idle/blink behavior model — useful prior art for the v0.3 behavior work.

## v0.4 — Connectivity (complete)
- `RestApi`: `/api/v1/*` endpoints wired to `CommandQueue` — `status`, `look`, `blink`, `wink`, `expression`, `sleep`, `wake` (`GET`/`POST /api/v1/config` deferred to v0.5)
- `WebServer`: serves `data/` control panel assets from LittleFS
- `WebSocketServer`: ~30Hz state broadcast at `/ws`
- `WifiManager`: STA connect with retry backoff; credentials via gitignored `include/WifiCredentials.h`
- Web UI: joystick, blink/wink/sleep/wake, expressions, live diagnostics
- New native-testable `lib/Protocol` module: JSON request parsing and response building, zero Arduino dependency

## v0.5 — Persistence & OTA (complete)
- `CalibrationManager` <-> `PreferencesStore` persistence (`loadFromStorage`/`saveToStorage`; `saveToStorage` writes all 37 keys unconditionally on every config change -- NVS itself skips no-op writes to unchanged keys, so in practice only the ~6 keys for the one changed channel actually hit flash)
- `OtaManager`: real `ArduinoOTA` wiring — reflash over WiFi, no USB required
- Config REST endpoints: `GET`/`POST /api/v1/config` (one channel per POST; `RestApi`'s one carve-out to mutate state outside `CommandQueue`, since calibration isn't a live-motion command)
- `clang-tidy` in CI (informational this pass, not a hard gate — first-ever lint pass on this codebase)

## v0.6 — Integrations (complete)
- Face-tracking bridge: `POST /api/v1/track`, a new `CommandType::Track`, and a `TrackingBehavior` stale-timeout redesign (recenters if a tracked face goes unseen for 3s without `hold: true`)
- MQTT: `lib/Integrations/MqttBridge` subscribes to `<prefix>/command` (reusing every existing `lib/Protocol` parser via one new type-dispatch function) and publishes `<prefix>/status` at 1Hz
- Bluetooth gamepad control: `lib/Integrations/GamepadBridge` on `tbekas/BLE-Gamepad-Client` (BLE HID host, Xbox Wireless Controller), mapped via a pure, native-tested `GamepadInputMapper`

## v1.0 — Stable release (complete)
- API freeze: `/api/v1/`, the WebSocket protocol, and MQTT are all now
  additive-only-changes-under-the-same-version (see `docs/api-reference.md`'s
  "Versioning & stability" section); every status response gained a
  `firmwareVersion` field
- Full documentation: `docs/api-reference.md` (full REST/WS/MQTT protocol
  reference), `docs/hardware.md` (wiring/BOM/power), `docs/known-limitations.md`
  (consolidated future-work notes), root `README.md` rewritten as a
  getting-started guide (hardware, flashing, calibration, control examples)
- Example configs: `examples/` populated with calibration, REST, and MQTT
  examples plus a disposable local MQTT broker compose file
