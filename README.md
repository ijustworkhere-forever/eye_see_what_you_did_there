# EyeSee

A modular ESP32 + PCA9685 firmware platform for animatronic eyes, inspired
by the behavior of Will Cogley's EyeMech project but built from scratch as
a long-lived firmware platform (in the spirit of Marlin / ESPHome / WLED),
not an Arduino sketch.

## Hardware

- ESP32 DevKit
- PCA9685 servo driver (Adafruit PWM Servo Driver library)
- 6x SG90/MG90S servos: LR (left/right eye), UD (up/down eye), TL/BL (left
  eyelids), TR/BR (right eyelids)

## Architecture

No code manipulates servos directly. All motion goes through `EyeController`;
all inputs are arbitrated by `BehaviorEngine` through a single command queue:

```
Web UI / REST API / WebSocket / Bluetooth / Serial
                ↓ (EyeCommand)
              CommandQueue
                ↓
           BehaviorEngine  (WHAT: "look over there")
                ↓ (GazeTarget)
          IAnimationEngine (HOW: timing/easing)
                ↓ (EyePose)
            EyeController  (WHERE: pose -> servo pulses)
                ↓ (ServoOutput)
            IServoOutput   (hardware)
                ↓
              PCA9685
```

See `docs/architecture.md` for the full design, including the three core
invariants (EyeController owns no time; MotionHardware owns no state;
Behavior never knows hardware) and `docs/ROADMAP.md` for what's planned.

## Building

Requires [PlatformIO](https://platformio.org/).

```bash
pio run -e esp32dev      # build firmware
pio run -e esp32dev -t upload  # flash to a connected ESP32
pio test -e native        # run hardware-independent unit tests
```

## Module layout

| Module | Responsibility |
|---|---|
| `lib/EyeController` | Sole owner of eye motion state; converts poses to servo pulses |
| `lib/MotionHardware` | Abstracts the PCA9685 (or future actuator hardware) |
| `lib/Animation` | Timing/easing between behavior intent and eye pose |
| `lib/Behavior` | Command arbitration, system state, pluggable behaviors |
| `lib/Protocol` | Native-testable JSON encode/decode for the Networking layer |
| `lib/Networking` | Web UI, REST API, WebSocket |
| `lib/Storage` | Flash-backed key/value persistence |
| `lib/OTA` | Over-the-air updates (placeholder) |
| `lib/Logger` | Serial logging |
| `lib/Configuration` | Typed config structs + `CalibrationManager` |

Each module folder has its own `README.md` with more detail.

## Contributing

This is a young, actively-scaffolded project — see `docs/ROADMAP.md` for
what's planned and `docs/architecture.md` for the design rules new code
should follow. Issues and PRs welcome.

## License

CC0 1.0 Universal — see `LICENSE`.
