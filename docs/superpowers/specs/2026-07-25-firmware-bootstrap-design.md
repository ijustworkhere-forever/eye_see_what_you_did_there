# EyeSee Firmware Bootstrap — Design

Date: 2026-07-25 (revised after design review)

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

### Frame update model

The firmware is frame-based, not event-driven. Target update frequency: **100 Hz**.

Every module that has ongoing state implements `update(uint32_t deltaMs)`.
`main.cpp`'s `loop()` contains no business logic — it only measures elapsed
time and calls `update(dt)` down the chain:

```
loop()
  ↓
Clock (measures dt)
  ↓
BehaviorEngine.update(dt)
  ↓
IAnimationEngine.update(dt)
  ↓
EyeController.update(dt)
  ↓
IServoOutput.update(dt)
```

### Control-flow invariant

No code manipulates servos directly. All motion goes through `EyeController`,
and all *inputs* to the system are arbitrated by `BehaviorEngine` through a
single command queue — no input source is allowed to fight another:

```
Web UI / REST API / WebSocket / Bluetooth / Serial
                ↓  (push EyeCommand)
              CommandQueue
                ↓  (drained each frame)
           BehaviorEngine  — decides WHAT: "look over there"       (EyeCommand)
                ↓
          IAnimationEngine — decides HOW: "350ms, cubic easing"    (GazeTarget in, EyePose out)
                ↓
            EyeController  — decides WHERE: pose → calibrated servo pulses  (EyePose in, ServoOutput out)
                ↓
            IServoOutput   — moves hardware                        (ServoOutput in)
                ↓
              PCA9685 (or other future actuator hardware)
```

`BehaviorEngine` is the single arbiter between the outside world and the eye
— every REST/WebSocket/BT/Serial handler, present or future, pushes an
`EyeCommand` into the same `CommandQueue` rather than calling `EyeController`
or `IAnimationEngine` directly. This is a change from the original draft
(which allowed manual commands to bypass `BehaviorEngine`) — the reviewer's
point about inputs fighting each other is worth the extra indirection.

**Design synthesis note (confirmed):** `GazeTarget` (position + speed +
blinkOnArrival + hold) is the vocabulary of the **command layer** — what
`EyeCommand`/`BehaviorEngine`/`IAnimationEngine` pass around. `IAnimationEngine`
converts that into a plain `EyePose` each frame, and `EyeController` converts
`EyePose` into a calibrated `ServoOutput`. Each arrow in the pipeline is a
different value type, and each stage does exactly one kind of conversion —
no stage carries data it doesn't need.

### Three invariants

These are load-bearing and worth stating explicitly, since they're what
keeps the layers above from re-coupling as the codebase grows:

1. **`EyeController` never owns time.** No delays, no timers, no easing, no
   animation — it holds only its *current* `EyePose` as a value, and
   converts it to a `ServoOutput` synchronously. All animation lives in
   `IAnimationEngine`.
2. **`MotionHardware` never owns state.** `IServoOutput` receives a
   `ServoOutput`, writes PWM, done. It doesn't remember what it was asked to
   do last frame beyond whatever the underlying driver chip itself holds.
3. **`Behavior` never knows hardware.** `BehaviorEngine`/`IBehavior` only
   ever deal in `EyeCommand`/`GazeTarget`/`Expression` — never servo
   channels, pulse widths, calibration, or inversion. Those concepts don't
   exist above `EyeController`.

A useful side effect of invariant 2 in particular: swapping `IServoOutput`
for a host-side implementation (e.g. an SDL window drawing the eyes) is a
one-class change with zero impact on `EyeController`/`Animation`/`Behavior`.
Not built this pass, but the boundary is what makes it possible later.

### Namespace

All firmware code lives under `namespace eyesee`.

### Module layout

Each module is a flat PlatformIO private library under `lib/<Module>/`
(header + source at the library root). Every module folder has its own
`README.md` (purpose, responsibilities, planned features, future work).

