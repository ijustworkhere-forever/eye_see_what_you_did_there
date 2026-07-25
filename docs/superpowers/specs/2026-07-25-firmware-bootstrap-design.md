# EyeSee Firmware Bootstrap — Design

Date: 2026-07-25

## Purpose

Scaffold a modular ESP32 + PCA9685 firmware platform for animatronic eyes,
inspired by Will Cogley's EyeMech behavior but implemented as a new,
from-scratch codebase intended to grow into a long-lived open-source
firmware project (in the spirit of Marlin / ESPHome / WLED), not an Arduino
sketch.

This bootstrap creates the module tree, interfaces, and placeholder
implementations. **No advanced behavior is implemented in this pass** —
every subsystem beyond the servo/eye plumbing is a documented stub with
TODOs. The only functional requirement for this pass is: **it compiles**,
and the module boundaries are right for future contributors to build on.

## Hardware Target

- MCU: ESP32 DevKit
- Servo driver: PCA9685 (via Adafruit PWM Servo Driver library)
- Actuators: 6× SG90/MG90S servos — LR, UD, TL, BL, TR, BR
- Build system: PlatformIO, `framework = arduino`, `board = esp32dev`

## Architecture

### Control-flow invariant

No code manipulates servos directly. All motion goes through `EyeController`:

```
Web UI / REST API / Bluetooth / Serial
                ↓
         BehaviorEngine (autonomous behavior) OR direct manual command
                ↓
           EyeController
                ↓
          IServoDriver (Pca9685ServoDriver)
                ↓
              PCA9685
```

`BehaviorEngine` sits between input sources and `EyeController` for
autonomous behavior (idle scan, tracking, sleep/wake). Manual commands
(e.g. a REST `POST /look`) may call `EyeController` directly, bypassing
`BehaviorEngine` — but nothing is ever allowed to reach `IServoDriver`
except through `EyeController`.

### Namespace

All firmware code lives under `namespace eyesee`.

### Module layout

Each module is a flat PlatformIO private library under `lib/<Module>/`
(header + source at the library root — no nested `include/`/`src/` split,
which PlatformIO does not require for private libraries). Every module
folder has its own `README.md` (purpose, responsibilities, planned
features, future work).

```
include/            # project-wide public headers (currently empty/placeholder)
src/                 # main.cpp only — thin wiring, no logic
lib/
  EyeController/     # concrete: sole owner of eye motion state
  ServoDriver/        # IServoDriver + Pca9685ServoDriver
  Animation/           # IAnimationEngine + PassthroughAnimationEngine stub
  Behavior/            # IBehaviorEngine + BehaviorEngineSkeleton stub
  Networking/          # WebServer, RestApi, WebSocketServer placeholders
  Storage/             # IStorage + PreferencesStore (ESP32 Preferences wrapper)
  OTA/                 # OtaManager placeholder
  Logger/              # static Logger utility
  Configuration/       # EyeConfig, ServoConfig, NetworkConfig, BehaviorConfig structs
data/                # reserved for future SPIFFS/LittleFS web assets
docs/
  architecture.md    # this diagram + module responsibilities, long-form
  Doxyfile           # doxygen config, scans lib/ + src/, outputs docs/api/ (gitignored)
examples/            # reserved for future example sketches/configs
test/                # PlatformIO Unity tests
```

### Interfaces vs. concrete classes

Interfaces (pure abstract classes) are introduced **only** where a second
implementation is genuinely anticipated (YAGNI):

| Module | Interface? | Why |
|---|---|---|
| ServoDriver | `IServoDriver` → `Pca9685ServoDriver` | future mock driver for host-side tests, alternate driver chips |
| Animation | `IAnimationEngine` → `PassthroughAnimationEngine` (stub) | interpolation strategies will multiply (linear/easing/spline) |
| Behavior | `IBehaviorEngine` → `BehaviorEngineSkeleton` (stub) | behavior strategies (idle/tracking/emotion) will multiply |
| Storage | `IStorage` → `PreferencesStore` | swappable persistence backend, testability |
| EyeController | none (concrete) | single required implementation — this class *is* the abstraction boundary for motion |
| CalibrationManager | none (concrete) | single required implementation |
| Networking (WebServer/RestApi/WebSocketServer) | none (concrete, empty stub bodies) | only one implementation anticipated; each wraps a specific future library |
| OTA (OtaManager) | none (concrete, empty stub bodies) | single implementation |
| Logger | none (static utility) | cross-cutting concern; the one accepted exception to "avoid globals," documented inline |

Stub implementations (`PassthroughAnimationEngine`, `BehaviorEngineSkeleton`)
exist so `EyeController`/`main.cpp` wire together into a real, compiling
object graph rather than leaving dangling TODOs — but they contain no real
logic, only comments marking future work.

### EyeController

