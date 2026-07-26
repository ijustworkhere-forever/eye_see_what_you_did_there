# Roadmap

## v0.1 — Firmware bootstrap (this milestone)
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

## v0.4 — Connectivity
- `RestApi`: `/api/v1/*` endpoints wired to `CommandQueue`
- `WebServer`: serves `data/` control panel assets
- `WebSocketServer`: 30-60Hz state broadcast
- Web UI: joystick, blink, expressions, diagnostics

## v0.5 — Persistence & OTA
- `CalibrationManager` <-> `PreferencesStore` persistence
- `OtaManager`: `ArduinoOTA` wiring
- Config REST endpoints (`GET`/`POST /api/v1/config`)
- `clang-tidy` in CI

## v0.6 — Integrations
- Face-tracking bridge
- MQTT
- Bluetooth gamepad control

## v1.0 — Stable release
- API freeze
- Full documentation
- Example configs
