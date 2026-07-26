# Changelog

## v1.0 — Stable release
API freeze (`/api/v1/`, the WebSocket protocol, and MQTT are now
additive-only-changes-under-the-same-version; every status response gained
a `firmwareVersion` field), full documentation (`docs/api-reference.md`,
`docs/hardware.md`, `docs/known-limitations.md`, a rewritten
getting-started `README.md`), and example configs (`examples/`:
calibration, REST, and MQTT examples plus a disposable local MQTT broker
compose file).

## v0.6 — Integrations
Face-tracking bridge (`POST /api/v1/track`, a new `CommandType::Track`,
and a `TrackingBehavior` stale-timeout redesign that recenters if a
tracked face goes unseen for 3s without `hold: true`); MQTT
(`lib/Integrations/MqttBridge`, subscribing to `<prefix>/command` and
publishing `<prefix>/status` at 1Hz); and Bluetooth gamepad control
(`lib/Integrations/GamepadBridge` on an Xbox Wireless Controller, mapped
via `GamepadInputMapper`).

## v0.5 — Persistence & OTA
`CalibrationManager` <-> `PreferencesStore` persistence
(`loadFromStorage`/`saveToStorage`); config REST endpoints
(`GET`/`POST /api/v1/config`, one channel per POST — `RestApi`'s one
carve-out to mutate state outside `CommandQueue`, since calibration isn't
a live-motion command); over-the-air firmware updates (`OtaManager` on
real `ArduinoOTA` wiring, no USB required); and `clang-tidy` added to CI
(informational this pass — the first-ever lint pass on this codebase).

## v0.4 — Connectivity
`RestApi`'s `/api/v1/*` endpoints wired to `CommandQueue` (`status`,
`look`, `blink`, `wink`, `expression`, `sleep`, `wake`); `WebServer`
serving `data/` control panel assets from LittleFS; `WebSocketServer`'s
~30Hz state broadcast at `/ws`; `WifiManager` STA connect with retry
backoff; a Web UI (joystick, blink/wink/sleep/wake, expressions, live
diagnostics); and the new native-testable `lib/Protocol` module for JSON
request parsing and response building, with zero Arduino dependency.

## v0.3 — Behavior
`BehaviorEngine`'s `EyeState`-driven behavior switching (registration
table + onEnter/onExit lifecycle) with `IdleBehavior`, `SleepBehavior`,
and `TrackingBehavior` registered (`CuriousBehavior`/`RandomBehavior`
built and tested as alternative idle personalities, not yet registered);
idle scanning / micro-saccades via `IdleBehavior` + an injectable
`IRandomSource`; animated (close + reopen) blinking; and expression pose
blending via a shared `expressionEyelidTarget()` lookup table.

## v0.2 — Real motion
`EyeController::toServoOutput()` calibration math (scaling, invert,
mirror, offset); real `IAnimationEngine` interpolation (linear,
ease-in/out, cubic); `GazeTarget` speed honored in degrees/second (using
`lookRangeDegrees`); and frame-rate pacing in `loop()` that holds the
documented 100Hz target via a fixed-period gate.

## v0.1 — Firmware bootstrap
PlatformIO project structure (`esp32dev` + `native` test environments);
the `MotionHardware` (PCA9685) driver; `EyeController`,
`CalibrationManager`, and `Configuration`; animation/behavior interface
skeletons; and CI (build + native tests + formatting).
