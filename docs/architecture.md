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

## Four invariants

1. **`EyeController` never owns time.** No delays, timers, easing, or
   animation — it holds only its current `EyePose` and converts it to a
   `ServoOutput` synchronously. All animation lives in `IAnimationEngine`.
2. **`MotionHardware` never owns state.** `IServoOutput` receives a
   `ServoOutput`, writes PWM, done.
3. **`Behavior` never knows hardware.** `BehaviorEngine`/`IBehavior` only
   deal in `EyeCommand`/`GazeTarget`/`Expression` — never servo channels,
   pulse widths, calibration, or inversion.
4. **`Networking` only writes via `CommandQueue`.** Every input source
   (Web UI, REST, WebSocket, Bluetooth, Serial, MQTT) pushes an
   `EyeCommand`; reads are limited to existing read-only getters
   (`IBehaviorEngine::state()`, `EyeController::currentPose()`,
   `WifiManager::isConnected()` -- used by `RestApi` since v0.5 and by
   `MqttBridge` since v0.6) — never `IAnimationEngine` or hardware
   directly. One documented exception: see "Persistence & OTA (v0.5)"
   below for `RestApi`'s config route.

A side effect of invariant 2: swapping `IServoOutput` for a host-side
implementation (e.g. an SDL window drawing the eyes) is a one-class change
with zero impact on `EyeController`/`Animation`/`Behavior`.

## Animation engine (v0.2, extended in v0.3)