```
include/            # project-wide public headers (currently empty/placeholder)
src/                 # main.cpp only — thin wiring + frame clock, no logic
lib/
  EyeController/     # concrete: sole owner of eye motion state; GazeTarget, Expression, EyePose
  MotionHardware/     # IServoOutput + Pca9685ServoOutput (renamed from ServoDriver); ServoOutput
  Animation/           # IAnimationEngine + PassthroughAnimationEngine stub
  Behavior/            # IBehaviorEngine + BehaviorEngine, IBehavior + IdleBehaviorStub,
                       # EyeState, EyeCommand/CommandQueue
  Networking/          # WebServer, RestApi, WebSocketServer placeholders
  Storage/             # IStorage + PreferencesStore (ESP32 Preferences wrapper)
  OTA/                 # OtaManager placeholder
  Logger/              # static Logger utility
  Configuration/       # ServoConfig, EyeConfig, NetworkConfig, BehaviorConfig structs
data/                # reserved for future SPIFFS/LittleFS web assets
docs/
  architecture.md    # this diagram + module responsibilities, long-form
  ROADMAP.md         # milestone roadmap, v0.1 → v1.0
  Doxyfile           # doxygen config, scans lib/ + src/, outputs docs/api/ (gitignored)
examples/            # reserved for future example sketches/configs
test/
  test_native/       # hardware-independent Unity tests (run on host via `pio test -e native`)
.github/
  workflows/ci.yml    # build + native test + clang-format check
.clang-format
```

### Interfaces vs. concrete classes

| Module | Interface? | Why |
|---|---|---|
| MotionHardware | `IServoOutput` → `Pca9685ServoOutput` | future actuator hardware: ESP32 LEDC PWM, other PWM chips, Dynamixel, CAN servos, or even a host-side simulator — renamed from `ServoDriver`/`IServoDriver` so the abstraction isn't PCA9685-specific |
| Animation | `IAnimationEngine` → `PassthroughAnimationEngine` (stub) | interpolation strategies will multiply (linear/easing/spline) |
| Behavior (engine) | `IBehaviorEngine` → `BehaviorEngine` | alternate top-level orchestration is plausible (e.g. a test harness engine) |
| Behavior (strategy) | `IBehavior` → `IdleBehaviorStub` (only one stub this pass) | plugin-style behaviors (Idle/Tracking/Curious/Random/Sleep) share one contract; only `Idle` gets a stub body — the rest are listed in `docs/ROADMAP.md` rather than scaffolded as empty files, to avoid five placeholder classes with nothing in them yet |
| Storage | `IStorage` → `PreferencesStore` | swappable persistence backend, testability |
| EyeController | none (concrete) | single required implementation — this class *is* the abstraction boundary for motion |
| CalibrationManager | none (concrete) | single required implementation |
| CommandQueue | none (concrete data structure) | fixed-capacity ring buffer, not a strategy — nothing to swap |
| Networking (WebServer/RestApi/WebSocketServer) | none (concrete, empty stub bodies) | only one implementation anticipated |
| OTA (OtaManager) | none (concrete, empty stub bodies) | single implementation |
| Logger | none (static utility) | cross-cutting concern; documented exception to "avoid globals" |

### EyeController

Concrete class, constructor-injected with `IServoOutput&` and
`CalibrationManager&`. Owns the small value types used across the pipeline:

```cpp
enum class Expression { Neutral, Happy, Curious, Sleepy, Angry, Surprised };

struct GazeTarget {
    float x;
    float y;
    float speed;            // used by IAnimationEngine, ignored by EyeController
    bool blinkOnArrival;
    bool hold;
};

// The complete instantaneous "desired shape" of both eyes — no timing,
// no easing, just values. This is what IAnimationEngine produces once per
// frame and EyeController converts to hardware output.
struct EyePose {
    float lookX;         // normalized -1..1
    float lookY;         // normalized -1..1
    float upperLeftLid;  // normalized 0 (closed) .. 1 (open)
    float lowerLeftLid;
    float upperRightLid;
    float lowerRightLid;
};
```

The one true primitive is pose conversion; the named methods are thin
convenience wrappers over it (e.g. `blink()` builds the canonical
"eyelids closed" `EyePose` from the current gaze and applies it — instantly,
no animation, per invariant 1 above):

```cpp
class EyeController {
public:
    EyeController(IServoOutput& output, CalibrationManager& calibration);

    void applyPose(const EyePose& pose);   // the one true primitive
    void look(float x, float y);           // convenience: updates gaze, preserves eyelids
    void blink();
    void winkLeft();
    void winkRight();
    void sleep();
    void wake();
    void setExpression(Expression expression);
    void setIdle();
    void update(uint32_t deltaMs);         // reserved; currently a no-op, since EyeController owns no timed state

private:
    EyePose currentPose_;                  // last-applied pose, so partial updates (e.g. look()) don't clobber eyelids
    IServoOutput& output_;
    CalibrationManager& calibration_;

    ServoOutput toServoOutput(const EyePose& pose) const; // applies calibration: pulse range, offset, invert, mirror
};
```

All bodies are stubbed (TODO) this pass — `toServoOutput()` and `applyPose()`
get real math later; right now they exist so the pipeline compiles and the
type boundaries are locked in.

### MotionHardware (renamed from ServoDriver)

