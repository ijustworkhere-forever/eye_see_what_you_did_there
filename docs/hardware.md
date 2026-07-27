# Hardware

## Bill of materials

- 1x ESP32 DevKit (any board exposing the standard I2C pins over its
  default `Wire` bus — this firmware never overrides them, see below)
- 1x Adafruit PCA9685 16-channel PWM/servo driver breakout
- 6x hobby servos (SG90 or MG90S) — 2 for eye look (left/right, up/down),
  4 for eyelids (upper/lower per eye)
- A 5V power supply for the servos, sized as described below (**do not**
  power 6 servos from the ESP32 board's own 5V pin)

## Wiring

**I2C (ESP32 <-> PCA9685):** this firmware never calls `Wire.begin()` with
explicit pins (confirmed via `grep -rn "Wire.begin\|SDA\|SCL" lib/ src/`
returning no matches — re-run this before trusting the pin numbers below
if the codebase changes), so it uses the ESP32 Arduino core's default
`Wire` pins:
**SDA = GPIO 21, SCL = GPIO 22** on most ESP32 DevKit boards. Connect the
PCA9685's `SDA`/`SCL` to those pins, plus a shared `GND`.

**PCA9685 I2C address:** `0x40` (the board's default; only relevant if you
add a second PCA9685 on the same bus, which this firmware does not support
today).

**PCA9685 power:** the board has two separate power inputs —
`VCC` (logic power, can come from the ESP32's 3.3V or 5V pin) and `V+`
(servo power, the pins that actually drive the servos). **Do not** power
`V+` from the ESP32's own 5V pin.

**Power budget:** a small hobby servo can draw on the order of 500mA-1A
stalled; six of them under simultaneous load can exceed 3-5A briefly. Feed
the PCA9685's `V+`/servo-power rail from a dedicated 5V supply sized for
your specific servo model's stall current × 6 (check its datasheet — SG90
and MG90S have different stall-current ratings), with that supply's ground
tied to the ESP32's ground. Powering servos directly from the ESP32 dev
board's onboard 5V regulator is a common cause of brownouts/resets under
load and is not recommended.

**Servo channel mapping** (PCA9685 channel -> role), consistent throughout
this firmware's code (`lib/Configuration/Configuration.h`,
`lib/MotionHardware/Pca9685ServoOutput.h`):

| PCA9685 channel | Code name | Role |
|---|---|---|
| 0 | `lr` | Left/right eye look |
| 1 | `ud` | Up/down eye look |
| 2 | `tl` | Left eye, upper lid |
| 3 | `bl` | Left eye, lower lid |
| 4 | `tr` | Right eye, upper lid |
| 5 | `br` | Right eye, lower lid |

## Pulse-width defaults

Every channel defaults to `minPulseUs=1000, maxPulseUs=2000,
neutralPulseUs=1500` (`lib/Configuration/Configuration.h`); the REST config
API (`POST /api/v1/config`, see `docs/api-reference.md`) allows any value
in the absolute range `500-2500us`, `minPulseUs < maxPulseUs`, with
`neutralPulseUs` between them. Real servos vary — use the calibration
walkthrough in the root `README.md` to tune each channel's actual range
for your specific hardware rather than assuming the defaults are correct
for your servos.

PWM frequency: 50Hz (standard for analog hobby servos).

## Physical build: 3D-printed parts and print settings

The eye mechanism itself is not designed in this repo — it's built from
[Will Cogley's Animatronic Eye Mechanism on MakerWorld](https://makerworld.com/en/models/1184807-animatronic-eye-mechanism-e3-2)
and its [Instructables build guide](https://www.instructables.com/Animatronic-Eye-Mechanism/)
(see the root `README.md`). `hardware/EyeMech_sled_compact.stl` in this
repo is a custom addition: a sled that mounts an ESP32 mini and the
PCA9685 board alongside the mechanism, not part of the original design.

**Print settings notes (from this project's own build, sliced for a
Bambu Lab X1 Carbon on the "0.12mm High Quality" profile):** getting a
clean print of the mechanism's fine support geometry in PLA required a
different material for the support base vs. the support interface —
PLA for the support/raft base, PETG for the support/raft interface — with
a rectilinear base pattern (2.5mm spacing, 0° angle), 90% initial layer
density, and 0.12mm top/bottom Z distance with 2 top interface layers.
Mixing support materials this way is a real trade-off, not a minor
tweak: it forced the slicer to split what would otherwise be a single
print plate into three, and added many hours to the total print time.
If a single-material PLA print isn't giving you clean support removal on
the fine eyelid/eye-socket geometry, this combination is what fixed it
for this build — but expect the same plate-splitting and time cost if
you use it too.