`RealAnimationEngine` (in `lib/Animation/`) replaces the v0.1
`PassthroughAnimationEngine` placeholder. It runs two independent,
per-frame-composed transitions — one for gaze (`lookX`/`lookY`), one for
all four eyelids — each eased over its own duration via `Easing.h`'s three
curves (`Linear`, `EaseInOut`, `Cubic`), assigned per animation intent: gaze
uses Cubic; all eyelid moves (blink, wink, sleep, wake, expression) use
EaseInOut. Blink is the only one-way-vs-symmetric case — `animateBlink()`
closes then auto-reopens over the same duration it closed with
(`EyelidTransition::autoReopenOnComplete`); wink and sleep remain one-way,
same as before. Expression blending uses the shared
`expressionEyelidTarget()` lookup table (`lib/EyeController/ExpressionPose.h`)
for both the instant path (`EyeController::setExpression`) and the animated
path (`RealAnimationEngine::animateExpression`, which eases toward the same
table's values via the eyelid-transition machinery), so the two paths can
never disagree on what an `Expression` means. `GazeTarget.speed`
(degrees/second) combines with `EyeConfig.lookRangeDegrees` to compute how
long a gaze transition takes. See
`docs/superpowers/specs/2026-07-25-v0.2-real-motion-design.md` and
`docs/superpowers/specs/2026-07-25-v0.3-behavior-design.md` for the full
design. `GazeTarget.hold` is read (not set) by `TrackingBehavior` as of
v0.6 — see "Integrations (v0.6)" below.

## Behavior switching (v0.3)

`BehaviorEngine` now holds a fixed-size (`std::array<IBehavior*, 8>`)
registration table mapping `EyeState` to a concrete `IBehavior`, set up via
`registerBehavior()`. `setState()` looks up the table (falling back to a
constructor-injected default if nothing's registered for that state), and
calls the outgoing/incoming behavior's `onExit()`/`onEnter()` lifecycle
hooks — both default no-ops on `IBehavior`, overridden only where a
behavior needs a one-time transition action (the glance-based behaviors
reset their countdown timer on entry). Closing the eyes for a `Sleep`
command is handled by `BehaviorEngine::dispatch()` itself, before
`setState(Sleeping)` runs — see the next paragraph.

`Sleep`/`Wake` `EyeCommand`s now also drive the state transition
(`BehaviorEngine::dispatch()` calls `setState(Sleeping)`/`setState(Idle)`
in addition to the animation call) — see
docs/superpowers/specs/2026-07-25-v0.3-behavior-design.md for why this
lives in `BehaviorEngine` rather than a future REST layer.

`IdleBehavior`, `CuriousBehavior`, and `RandomBehavior` all depend on an
injectable `IRandomSource` (real: `ArduinoRandomSource`; test: a
`FakeRandomSource` returning a pre-programmed sequence) so glance timing
and direction stay deterministic in native tests.

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

Logger, OTA: leaf modules, no dependency on the above. Storage
(`IStorage`) is also a leaf, but `Configuration` now depends on it
(`CalibrationManager::loadFromStorage`/`saveToStorage`, v0.5) — the
dependency runs Configuration -> Storage, not the reverse. Networking
depends on Protocol (v0.4) and, as of v0.5, on Configuration and Storage
too (`RestApi`'s config route). As of v0.6, Integrations depends on
Behavior (`MqttBridge.h` includes `CommandQueue.h` and `IBehaviorEngine.h`),
EyeController (`EyeController.h`), Networking (`WifiManager.h`), and
Protocol (`MqttBridge` reuses `lib/Protocol`'s JSON parsers via
`MqttCommandJson`) — it is not a leaf.
```

## Connectivity (v0.4)

`lib/Protocol` is a new leaf library: pure JSON encode/decode (via the
portable ArduinoJson), zero Arduino dependency, fully native-tested.
`lib/Networking`'s `WifiManager`/`WebServer`/`RestApi`/`WebSocketServer`
stay Arduino-only and native-ignored (unchanged `lib_ignore` entry) — they
wire `lib/Protocol`'s pure functions to ESPAsyncWebServer, sharing one
`AsyncWebServer` instance constructed in `main.cpp`. Every route either
reads existing read-only state (`IBehaviorEngine::state()`,
`EyeController::currentPose()`) or pushes an `EyeCommand` into the shared
`CommandQueue` — Networking never touches `IAnimationEngine` or hardware
directly, preserving invariant 3 the same way `Behavior` does.

`lib/Protocol`'s parse functions take an already-deserialized
`JsonVariantConst` -- ESPAsyncWebServer's JSON route handler
(`AsyncCallbackJsonWebHandler`) parses the raw request body itself and
responds 400 automatically on malformed JSON before a route's callback (and
therefore `lib/Protocol`) ever runs. `lib/Protocol`'s native tests cover
every field-level validation this project's own code performs; the
JSON-syntax layer above it is ESPAsyncWebServer's behavior, not
independently tested here.

See `docs/superpowers/specs/2026-07-25-v0.4-connectivity-design.md` for the
full design, including why the REST surface covers `wink`/`sleep`/`wake`
(not just the `look`/`blink`/`expression` this file's older "REST API
versioning" section below originally scoped).

## Persistence & OTA (v0.5)

`IStorage` gained `int16_t`/`bool`/`float` accessors alongside the existing
`uint16_t` pair, matching what `ServoConfig`/`EyeConfig` actually need.
`CalibrationManager` gained `loadFromStorage(IStorage&)`/
`saveToStorage(IStorage&) const` — explicit methods, not constructor
injection, so its existing constructors and every existing native test
stay unchanged. `main.cpp` calls `loadFromStorage()` once at boot;
`RestApi`'s new `POST /api/v1/config` route calls `setServoConfig()` then
`saveToStorage()` on every write, making persistence automatic from a
client's point of view without `CalibrationManager` itself needing to know
about flash. This is the one place `RestApi` mutates state outside
`CommandQueue` — calibration is static per-servo tuning, not a live-motion
`EyeCommand`, so routing it through the same queue that arbitrates gaze/
expression timing would misuse a mechanism built for a different purpose.
`OtaManager` wraps the global `ArduinoOTA` singleton with no new
architectural surface. See
`docs/superpowers/specs/2026-07-26-v0.5-persistence-ota-design.md` for the
full design, including the exact persistence key table (every key ≤15
characters, ESP32 NVS's hard limit).

## Integrations (v0.6)

Face-tracking reuses the existing REST/Protocol/Behavior machinery rather
than a new external protocol: a new `CommandType::Track` and
`IBehavior::receiveGazeTarget()` hook let `BehaviorEngine::dispatch()`
forward a `Track` command's payload to whichever behavior is active
(today, always `TrackingBehavior`, since `Track` also calls
`setState(EyeState::Tracking)`). `TrackingBehavior` now recenters once if
no new target arrives within 3 seconds, unless the last-received target's
`hold` field is `true` — implementing the "auto-return to center" decision
the original v0.2 spec always meant `GazeTarget.hold` to drive.

MQTT and Bluetooth gamepad are genuinely new external-protocol
integrations, living in a new library, `lib/Integrations` -- not a leaf:
it depends on Behavior (`CommandQueue`, `IBehaviorEngine`), EyeController,
Networking (`WifiManager`), and Protocol (see "Module dependency graph"
above). Both follow the same rule `Networking` established in v0.4: every input source
only ever pushes an `EyeCommand` into the shared `CommandQueue`, never
touching `IAnimationEngine` or hardware directly. `MqttBridge` reuses
every existing `lib/Protocol` JSON parser (no new per-command parsing
logic — only one small type-dispatch wrapper, `parseMqttCommand`).
`GamepadBridge` uses `tbekas/BLE-Gamepad-Client` as a BLE HID *host*
(the ESP32 connects to a real Xbox controller) — deliberately not
Bluepad32, which requires swapping this project's Arduino framework
entirely. `GamepadInputMapper` (the actual button/stick-to-`EyeCommand`
mapping) is pure and native-tested; only the BLE wiring itself is
Arduino-bound and native-ignored (via `#ifdef ARDUINO`, matching
`lib/Storage`'s v0.5 pattern — `lib/Integrations` is not folder-ignored).

## Namespace

All firmware code lives under `namespace eyesee`.

## REST API versioning

All REST routes are versioned under `/api/v1/`: `GET /api/v1/status`,
`POST /api/v1/look`, `POST /api/v1/blink`, `POST /api/v1/wink`,
`POST /api/v1/expression`, `POST /api/v1/track`, `POST /api/v1/sleep`,
`POST /api/v1/wake`, `GET`/`POST /api/v1/config`.

## Testing strategy

`Configuration`, `CalibrationManager`, `EyeController`, `Animation`, and
`Behavior` are free of `Arduino.h` and are unit-tested on the host via the
`native` PlatformIO environment (`pio test -e native`). `Logger`,
`MotionHardware` (specifically `Pca9685ServoOutput`), and `Storage`
(`PreferencesStore`) depend on Arduino/hardware headers and are verified only
by `pio run -e esp32dev` compiling successfully.

`Protocol` (JSON encode/decode) is Arduino-free (only ArduinoJson, which is
portable) and unit-tested on the host alongside `Configuration`,
`CalibrationManager`, `EyeController`, `Animation`, and `Behavior`.
`lib/Protocol/EyeConfigJson` and `CalibrationManager::loadFromStorage`/
`saveToStorage` (via a `FakeStorage` test double, the same pattern as
`FakeAnimationEngine`/`FakeRandomSource`) are native-tested the same way;
`PreferencesStore`'s new accessors and `OtaManager` stay Arduino-only,
verified only by `pio run -e esp32dev`.

`Networking` (`WifiManager`, `WebServer`, `RestApi`, `WebSocketServer`) is
Arduino/ESPAsyncWebServer-bound and verified only by `pio run -e esp32dev`,
same as `Logger`/`MotionHardware`/`Storage`/`OTA`.