```cpp
// Calibrated pulse widths for all six channels — the hardware-facing payload.
struct ServoOutput {
    uint16_t lr, ud, tl, bl, tr, br;  // microseconds
};

class IServoOutput {
public:
    virtual ~IServoOutput() = default;
    virtual void write(const ServoOutput& output) = 0;              // primary bulk write, called every frame by EyeController
    virtual void moveServo(uint8_t channel, float angleDegrees) = 0; // manual/diagnostic/calibration use
    virtual void setAngle(uint8_t channel, float angleDegrees) = 0;
    virtual void setPulse(uint8_t channel, uint16_t pulseUs) = 0;
    virtual void update(uint32_t deltaMs) = 0;
};
```

`Pca9685ServoOutput` implements it using the Adafruit PWM Servo Driver
library; constructor takes I2C address (default `0x40`). `write()` is what
the per-frame pipeline uses; the per-channel methods stay for calibration
tooling and diagnostics, per the original driver spec.

### Animation module

```cpp
class IAnimationEngine {
public:
    virtual ~IAnimationEngine() = default;
    virtual void animateGaze(const GazeTarget& target) = 0;
    virtual void animateBlink(uint32_t durationMs) = 0;
    virtual void animateExpression(Expression expression, uint32_t durationMs) = 0;
    virtual void update(uint32_t deltaMs) = 0;   // advances any in-progress animation, calls EyeController::applyPose()
};
```

`PassthroughAnimationEngine` is constructor-injected with `EyeController&`
and implements the interface with no real interpolation this pass (TODO —
easing/spline math is explicitly out of scope): `animateGaze()` computes the
target `EyePose` immediately and hands it straight to `applyPose()`, skipping
the "take 350ms" part for now. The seam is real even though the math isn't.

### CalibrationManager & Configuration

Plain structs with `constexpr` defaults, no behavior — calibration is
expressed entirely in **pulse widths**, not angles, so it's independent of
servo brand; angle is a computed value at the `MotionHardware` boundary:

```cpp
struct ServoConfig {
    uint8_t channel;
    uint16_t minPulseUs;
    uint16_t maxPulseUs;
    uint16_t neutralPulseUs;
    int16_t mechanicalOffset;
    bool inverted;
    bool mirrored;
};

struct EyeConfig {
    ServoConfig lr, ud, tl, bl, tr, br;
    float lookRangeDegrees;
};

struct NetworkConfig { /* SSID/password placeholders, web server port */ };
struct BehaviorConfig { /* idle-timing placeholders */ };
```

`CalibrationManager` owns an in-memory `EyeConfig`, exposes getters/setters
per servo. Persistence is a stubbed call into `IStorage` (TODO — not
implemented this pass).

### Behavior module

**State machine** — `BehaviorEngine` owns the system-level state (replacing
ad hoc booleans like `sleeping`/`idle`/`tracking`):

```cpp
enum class EyeState {
    Startup, Calibration, Manual, Idle, Tracking, Sleeping, Disabled, Error
};
```

**Command queue** — every input source (present or future) pushes an
`EyeCommand`; nothing calls `IAnimationEngine`/`EyeController` directly:

```cpp
enum class CommandType { Look, Blink, WinkLeft, WinkRight, Sleep, Wake, SetExpression };
enum class CommandPriority { Low, Normal, High };

struct EyeCommand {
    CommandType type;
    CommandPriority priority;
    uint32_t durationMs;
    GazeTarget gazeTarget;     // valid when type == Look
    Expression expression;    // valid when type == SetExpression
};

class CommandQueue {
public:
    bool push(const EyeCommand& command);
    bool pop(EyeCommand& outCommand);
    void clear();
private:
    std::array<EyeCommand, kCapacity> buffer_; // fixed-capacity ring buffer, no heap allocation
    // TODO: real ring-buffer indexing; this pass just defines the contract
};
```

**Plugin-friendly behaviors** — `IBehaviorEngine`'s concrete `BehaviorEngine`
delegates per-frame `update(dt)` to whichever `IBehavior` is active for the
current `EyeState`:

```cpp
class IBehavior {
public:
    virtual ~IBehavior() = default;
    virtual void update(uint32_t deltaMs, IAnimationEngine& animation) = 0;
    virtual EyeState state() const = 0;
};
```

Only `IdleBehaviorStub` is scaffolded this pass (empty body, proves the
interface compiles and wires up). `TrackingBehavior`, `CuriousBehavior`,
`RandomBehavior`, `SleepBehavior` are named in `docs/ROADMAP.md` as v0.3+
work rather than stubbed as empty files now — avoids scaffolding classes
with literally nothing in them.

### Networking / OTA / Storage

Placeholder-only this pass:

