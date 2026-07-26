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

See `docs/hardware.md` for wiring, I2C address, power-budget guidance, and
the full servo channel mapping.

## Architecture

No code manipulates servos directly. All motion goes through `EyeController`;
all inputs are arbitrated by `BehaviorEngine` through a single command queue:

```
Web UI (via REST) / REST API / MQTT / Bluetooth gamepad
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

See `docs/architecture.md` for the full design, including the four core
invariants (EyeController owns no time; MotionHardware owns no state;
Behavior never knows hardware; Networking only writes via CommandQueue)
and `docs/ROADMAP.md` for what's planned.

## Building & flashing

Requires [PlatformIO](https://platformio.org/).

1. Copy the credential templates and fill in real values for your network:
   ```bash
   cp include/WifiCredentials.h.example include/WifiCredentials.h
   cp include/MqttCredentials.h.example include/MqttCredentials.h
   ```
   Both real files are gitignored — never commit them. If you don't use
   MQTT, the placeholder values in `MqttCredentials.h.example` are enough
   to build (the device will just fail to connect to a nonexistent
   broker, harmlessly).

2. Build and flash the firmware:
   ```bash
   pio run -e esp32dev              # build only
   pio run -e esp32dev -t upload    # build and flash over USB
   ```

3. Flash the filesystem image (required once, for the Web UI to exist at
   all — a firmware-only flash does not touch this partition):
   ```bash
   pio run -t uploadfs -e esp32dev
   ```

4. Run the hardware-independent unit test suite:
   ```bash
   pio test -e native
   ```

5. (Optional, once the device is on your network) reflash without a USB
   cable via OTA — this firmware advertises itself via mDNS as `eyesee`,
   but `platformio.ini` doesn't preconfigure an OTA upload target, so pass
   it explicitly:
   ```bash
   pio run -t upload --upload-protocol espota --upload-port eyesee.local
   ```
   (or substitute the device's IP address for `eyesee.local` if mDNS
   resolution doesn't work on your network).

## First boot & calibration

On first boot every channel uses `Configuration.h`'s defaults, which are
unlikely to be exactly right for your physical servos. Calibrate each of
the six channels (`lr`, `ud`, `tl`, `bl`, `tr`, `br`) with
`POST /api/v1/config` — see `docs/api-reference.md` for the full field
list. Example, calibrating the left/right look channel:

```bash
curl -X POST http://<device-ip>/api/v1/config \
  -H "Content-Type: application/json" \
  -d '{"channel": "lr", "minPulseUs": 1000, "maxPulseUs": 2000, "neutralPulseUs": 1500, "mechanicalOffset": 0, "inverted": false, "mirrored": false}'
```

Each change is persisted to flash immediately — no separate save step.
`docs/hardware.md` explains the pulse-width fields; `examples/calibration.json.example`
has a ready-to-copy version of this same request body.

## Controlling the eye

Three independent ways to send commands — see `docs/api-reference.md` for
the full protocol reference of all three.

**REST:**
```bash
curl -X POST http://<device-ip>/api/v1/look -H "Content-Type: application/json" -d '{"x": 0.5, "y": -0.25}'
```

**MQTT** (requires a broker reachable from the device — see
`examples/mosquitto-compose.yml` for a disposable local one):
```bash
mosquitto_pub -h <broker-host> -t eyesee/command -m '{"type": "look", "x": 0.5, "y": -0.25}'
```

**Bluetooth gamepad:** pair an Xbox Wireless Controller with the device
over BLE — see `lib/Integrations/README.md` for the button-to-command
mapping.

**Web UI:** open `http://<device-ip>/` in a browser once the filesystem
image is flashed (see step 3 above) — it's a front-end built on the same
REST and WebSocket APIs documented above, not an independent protocol.

More examples of every command type for all three transports live in
`examples/`.

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
| `lib/Integrations` | MQTT and Bluetooth gamepad input, native-tested mapping logic |
| `lib/OTA` | Over-the-air firmware updates via ArduinoOTA (no listener auth — see docs/known-limitations.md) |
| `lib/Logger` | Serial logging |
| `lib/Configuration` | Typed config structs + `CalibrationManager` |

Each module folder has its own `README.md` with more detail.

## Contributing

This is a young, actively-scaffolded project — see `docs/ROADMAP.md` for
what's planned and `docs/architecture.md` for the design rules new code
should follow. Issues and PRs welcome.

## License

CC0 1.0 Universal — see `LICENSE`.
