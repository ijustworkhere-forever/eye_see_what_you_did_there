# Architecture

## Frame update model

The firmware is frame-based, target update frequency **100 Hz**. Every
stateful module implements `update(uint32_t deltaMs)`. `main.cpp`'s `loop()`
contains no business logic — it measures elapsed time and calls `update(dt)`
down the chain:

```
loop() -> Clock -> BehaviorEngine.update(dt) -> IAnimationEngine.update(dt)
       -> EyeController.update(dt) -> IServoOutput.update(dt)
```

## Control-flow invariant

No code manipulates servos directly. All motion goes through
`EyeController`. All inputs are arbitrated by `BehaviorEngine` through a
single `CommandQueue` — no input source is allowed to fight another:

```
Web UI / REST API / WebSocket / Bluetooth / Serial
                |  (push EyeCommand)
              CommandQueue
                |  (drained each frame)
           BehaviorEngine  -- decides WHAT           (EyeCommand)
                |
          IAnimationEngine -- decides HOW            (GazeTarget in, EyePose out)
                |
            EyeController  -- decides WHERE          (EyePose in, ServoOutput out)
                |
            IServoOutput   -- moves hardware         (ServoOutput in)
                |
              PCA9685 (or other future actuator hardware)
```

`GazeTarget` (position + speed + blinkOnArrival + hold) is the vocabulary of
the command layer. `IAnimationEngine` converts that into a plain `EyePose`
each frame; `EyeController` converts `EyePose` into a calibrated
`ServoOutput`. Each stage does exactly one kind of conversion.

## Three invariants

1. **`EyeController` never owns time.** No delays, timers, easing, or
   animation — it holds only its current `EyePose` and converts it to a
   `ServoOutput` synchronously. All animation lives in `IAnimationEngine`.
2. **`MotionHardware` never owns state.** `IServoOutput` receives a
   `ServoOutput`, writes PWM, done.
3. **`Behavior` never knows hardware.** `BehaviorEngine`/`IBehavior` only
   deal in `EyeCommand`/`GazeTarget`/`Expression` — never servo channels,
   pulse widths, calibration, or inversion.

A side effect of invariant 2: swapping `IServoOutput` for a host-side
implementation (e.g. an SDL window drawing the eyes) is a one-class change
with zero impact on `EyeController`/`Animation`/`Behavior`.

## Module dependency graph

Two independent leaf modules sit at the bottom. `Configuration` and
`MotionHardware` know nothing about each other — `IServoOutput`/`ServoOutput`
only include `<cstdint>`:

```
Configuration                          MotionHardware
(leaf: ServoConfig, EyeConfig,         (leaf: ServoOutput, IServoOutput,
 NetworkConfig, BehaviorConfig,         Pca9685ServoOutput)
 CalibrationManager)
         ^                                     ^
         |                                     |
         +------------------+------------------+
                            |
                      EyeController ---> depends on Configuration + MotionHardware
                            ^             (also owns EyeTypes.h: Expression,
                            |              GazeTarget, EyePose)
                       Animation ------> depends on EyeController
                            ^
                            |
                        Behavior ------> depends on Animation + EyeTypes.h
                                          (GazeTarget/Expression only — never
                                           EyeController itself, so hardware and
                                           calibration types stay out of scope)

Logger, Storage, Networking, OTA: leaf modules, no dependency on the above.
```

## Namespace

All firmware code lives under `namespace eyesee`.

## REST API versioning

All REST routes (not implemented yet) are versioned under `/api/v1/`:
`GET /api/v1/status`, `POST /api/v1/look`, `POST /api/v1/blink`,
`POST /api/v1/expression`, `POST /api/v1/config`, `GET /api/v1/config`.

## Testing strategy

`Configuration`, `CalibrationManager`, `EyeController`, `Animation`, and
`Behavior` are free of `Arduino.h` and are unit-tested on the host via the
`native` PlatformIO environment (`pio test -e native`). `Logger`,
`MotionHardware` (specifically `Pca9685ServoOutput`), and `Storage`
(`PreferencesStore`) depend on Arduino/hardware headers and are verified only
by `pio run -e esp32dev` compiling successfully. `Networking` and `OTA` are
Arduino-free today (they only include `<cstdint>`) — they will gain hardware
dependencies once their bodies are implemented.
