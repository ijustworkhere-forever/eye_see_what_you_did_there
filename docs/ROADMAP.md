# Roadmap

## v0.1 — Firmware bootstrap (this milestone)
- PlatformIO project structure, `esp32dev` + `native` test environments
- MotionHardware (PCA9685) driver
- EyeController, CalibrationManager, Configuration
- Animation/Behavior interfaces and skeletons
- CI (build + native tests + formatting)

## v0.2 — Real motion
- `EyeController::toServoOutput()` calibration math (scaling, invert, mirror, offset)
- `IAnimationEngine` real interpolation: linear, ease-in/out, cubic
- `GazeTarget` speed/hold honored

## v0.3 — Behavior
- `EyeState`-driven behavior switching in `BehaviorEngine`
- `TrackingBehavior`, `CuriousBehavior`, `RandomBehavior`, `SleepBehavior`
- Idle scanning / micro-saccades
- Animated blink (close + reopen), expression pose blending

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