Concrete class, constructor-injected with `IServoDriver&` and
`CalibrationManager&` (dependency injection, no globals/singletons).
Public methods per spec, all present with real signatures, bodies stubbed:

`look(x, y)`, `blink()`, `winkLeft()`, `winkRight()`, `sleep()`, `wake()`,
`setExpression(Expression)`, `setIdle()`, `update()`.

### ServoDriver

`IServoDriver` interface: `moveServo(channel, angle)`, `setAngle(channel, angle)`,
`setPulse(channel, pulseUs)`, `update()`.
`Pca9685ServoDriver` implements it using the Adafruit PWM Servo Driver
library; constructor takes I2C address (default 0x40).

### CalibrationManager

Concrete class owning an in-memory `EyeConfig` (six `ServoConfig`s: limits,
neutral position, inversion, mirroring, offset). Getters/setters per
servo. Persistence is a stubbed call into `IStorage` (TODO — not
implemented this pass; will use ESP32 `Preferences` eventually).

### Configuration

Plain structs with `constexpr` defaults, no behavior:

- `ServoConfig` — channel, min/max pulse, neutral pulse, inverted (bool), offset
- `EyeConfig` — aggregates 6× `ServoConfig` (LR, UD, TL, BL, TR, BR) + look-range limits
- `NetworkConfig` — SSID/password placeholders, web server port
- `BehaviorConfig` — idle-timing placeholders

### Logger

Static-method utility class: `Logger::debug/info/warn/error(tag, message)`,
backed by `Serial`. `enum class LogLevel`. Not instance-injected — logging
is cross-cutting and threading a logger reference through every
constructor buys nothing; this is called out as an intentional exception
via a short inline comment.

### Networking / OTA / Storage

All placeholder-only this pass:

- `WebServer`, `RestApi`, `WebSocketServer` — classes with `begin()`/`update()`
  stubs, empty bodies, TODOs documenting the future REST endpoints
  (`GET /status`, `POST /look`, `POST /blink`, `POST /expression`,
  `POST /config`, `GET /config`) and target WebSocket update rate (30–60 Hz).
  No networking library dependency added yet — kept out of `platformio.ini`
  until real implementation begins.
- `OtaManager` — placeholder class, empty `begin()`/`update()`.
- `PreferencesStore` (`IStorage` impl) — wraps ESP32 `Preferences.h`
  (built into the Arduino core, no extra `lib_dep`), stub get/set methods.

### main.cpp

Thin wiring only — instantiates concrete objects, injects dependencies,
delegates to `update()` in `loop()`. No business logic in `main.cpp`.

```cpp
Pca9685ServoDriver servoDriver;
CalibrationManager calibration;
EyeController eyeController(servoDriver, calibration);
PassthroughAnimationEngine animationEngine;
BehaviorEngineSkeleton behaviorEngine(eyeController, animationEngine);

void setup() {
  Logger::init();
  servoDriver.init();
  eyeController.setIdle();
}

void loop() {
  behaviorEngine.update();
  eyeController.update();
}
```

## Build Configuration

`platformio.ini`:

- `[env:esp32dev]`
- `platform = espressif32`
- `board = esp32dev`
- `framework = arduino`
- `lib_deps = adafruit/Adafruit PWM Servo Driver Library`
- `build_flags = -std=gnu++17`
- `build_unflags = -std=gnu++11` (ESP32 Arduino core defaults to gnu++11)
- `monitor_speed = 115200`

No other third-party libraries are declared this pass — Networking/Storage/OTA
stubs don't touch a library yet, so nothing is added prematurely.

## Testing

One PlatformIO Unity test under `test/` (e.g. `test_calibration_defaults`)
asserting a trivial, real fact (default neutral servo position from
`CalibrationManager`) — proves `pio test` works end-to-end from the first
commit. No broader test coverage is expected yet since there's no real
logic to test.

## Documentation

- Root `README.md` rewritten: project overview, hardware, architecture
  diagram, build/flash instructions, module list, contributing pointer
  (replaces current 2-line stub).
- `docs/architecture.md`: expanded version of the architecture section
  above.
- `docs/Doxyfile`: scans `lib/` + `src/`, outputs to `docs/api/` (gitignored).
- Every `lib/<Module>/README.md`: purpose, responsibilities, planned
  features, future work.
- Every public class gets a Doxygen-style `/** ... */` block.
- `.gitignore`: `.pio/`, `docs/api/`.

## Out of Scope (explicit TODOs, not implemented this pass)

- Any real animation/interpolation (easing, spline, saccades, micro-saccades)
- Any real behavior logic (idle scanning, tracking, sleep/wake triggers, emotion)
- Any real networking (WebServer/RestApi/WebSocketServer bodies, Web UI)
- OTA implementation
- Calibration persistence to flash
- Expression blending

## Git

Work happens directly on `main` (repo currently has only `LICENSE` and a
2-line `README.md`, no prior history to protect).
