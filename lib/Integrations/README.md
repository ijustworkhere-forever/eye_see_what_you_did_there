# Integrations

**Purpose:** New external-protocol input sources beyond HTTP/WebSocket
(`lib/Networking`) -- MQTT and Bluetooth gamepad control
(docs/ROADMAP.md v0.6).

**Responsibilities:** `GamepadInputMapper.h` -- pure, native-testable
mapping from one frame of gamepad state to zero or more `EyeCommand`s
(deadzone handling, button press-edge detection, no heap allocation).
`MqttBridge`/`GamepadBridge` (Arduino/BLE-bound, native-ignored via
`#ifdef ARDUINO`) wire that logic (and `lib/Protocol`'s existing JSON
parsers, for MQTT) to real hardware/network APIs. Every class here pushes
into the shared `CommandQueue` -- never touches `IAnimationEngine` or
hardware directly, the same rule `lib/Networking` already follows.

**Button mapping:** `GamepadInputMapper::mapGamepadState` (deadzone 0.1 on
both stick axes):

| Input | Command | Duration |
|---|---|---|
| Left stick | Look (x/y = stick axes) | -- |
| Button A | Blink | 150ms |
| Left bumper | WinkLeft | 150ms |
| Right bumper | WinkRight | 150ms |
| Button Y | SetExpression(Happy) | 200ms |
| Button B | Sleep | 500ms |
| Button X | Wake | 500ms |

All button commands fire once on the press edge (not every frame a button
is held).

**Consumed by:** `src/main.cpp`.