- `WebServer`, `RestApi`, `WebSocketServer` — `begin()`/`update()` stubs,
  empty bodies, TODOs documenting future endpoints under a **versioned**
  prefix: `GET /api/v1/status`, `POST /api/v1/look`, `POST /api/v1/blink`,
  `POST /api/v1/expression`, `POST /api/v1/config`, `GET /api/v1/config`.
  Target WebSocket update rate 30–60 Hz. No networking library dependency
  added yet.
- `OtaManager` — placeholder class, empty `begin()`/`update()`.
- `PreferencesStore` (`IStorage` impl) — wraps ESP32 `Preferences.h`
  (built into the Arduino core, no extra `lib_dep`), stub get/set methods.

### Logger

Static-method utility class: `Logger::debug/info/warn/error(tag, message)`,
backed by `Serial`. `enum class LogLevel`. Documented exception to "avoid
globals" — cross-cutting, and threading a logger reference through every
constructor buys nothing.

### main.cpp

```cpp
Pca9685ServoOutput servoOutput;
CalibrationManager calibration;
EyeController eyeController(servoOutput, calibration);
PassthroughAnimationEngine animationEngine(eyeController);
CommandQueue commandQueue;
IdleBehaviorStub idleBehavior;
BehaviorEngine behaviorEngine(animationEngine, commandQueue, idleBehavior);

void setup() {
  Logger::init();
  servoOutput.init();
  behaviorEngine.setState(eyesee::EyeState::Startup);
}

void loop() {
  const uint32_t dt = frameClock.tick();
  behaviorEngine.update(dt);
  animationEngine.update(dt);
  eyeController.update(dt);
  servoOutput.update(dt);
}
```

## Build Configuration

`platformio.ini` — two environments: the real target, and a `native` one for
hardware-independent unit tests (this is why `Configuration`, `CalibrationManager`,
`EyeCommand`/`CommandQueue`, and `EyeController`'s value types must stay
free of `Arduino.h`/hardware includes):

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = adafruit/Adafruit PWM Servo Driver Library
build_flags = -std=gnu++17
build_unflags = -std=gnu++11
monitor_speed = 115200

[env:native]
platform = native
build_flags = -std=c++17
test_filter = test_native/*
```

## Testing

One PlatformIO Unity test under `test/test_native/` (e.g.
`test_calibration_defaults`) asserting a trivial, real fact (default neutral
pulse from `CalibrationManager`) — runs via `pio test -e native` with no
hardware required, which is also what CI runs.

## Continuous Integration

`.github/workflows/ci.yml`, on every push/PR:

1. Checkout, set up Python, `pip install platformio`.
2. `pio run -e esp32dev` — build the real firmware target.
3. `pio test -e native` — run hardware-independent unit tests.
4. `clang-format --dry-run --Werror` over `lib/` and `src/` against the
   root `.clang-format`.

`clang-tidy` is **not** wired into CI this pass — getting it working against
the ESP32/Arduino toolchain is a real chunk of work on its own and isn't a
scaffold-level task. Left as a `docs/ROADMAP.md` item instead of faking a
CI step that would need to be immediately disabled.

## Documentation

- Root `README.md` rewritten: project overview, hardware, architecture
  diagram, build/flash instructions, module list, contributing pointer.
- `docs/architecture.md`: expanded version of the architecture section above.
- `docs/ROADMAP.md`: milestone list, v0.1 (this scaffold) through v1.0.
- `docs/Doxyfile`: scans `lib/` + `src/`, outputs to `docs/api/` (gitignored).
- Every `lib/<Module>/README.md`: purpose, responsibilities, planned
  features, future work.
- Every public class gets a Doxygen-style `/** ... */` block.
- `.gitignore`: `.pio/`, `docs/api/`.

## Out of Scope (explicit TODOs, not implemented this pass)

- Any real animation/interpolation (easing, spline, saccades, micro-saccades)
  — `IAnimationEngine` exists, `animateGaze()`/`animateBlink()`/`animateExpression()`
  apply their target `EyePose` immediately instead of over time
- `EyeController::toServoOutput()` calibration math (channel mapping, pulse
  scaling, invert/mirror/offset) — signature exists, body is a stub
- Any real behavior logic (state transitions, idle scanning, tracking,
  sleep/wake triggers, emotion) — `IBehavior`/`EyeState` contracts exist,
  bodies don't
- `CommandQueue` ring-buffer indexing (contract only)
- Any real networking (WebServer/RestApi/WebSocketServer bodies, Web UI)
- OTA implementation
- Calibration persistence to flash
- Expression blending
- A host-side/SDL `IServoOutput` simulator (enabled by the design, not built)
- `clang-tidy` in CI

## Git

Work happens directly on `main`.
