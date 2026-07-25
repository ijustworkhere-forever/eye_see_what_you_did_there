# EyeSee Firmware Bootstrap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Scaffold the full EyeSee ESP32 + PCA9685 firmware module tree — interfaces, value types, and placeholder implementations — so the project compiles for `esp32dev`, passes a native unit-test suite, and is ready for future contributors to build real behavior on top of.

**Architecture:** Nine PlatformIO private libraries under `lib/`, each with a single responsibility, wired together in a thin `src/main.cpp` composition root. Every stateful module exposes `update(uint32_t deltaMs)`. Inputs are arbitrated through one `CommandQueue`; the pipeline `EyeCommand → GazeTarget → EyePose → ServoOutput` gives each layer (Behavior/Animation/EyeController/MotionHardware) its own value type so no layer carries data it doesn't need. Full design rationale: `docs/superpowers/specs/2026-07-25-firmware-bootstrap-design.md`.

**Tech Stack:** PlatformIO, Arduino framework, ESP32 (`board = esp32dev`), Adafruit PWM Servo Driver Library, Unity (via `pio test`), a second `native` PlatformIO environment for hardware-independent unit tests, GitHub Actions CI, clang-format, Doxygen.

## Global Constraints

- MCU: ESP32 DevKit. Servo driver: PCA9685 via Adafruit PWM Servo Driver Library. 6 servos: LR, UD, TL, BL, TR, BR.
- PlatformIO, `framework = arduino`, `board = esp32dev`.
- All firmware code lives under `namespace eyesee`.
- C++ standard: `gnu++17` for `esp32dev` (`build_unflags = -std=gnu++11` first, since the ESP32 Arduino core defaults to gnu++11), `c++17` for `native`.
- No third-party `lib_deps` beyond `adafruit/Adafruit PWM Servo Driver Library` this pass.
- No dynamic allocation unless necessary — `CommandQueue` is a fixed-capacity `std::array` ring buffer.
- Target frame rate: 100 Hz. Every stateful module exposes `update(uint32_t deltaMs)`.
- REST endpoints (documented only, not implemented) are versioned under `/api/v1/`.
- Every public class gets a Doxygen-style `/** ... */` block.
- Files stay under ~300 lines where practical.
- Every `lib/<Module>/` gets its own `README.md`.
- Work happens directly on `main`. Commit after each task.
- No advanced behavior/animation/networking logic this pass — every such body is a real, minimal, compiling stub with a `TODO` pointing at the `docs/ROADMAP.md` milestone that will implement it. "Stub" never means "silently wrong" — see per-task notes for exactly what each stub does.

## Notes on turning the spec into code (read before starting)

Four small decisions were needed to make the approved spec concrete; flagging them here rather than burying them in a task:

1. **`CalibrationManager` lives in `lib/Configuration/`**, not its own `lib/` folder. The spec's own top-level directory layout (from the original project brief) only lists `EyeController/ ServoDriver/ Animation/ Behavior/ Networking/ Storage/ OTA/ Logger/ Configuration/` — `CalibrationManager` was never its own folder, and it operates entirely on `Configuration`'s structs, so it's colocated with them.
2. **`IAnimationEngine` has four `animate*` methods, not one.** The spec's `EyeCommand`/`CommandType` enum includes `WinkLeft`, `WinkRight`, `Sleep`, `Wake` alongside `Look`/`Blink`/`SetExpression`, but the spec's `IAnimationEngine` sketch only showed `animateGaze`/`animateBlink`/`animateExpression`. For `BehaviorEngine` to dispatch every `CommandType` without ever calling `EyeController` directly (the whole point of the command-queue arbitration), `IAnimationEngine` needs `animateWinkLeft`, `animateWinkRight`, `animateSleep`, and `animateWake` too. Added below.
3. **`CommandQueue` is a real, fully-working fixed-capacity ring buffer**, not a stubbed contract. The spec listed its ring-buffer indexing as out of scope, but a correct fixed-size circular buffer is a few lines of uncontroversial data-structure code — nothing "advanced" (that term refers to animation/behavior/networking domain logic) — and leaving it half-implemented would mean the one task most exercised by later tests (`BehaviorEngine`) has an untested foundation.
4. **All native tests share one `test/test_native/test_main.cpp` runner.** Discovered during Task 4: PlatformIO's native test runner links every `.cpp` under a matched test directory into one executable, so only one file may define `int main()` per directory — not one per test file, as earlier drafts of this plan assumed. `test_main.cpp` (created as part of Task 4's fix-up) owns the single `UNITY_BEGIN()`/`RUN_TEST(...)`/`UNITY_END()` body; every other `test_*.cpp` file contributes test functions only (no `main()`), and each task that adds native tests appends `extern` declarations + `RUN_TEST` calls to `test_main.cpp` in its own step.

---

### Task 1: PlatformIO project bootstrap

**Files:**
- Create: `platformio.ini`
- Create: `src/main.cpp`
- Create: `.gitignore`
- Create: `include/README.md`
- Create: `data/README.md`
- Create: `examples/README.md`

**Interfaces:**
- Consumes: nothing (first task).
- Produces: a buildable `esp32dev` PlatformIO environment. Every later task builds on this.

- [ ] **Step 1: Create `platformio.ini`**

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
build_unflags = -std=gnu++11
build_flags = -std=gnu++17
```

- [ ] **Step 2: Create a minimal `src/main.cpp`**

```cpp
#include <Arduino.h>

void setup() {
}

void loop() {
}
```

- [ ] **Step 3: Create `.gitignore`**

```gitignore
.pio/
docs/api/
.vscode/
```

- [ ] **Step 4: Create placeholder READMEs for reserved directories**

`include/README.md`:
```markdown
# include/

Reserved for project-wide public headers shared across `lib/` modules and
`src/main.cpp`. Empty for now — each `lib/<Module>/` currently owns its own
headers.
```

`data/README.md`:
```markdown
# data/

Reserved for future SPIFFS/LittleFS web assets (the Web UI control panel —
see `docs/ROADMAP.md` v0.4). Empty this pass.
```

`examples/README.md`:
```markdown
# examples/

Reserved for future example configs/sketches demonstrating firmware usage.
Empty this pass.
```

- [ ] **Step 5: Verify it builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS` — an empty firmware that does nothing, but compiles and links.

- [ ] **Step 6: Commit**

```bash
git add platformio.ini src/main.cpp .gitignore include/README.md data/README.md examples/README.md
git commit -m "Bootstrap PlatformIO project skeleton"
```

---

### Task 2: Logger module

**Files:**
- Create: `lib/Logger/Logger.h`
- Create: `lib/Logger/Logger.cpp`
- Create: `lib/Logger/README.md`
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: `eyesee::Logger::init(unsigned long baudRate = 115200)`, `Logger::debug/info/warn/error(const char* tag, const char* message)` — static methods, used by every later module.

- [ ] **Step 1: Create `lib/Logger/Logger.h`**

```cpp
#pragma once

namespace eyesee {

/** Log severity, from least to most urgent. */
enum class LogLevel { Debug, Info, Warn, Error };

/**
 * Static logging utility backed by Serial.
 *
 * Static rather than instance-based on purpose: logging is cross-cutting,
 * and threading a Logger reference through every constructor in the
 * firmware buys nothing. This is the one intentional exception to
 * "avoid globals" (see docs/architecture.md).
 */
class Logger {
public:
    static void init(unsigned long baudRate = 115200);

    static void debug(const char* tag, const char* message);
    static void info(const char* tag, const char* message);
    static void warn(const char* tag, const char* message);
    static void error(const char* tag, const char* message);

private:
    static void log(LogLevel level, const char* tag, const char* message);
    static const char* levelLabel(LogLevel level);
};

}  // namespace eyesee
```

- [ ] **Step 2: Create `lib/Logger/Logger.cpp`**

```cpp
#include "Logger.h"

#include <Arduino.h>

namespace eyesee {

void Logger::init(unsigned long baudRate) {
    Serial.begin(baudRate);
}

void Logger::debug(const char* tag, const char* message) { log(LogLevel::Debug, tag, message); }
void Logger::info(const char* tag, const char* message) { log(LogLevel::Info, tag, message); }
void Logger::warn(const char* tag, const char* message) { log(LogLevel::Warn, tag, message); }
void Logger::error(const char* tag, const char* message) { log(LogLevel::Error, tag, message); }

void Logger::log(LogLevel level, const char* tag, const char* message) {
    Serial.print('[');
    Serial.print(levelLabel(level));
    Serial.print("] ");
    Serial.print(tag);
    Serial.print(": ");
    Serial.println(message);
}

const char* Logger::levelLabel(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warn: return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

}  // namespace eyesee
```

- [ ] **Step 3: Create `lib/Logger/README.md`**

```markdown
# Logger

**Purpose:** Cross-cutting logging abstraction backed by `Serial`.

**Responsibilities:** Provide `debug`/`info`/`warn`/`error` static calls with
a consistent `[LEVEL] tag: message` format.

**Planned features:** None beyond level filtering (e.g. compile-time or
runtime minimum level).

**Future work:** Optional log sinks other than Serial (e.g. a ring buffer
exposed over `/api/v1/status` for remote diagnostics).
```

- [ ] **Step 4: Modify `src/main.cpp`**

```cpp
#include <Arduino.h>

#include "Logger.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
}

void loop() {
}
```

- [ ] **Step 5: Verify it builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS`

- [ ] **Step 6: Commit**

```bash
git add lib/Logger src/main.cpp
git commit -m "Add Logger module"
```

---

### Task 3: Configuration module + native test environment

**Files:**
- Create: `lib/Configuration/Configuration.h`
- Create: `lib/Configuration/README.md`
- Create: `test/test_native/test_configuration_defaults.cpp`
- Modify: `platformio.ini`

**Interfaces:**
- Consumes: nothing.
- Produces: `eyesee::ServoConfig`, `eyesee::EyeConfig`, `eyesee::NetworkConfig`, `eyesee::BehaviorConfig`, `eyesee::makeDefaultEyeConfig()`. Consumed by `CalibrationManager` (Task 4) and, transitively, `EyeController` (Task 6).

- [ ] **Step 1: Create `lib/Configuration/Configuration.h`**

```cpp
#pragma once

#include <cstdint>

namespace eyesee {

/** Per-servo calibration, expressed entirely in pulse widths (not angles) so it's independent of servo brand. */
struct ServoConfig {
    uint8_t channel = 0;
    uint16_t minPulseUs = 1000;
    uint16_t maxPulseUs = 2000;
    uint16_t neutralPulseUs = 1500;
    int16_t mechanicalOffset = 0;
    bool inverted = false;
    bool mirrored = false;
};

/** Calibration for all six servos plus the eye's overall look range. */
struct EyeConfig {
    ServoConfig lr;
    ServoConfig ud;
    ServoConfig tl;
    ServoConfig bl;
    ServoConfig tr;
    ServoConfig br;
    float lookRangeDegrees = 30.0f;
};

/** Placeholder network settings — no networking implemented this pass. */
struct NetworkConfig {
    const char* ssid = "";
    const char* password = "";
    uint16_t webServerPort = 80;
};

/** Placeholder behavior timing — no behavior logic implemented this pass. */
struct BehaviorConfig {
    uint32_t idleTimeoutMs = 15000;
};

constexpr ServoConfig makeDefaultServoConfig(uint8_t channel) {
    ServoConfig config;
    config.channel = channel;
    return config;
}

/** Channel assignment: 0=LR, 1=UD, 2=TL, 3=BL, 4=TR, 5=BR. */
constexpr EyeConfig makeDefaultEyeConfig() {
    EyeConfig config;
    config.lr = makeDefaultServoConfig(0);
    config.ud = makeDefaultServoConfig(1);
    config.tl = makeDefaultServoConfig(2);
    config.bl = makeDefaultServoConfig(3);
    config.tr = makeDefaultServoConfig(4);
    config.br = makeDefaultServoConfig(5);
    return config;
}

}  // namespace eyesee
```

- [ ] **Step 2: Create `lib/Configuration/README.md`**

```markdown
# Configuration

**Purpose:** Strongly-typed, dependency-free data structs shared across the
firmware.

**Responsibilities:** Define `ServoConfig`, `EyeConfig`, `NetworkConfig`,
`BehaviorConfig` with sane `constexpr` defaults. Also owns `CalibrationManager`
(see below), since calibration is just typed access to `EyeConfig`.

**Planned features:** None — this module is data, not behavior.

**Future work:** `CalibrationManager` persistence via `IStorage` (see
`docs/ROADMAP.md` v0.5).
```

- [ ] **Step 3: Add the `native` test environment to `platformio.ini`**

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
build_unflags = -std=gnu++11
build_flags = -std=gnu++17

[env:native]
platform = native
build_flags = -std=c++17
test_filter = test_native/*
```

- [ ] **Step 4: Create `test/test_native/test_configuration_defaults.cpp`**

```cpp
#include <unity.h>

#include "Configuration.h"

using eyesee::makeDefaultEyeConfig;

void test_default_eye_config_has_six_distinct_channels() {
    const auto config = makeDefaultEyeConfig();
    TEST_ASSERT_EQUAL_UINT8(0, config.lr.channel);
    TEST_ASSERT_EQUAL_UINT8(1, config.ud.channel);
    TEST_ASSERT_EQUAL_UINT8(2, config.tl.channel);
    TEST_ASSERT_EQUAL_UINT8(3, config.bl.channel);
    TEST_ASSERT_EQUAL_UINT8(4, config.tr.channel);
    TEST_ASSERT_EQUAL_UINT8(5, config.br.channel);
}

void test_default_servo_config_is_centered_and_uninverted() {
    const auto config = makeDefaultEyeConfig();
    TEST_ASSERT_EQUAL_UINT16(1500, config.lr.neutralPulseUs);
    TEST_ASSERT_FALSE(config.lr.inverted);
    TEST_ASSERT_FALSE(config.lr.mirrored);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_default_eye_config_has_six_distinct_channels);
    RUN_TEST(test_default_servo_config_is_centered_and_uninverted);
    return UNITY_END();
}
```

- [ ] **Step 5: Run the test to verify it passes**

Run: `pio test -e native`
Expected: `2 Tests 0 Failures 0 Ignored` — `PASSED`

- [ ] **Step 6: Verify `esp32dev` still builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS`

- [ ] **Step 7: Commit**

```bash
git add lib/Configuration platformio.ini test/test_native/test_configuration_defaults.cpp
git commit -m "Add Configuration module and native test environment"
```

---

### Task 4: CalibrationManager

**Files:**
- Create: `lib/Configuration/CalibrationManager.h`
- Create: `lib/Configuration/CalibrationManager.cpp`
- Create: `test/test_native/test_calibration_manager.cpp`
- Modify: `lib/Configuration/README.md`

**Interfaces:**
- Consumes: `eyesee::EyeConfig`, `eyesee::ServoConfig`, `eyesee::makeDefaultEyeConfig()` (Task 3).
- Produces: `eyesee::EyeChannel`, `eyesee::CalibrationManager` with `eyeConfig()`, `servoConfig(EyeChannel)`, `setServoConfig(EyeChannel, const ServoConfig&)`. Consumed by `EyeController` (Task 6).

- [ ] **Step 1: Create `lib/Configuration/CalibrationManager.h`**

```cpp
#pragma once

#include "Configuration.h"

namespace eyesee {

/** Selects one of the six calibrated servos. */
enum class EyeChannel { LR, UD, TL, BL, TR, BR };

/**
 * Owns the in-memory EyeConfig: per-servo limits, neutral position,
 * inversion, mirroring, and mechanical offset.
 *
 * Persistence to flash (via IStorage) is not implemented this pass —
 * see docs/ROADMAP.md v0.5.
 */
class CalibrationManager {
public:
    CalibrationManager();
    explicit CalibrationManager(const EyeConfig& initialConfig);

    const EyeConfig& eyeConfig() const;
    const ServoConfig& servoConfig(EyeChannel channel) const;
    void setServoConfig(EyeChannel channel, const ServoConfig& config);

private:
    EyeConfig config_;
    ServoConfig& mutableServoConfig(EyeChannel channel);
};

}  // namespace eyesee
```

- [ ] **Step 2: Create `lib/Configuration/CalibrationManager.cpp`**

```cpp
#include "CalibrationManager.h"

namespace eyesee {

CalibrationManager::CalibrationManager() : config_(makeDefaultEyeConfig()) {}

CalibrationManager::CalibrationManager(const EyeConfig& initialConfig) : config_(initialConfig) {}

const EyeConfig& CalibrationManager::eyeConfig() const {
    return config_;
}

const ServoConfig& CalibrationManager::servoConfig(EyeChannel channel) const {
    switch (channel) {
        case EyeChannel::LR: return config_.lr;
        case EyeChannel::UD: return config_.ud;
        case EyeChannel::TL: return config_.tl;
        case EyeChannel::BL: return config_.bl;
        case EyeChannel::TR: return config_.tr;
        case EyeChannel::BR: return config_.br;
    }
    return config_.lr;
}

void CalibrationManager::setServoConfig(EyeChannel channel, const ServoConfig& config) {
    mutableServoConfig(channel) = config;
    // TODO: persist via IStorage (docs/ROADMAP.md v0.5) — in-memory only this pass.
}

ServoConfig& CalibrationManager::mutableServoConfig(EyeChannel channel) {
    switch (channel) {
        case EyeChannel::LR: return config_.lr;
        case EyeChannel::UD: return config_.ud;
        case EyeChannel::TL: return config_.tl;
        case EyeChannel::BL: return config_.bl;
        case EyeChannel::TR: return config_.tr;
        case EyeChannel::BR: return config_.br;
    }
    return config_.lr;
}

}  // namespace eyesee
```

- [ ] **Step 3: Create `test/test_native/test_calibration_manager.cpp`**

```cpp
#include <unity.h>

#include "CalibrationManager.h"

using eyesee::CalibrationManager;
using eyesee::EyeChannel;
using eyesee::ServoConfig;

void test_default_calibration_reports_default_lr_channel() {
    CalibrationManager calibration;
    TEST_ASSERT_EQUAL_UINT8(0, calibration.servoConfig(EyeChannel::LR).channel);
    TEST_ASSERT_EQUAL_UINT16(1500, calibration.servoConfig(EyeChannel::LR).neutralPulseUs);
}

void test_set_servo_config_updates_stored_value() {
    CalibrationManager calibration;
    ServoConfig updated = calibration.servoConfig(EyeChannel::UD);
    updated.inverted = true;
    updated.neutralPulseUs = 1600;

    calibration.setServoConfig(EyeChannel::UD, updated);

    TEST_ASSERT_TRUE(calibration.servoConfig(EyeChannel::UD).inverted);
    TEST_ASSERT_EQUAL_UINT16(1600, calibration.servoConfig(EyeChannel::UD).neutralPulseUs);
}

void test_set_servo_config_does_not_affect_other_channels() {
    CalibrationManager calibration;
    ServoConfig updated = calibration.servoConfig(EyeChannel::TL);
    updated.neutralPulseUs = 1700;

    calibration.setServoConfig(EyeChannel::TL, updated);

    TEST_ASSERT_EQUAL_UINT16(1500, calibration.servoConfig(EyeChannel::TR).neutralPulseUs);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_default_calibration_reports_default_lr_channel);
    RUN_TEST(test_set_servo_config_updates_stored_value);
    RUN_TEST(test_set_servo_config_does_not_affect_other_channels);
    return UNITY_END();
}
```

- [ ] **Step 4: Update `lib/Configuration/README.md`** — append:

```markdown

**CalibrationManager:** In-memory owner of `EyeConfig`. `EyeChannel` selects
which of the six servos a getter/setter operates on.
```

- [ ] **Step 5: Run the test to verify it passes**

Run: `pio test -e native`
Expected: `5 Tests 0 Failures 0 Ignored` (2 from Task 3 + 3 new) — `PASSED`

- [ ] **Step 6: Commit**

```bash
git add lib/Configuration test/test_native/test_calibration_manager.cpp
git commit -m "Add CalibrationManager"
```

---

### Task 5: MotionHardware module (IServoOutput + Pca9685ServoOutput)

**Files:**
- Create: `lib/MotionHardware/IServoOutput.h`
- Create: `lib/MotionHardware/Pca9685ServoOutput.h`
- Create: `lib/MotionHardware/Pca9685ServoOutput.cpp`
- Create: `lib/MotionHardware/README.md`
- Modify: `platformio.ini`
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: Adafruit PWM Servo Driver Library (`Adafruit_PWMServoDriver`).
- Produces: `eyesee::ServoOutput { uint16_t lr, ud, tl, bl, tr, br; }`, `eyesee::IServoOutput` (`write`, `moveServo`, `setAngle`, `setPulse`, `update`), `eyesee::Pca9685ServoOutput`. `IServoOutput`/`ServoOutput` are Arduino-free and consumed by `EyeController` (Task 6, including its native tests via a fake).

- [ ] **Step 1: Add the library dependency to `platformio.ini`**

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
build_unflags = -std=gnu++11
build_flags = -std=gnu++17
lib_deps = adafruit/Adafruit PWM Servo Driver Library

[env:native]
platform = native
build_flags = -std=c++17
test_filter = test_native/*
```

- [ ] **Step 2: Create `lib/MotionHardware/IServoOutput.h`**

```cpp
#pragma once

#include <cstdint>

namespace eyesee {

/** Calibrated pulse widths (microseconds) for all six channels — the hardware-facing payload. */
struct ServoOutput {
    uint16_t lr = 0;
    uint16_t ud = 0;
    uint16_t tl = 0;
    uint16_t bl = 0;
    uint16_t tr = 0;
    uint16_t br = 0;
};

/**
 * Abstracts the physical actuator hardware. Deliberately not PCA9685-specific
 * — future implementations could target ESP32 LEDC PWM, a different PWM
 * chip, Dynamixel/CAN servos, or a host-side simulator for desktop testing.
 *
 * Owns no state beyond whatever the underlying driver chip itself holds
 * (see docs/architecture.md invariant 2).
 */
class IServoOutput {
public:
    virtual ~IServoOutput() = default;

    /** Primary per-frame call: write all six calibrated channels at once. */
    virtual void write(const ServoOutput& output) = 0;

    /** Manual/diagnostic/calibration use — not used by the per-frame pipeline. */
    virtual void moveServo(uint8_t channel, float angleDegrees) = 0;
    virtual void setAngle(uint8_t channel, float angleDegrees) = 0;
    virtual void setPulse(uint8_t channel, uint16_t pulseUs) = 0;

    virtual void update(uint32_t deltaMs) = 0;
};

}  // namespace eyesee
```

- [ ] **Step 3: Create `lib/MotionHardware/Pca9685ServoOutput.h`**

```cpp
#pragma once

#include <Adafruit_PWMServoDriver.h>

#include "IServoOutput.h"

namespace eyesee {

/** IServoOutput implementation driving a PCA9685 over I2C. */
class Pca9685ServoOutput : public IServoOutput {
public:
    explicit Pca9685ServoOutput(uint8_t i2cAddress = 0x40);

    /** Starts I2C and configures the PWM frequency. Call once from setup(). */
    void init();

    void write(const ServoOutput& output) override;
    void moveServo(uint8_t channel, float angleDegrees) override;
    void setAngle(uint8_t channel, float angleDegrees) override;
    void setPulse(uint8_t channel, uint16_t pulseUs) override;
    void update(uint32_t deltaMs) override;

private:
    Adafruit_PWMServoDriver driver_;

    static constexpr uint8_t kChannelLr = 0;
    static constexpr uint8_t kChannelUd = 1;
    static constexpr uint8_t kChannelTl = 2;
    static constexpr uint8_t kChannelBl = 3;
    static constexpr uint8_t kChannelTr = 4;
    static constexpr uint8_t kChannelBr = 5;
    static constexpr uint16_t kPwmFrequencyHz = 50;
};

}  // namespace eyesee
```

- [ ] **Step 4: Create `lib/MotionHardware/Pca9685ServoOutput.cpp`**

```cpp
#include "Pca9685ServoOutput.h"

namespace eyesee {

Pca9685ServoOutput::Pca9685ServoOutput(uint8_t i2cAddress) : driver_(i2cAddress) {}

void Pca9685ServoOutput::init() {
    driver_.begin();
    driver_.setPWMFreq(kPwmFrequencyHz);
}

void Pca9685ServoOutput::write(const ServoOutput& output) {
    setPulse(kChannelLr, output.lr);
    setPulse(kChannelUd, output.ud);
    setPulse(kChannelTl, output.tl);
    setPulse(kChannelBl, output.bl);
    setPulse(kChannelTr, output.tr);
    setPulse(kChannelBr, output.br);
}

void Pca9685ServoOutput::moveServo(uint8_t channel, float angleDegrees) {
    setAngle(channel, angleDegrees);
}

void Pca9685ServoOutput::setAngle(uint8_t channel, float angleDegrees) {
    constexpr float kMinPulseUs = 1000.0f;
    constexpr float kMaxPulseUs = 2000.0f;
    constexpr float kMaxAngleDegrees = 180.0f;

    const float clamped = angleDegrees < 0.0f
        ? 0.0f
        : (angleDegrees > kMaxAngleDegrees ? kMaxAngleDegrees : angleDegrees);
    const float pulseUs = kMinPulseUs + (clamped / kMaxAngleDegrees) * (kMaxPulseUs - kMinPulseUs);
    setPulse(channel, static_cast<uint16_t>(pulseUs));
}

void Pca9685ServoOutput::setPulse(uint8_t channel, uint16_t pulseUs) {
    constexpr uint32_t kTicksPerCycle = 4096;
    const uint32_t microsPerCycle = 1000000UL / kPwmFrequencyHz;
    const uint32_t ticks = (static_cast<uint32_t>(pulseUs) * kTicksPerCycle) / microsPerCycle;
    driver_.setPWM(channel, 0, static_cast<uint16_t>(ticks));
}

void Pca9685ServoOutput::update(uint32_t deltaMs) {
    (void)deltaMs;
    // TODO: reserved (see docs/architecture.md invariant 2 — MotionHardware owns no state).
}

}  // namespace eyesee
```

- [ ] **Step 5: Create `lib/MotionHardware/README.md`**

```markdown
# MotionHardware

**Purpose:** Abstract the physical actuator hardware behind `IServoOutput`,
so nothing above this module knows it's talking to a PCA9685.

**Responsibilities:** `Pca9685ServoOutput` converts calibrated pulse widths
(`ServoOutput`) into PWM writes over I2C.

**Planned features:** None — this module is intentionally "dumb": it writes
whatever it's told.

**Future work:** Alternative `IServoOutput` implementations — ESP32 LEDC PWM,
other PWM chips, Dynamixel/CAN servos, or a host-side/SDL simulator for
desktop development (see docs/architecture.md).
```

- [ ] **Step 6: Modify `src/main.cpp`**

```cpp
#include <Arduino.h>

#include "Logger.h"
#include "Pca9685ServoOutput.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
Pca9685ServoOutput servoOutput;
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
}

void loop() {
}
```

- [ ] **Step 7: Verify it builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS` (PlatformIO downloads the Adafruit PWM Servo Driver Library on first run)

- [ ] **Step 8: Commit**

```bash
git add lib/MotionHardware platformio.ini src/main.cpp
git commit -m "Add MotionHardware module (IServoOutput, Pca9685ServoOutput)"
```

---

### Task 6: EyeController module

**Files:**
- Create: `lib/EyeController/EyeController.h`
- Create: `lib/EyeController/EyeController.cpp`
- Create: `lib/EyeController/README.md`
- Create: `test/test_native/support/FakeServoOutput.h`
- Create: `test/test_native/test_eye_controller.cpp`
- Modify: `test/test_native/test_main.cpp` (append this task's tests — see Step 5b)
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: `eyesee::IServoOutput`, `eyesee::ServoOutput` (Task 5); `eyesee::CalibrationManager` (Task 4).
- Produces: `eyesee::Expression`, `eyesee::GazeTarget`, `eyesee::EyePose`, `eyesee::EyeController` (`applyPose`, `currentPose`, `look`, `blink`, `winkLeft`, `winkRight`, `sleep`, `wake`, `setExpression`, `setIdle`, `update`). Consumed by `Animation` (Task 7) and `Behavior` (Tasks 8–9, via `EyeCommand`'s use of `GazeTarget`/`Expression`).

- [ ] **Step 1: Create `lib/EyeController/EyeController.h`**

```cpp
#pragma once

#include <cstdint>

#include "CalibrationManager.h"
#include "IServoOutput.h"

namespace eyesee {

/** Named facial expressions. Pose blending per expression is future work (docs/ROADMAP.md v0.3). */
enum class Expression { Neutral, Happy, Curious, Sleepy, Angry, Surprised };

/** Command-layer gaze intent: where to look, and how (speed/blink/hold) — consumed by IAnimationEngine, not EyeController. */
struct GazeTarget {
    float x = 0.0f;
    float y = 0.0f;
    float speed = 1.0f;
    bool blinkOnArrival = false;
    bool hold = false;
};

/**
 * The complete instantaneous "desired shape" of both eyes — no timing, no
 * easing, just values. IAnimationEngine produces one of these per frame;
 * EyeController converts it to a ServoOutput.
 */
struct EyePose {
    float lookX = 0.0f;         // normalized -1..1
    float lookY = 0.0f;         // normalized -1..1
    float upperLeftLid = 1.0f;  // normalized 0 (closed) .. 1 (open)
    float lowerLeftLid = 1.0f;
    float upperRightLid = 1.0f;
    float lowerRightLid = 1.0f;
};

/**
 * Sole owner of eye motion state. Converts an EyePose into calibrated servo
 * pulses and writes them via IServoOutput. Owns no timed state — no delays,
 * no timers, no easing, no animation (see docs/architecture.md invariant 1).
 */
class EyeController {
public:
    EyeController(IServoOutput& output, CalibrationManager& calibration);

    /** The one true primitive: applies a pose immediately. */
    void applyPose(const EyePose& pose);
    EyePose currentPose() const;

    /** Convenience wrappers over applyPose(), each preserving prior pose fields they don't touch. */
    void look(float x, float y);
    void blink();
    void winkLeft();
    void winkRight();
    void sleep();
    void wake();
    void setExpression(Expression expression);
    void setIdle();

    /** Reserved; currently a no-op, since EyeController owns no timed state. */
    void update(uint32_t deltaMs);

private:
    IServoOutput& output_;
    CalibrationManager& calibration_;
    EyePose currentPose_;

    ServoOutput toServoOutput(const EyePose& pose) const;
};

}  // namespace eyesee
```

- [ ] **Step 2: Create `lib/EyeController/EyeController.cpp`**

```cpp
#include "EyeController.h"

namespace eyesee {

EyeController::EyeController(IServoOutput& output, CalibrationManager& calibration)
    : output_(output), calibration_(calibration), currentPose_() {}

void EyeController::applyPose(const EyePose& pose) {
    currentPose_ = pose;
    output_.write(toServoOutput(pose));
}

EyePose EyeController::currentPose() const {
    return currentPose_;
}

void EyeController::look(float x, float y) {
    EyePose pose = currentPose_;
    pose.lookX = x;
    pose.lookY = y;
    applyPose(pose);
}

void EyeController::blink() {
    EyePose pose = currentPose_;
    pose.upperLeftLid = 0.0f;
    pose.lowerLeftLid = 0.0f;
    pose.upperRightLid = 0.0f;
    pose.lowerRightLid = 0.0f;
    applyPose(pose);
    // TODO: reopening after a duration is IAnimationEngine's job (docs/ROADMAP.md v0.3) —
    // EyeController never owns time, so this call only closes the eyelids.
}

void EyeController::winkLeft() {
    EyePose pose = currentPose_;
    pose.upperLeftLid = 0.0f;
    pose.lowerLeftLid = 0.0f;
    applyPose(pose);
}

void EyeController::winkRight() {
    EyePose pose = currentPose_;
    pose.upperRightLid = 0.0f;
    pose.lowerRightLid = 0.0f;
    applyPose(pose);
}

void EyeController::sleep() {
    EyePose pose = currentPose_;
    pose.upperLeftLid = 0.0f;
    pose.lowerLeftLid = 0.0f;
    pose.upperRightLid = 0.0f;
    pose.lowerRightLid = 0.0f;
    applyPose(pose);
}

void EyeController::wake() {
    EyePose pose = currentPose_;
    pose.upperLeftLid = 1.0f;
    pose.lowerLeftLid = 1.0f;
    pose.upperRightLid = 1.0f;
    pose.lowerRightLid = 1.0f;
    applyPose(pose);
}

void EyeController::setExpression(Expression expression) {
    (void)expression;
    // TODO: per-expression pose blending (docs/ROADMAP.md v0.3). Every
    // expression resolves to the idle pose this pass.
    setIdle();
}

void EyeController::setIdle() {
    applyPose(EyePose{});
}

void EyeController::update(uint32_t deltaMs) {
    (void)deltaMs;
    // TODO: reserved. EyeController owns no timed state (docs/architecture.md invariant 1).
}

ServoOutput EyeController::toServoOutput(const EyePose& pose) const {
    (void)pose;
    // TODO: real pose -> pulse conversion (pulse scaling, invert, mirror,
    // mechanical offset) is out of scope this pass (docs/architecture.md).
    // Every channel reports its calibrated neutral pulse for now.
    const EyeConfig& config = calibration_.eyeConfig();
    ServoOutput out;
    out.lr = config.lr.neutralPulseUs;
    out.ud = config.ud.neutralPulseUs;
    out.tl = config.tl.neutralPulseUs;
    out.bl = config.bl.neutralPulseUs;
    out.tr = config.tr.neutralPulseUs;
    out.br = config.br.neutralPulseUs;
    return out;
}

}  // namespace eyesee
```

- [ ] **Step 3: Create `lib/EyeController/README.md`**

```markdown
# EyeController

**Purpose:** The sole class allowed to convert eye "intent" into servo
positions. No other module ever touches `IServoOutput` directly.

**Responsibilities:** Hold the current `EyePose`; convert poses to
`ServoOutput` via `CalibrationManager`; expose named convenience methods
(`look`, `blink`, `winkLeft`, `winkRight`, `sleep`, `wake`, `setExpression`,
`setIdle`).

**Planned features:** None — this class is deliberately "dumb" (see
docs/architecture.md invariant 1). All animation and behavior sit above it.

**Future work:** Real `toServoOutput()` calibration math (pulse scaling,
invert, mirror, mechanical offset) — docs/ROADMAP.md v0.2.
```

- [ ] **Step 4: Create `test/test_native/support/FakeServoOutput.h`**

```cpp
#pragma once

#include "IServoOutput.h"

class FakeServoOutput : public eyesee::IServoOutput {
public:
    eyesee::ServoOutput lastWrite{};
    int writeCallCount = 0;

    void write(const eyesee::ServoOutput& output) override {
        lastWrite = output;
        ++writeCallCount;
    }
    void moveServo(uint8_t, float) override {}
    void setAngle(uint8_t, float) override {}
    void setPulse(uint8_t, uint16_t) override {}
    void update(uint32_t) override {}
};
```

- [ ] **Step 5: Create `test/test_native/test_eye_controller.cpp`**

```cpp
#include <unity.h>

#include "CalibrationManager.h"
#include "EyeController.h"
#include "support/FakeServoOutput.h"

using eyesee::CalibrationManager;
using eyesee::EyeController;
using eyesee::EyePose;

void test_apply_pose_writes_neutral_pulses_and_calls_write_once() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.applyPose(EyePose{0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f});

    TEST_ASSERT_EQUAL_INT(1, output.writeCallCount);
    TEST_ASSERT_EQUAL_UINT16(calibration.eyeConfig().lr.neutralPulseUs, output.lastWrite.lr);
    TEST_ASSERT_EQUAL_UINT16(calibration.eyeConfig().ud.neutralPulseUs, output.lastWrite.ud);
}

void test_look_updates_gaze_but_preserves_eyelids() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.applyPose(EyePose{0.0f, 0.0f, 0.2f, 0.3f, 0.4f, 0.5f});
    controller.look(0.7f, -0.6f);

    const EyePose pose = controller.currentPose();
    TEST_ASSERT_EQUAL_FLOAT(0.7f, pose.lookX);
    TEST_ASSERT_EQUAL_FLOAT(-0.6f, pose.lookY);
    TEST_ASSERT_EQUAL_FLOAT(0.2f, pose.upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.3f, pose.lowerLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.4f, pose.upperRightLid);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, pose.lowerRightLid);
}

void test_blink_closes_all_four_lids() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.blink();

    const EyePose pose = controller.currentPose();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.lowerLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.upperRightLid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.lowerRightLid);
}

void test_wink_left_closes_only_left_lids() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.winkLeft();

    const EyePose pose = controller.currentPose();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.lowerLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, pose.upperRightLid);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, pose.lowerRightLid);
}

void test_set_idle_resets_to_default_pose() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.look(0.9f, 0.9f);
    controller.setIdle();

    const EyePose pose = controller.currentPose();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.lookX);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.lookY);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, pose.upperLeftLid);
}

```

No `main()` in this file — see the note on `test/test_native/test_main.cpp`
at the top of this task's file list. Every native test executable in
`test/test_native/` links as one PlatformIO test suite, so exactly one file
in the directory owns `main()`; individual `test_*.cpp` files contribute
only test functions.

- [ ] **Step 5b: Append this task's tests to `test/test_native/test_main.cpp`**

Add these `extern` declarations and `RUN_TEST` calls to the existing
`test/test_native/test_main.cpp` (created in Task 4's fix-up — see ledger).
Do not create a new `main()`; extend the existing one:

```cpp
extern void test_apply_pose_writes_neutral_pulses_and_calls_write_once();
extern void test_look_updates_gaze_but_preserves_eyelids();
extern void test_blink_closes_all_four_lids();
extern void test_wink_left_closes_only_left_lids();
extern void test_set_idle_resets_to_default_pose();
```

```cpp
    RUN_TEST(test_apply_pose_writes_neutral_pulses_and_calls_write_once);
    RUN_TEST(test_look_updates_gaze_but_preserves_eyelids);
    RUN_TEST(test_blink_closes_all_four_lids);
    RUN_TEST(test_wink_left_closes_only_left_lids);
    RUN_TEST(test_set_idle_resets_to_default_pose);
```

(add the `extern` block near the file's other `extern` declarations, and
each `RUN_TEST` line inside the existing `UNITY_BEGIN()`/`UNITY_END()` block,
before `return UNITY_END();`)

- [ ] **Step 6: Modify `src/main.cpp`**

```cpp
#include <Arduino.h>

#include "CalibrationManager.h"
#include "EyeController.h"
#include "Logger.h"
#include "Pca9685ServoOutput.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
Pca9685ServoOutput servoOutput;
CalibrationManager calibration;
EyeController eyeController(servoOutput, calibration);
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
    eyeController.setIdle();
}

void loop() {
}
```

- [ ] **Step 7: Run the native tests**

Run: `pio test -e native`
Expected: `10 Tests 0 Failures 0 Ignored` (5 from Tasks 3–4 + 5 new) — `PASSED`

- [ ] **Step 8: Verify `esp32dev` still builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS`

- [ ] **Step 9: Commit**

```bash
git add lib/EyeController test/test_native src/main.cpp
git commit -m "Add EyeController module"
```

---

### Task 7: Animation module

**Files:**
- Create: `lib/Animation/IAnimationEngine.h`
- Create: `lib/Animation/PassthroughAnimationEngine.h`
- Create: `lib/Animation/PassthroughAnimationEngine.cpp`
- Create: `lib/Animation/README.md`
- Create: `test/test_native/test_animation_engine.cpp`
- Modify: `test/test_native/test_main.cpp` (append this task's tests — see Step 5b)
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: `eyesee::EyeController`, `eyesee::GazeTarget`, `eyesee::Expression` (Task 6).
- Produces: `eyesee::IAnimationEngine` (`animateGaze`, `animateBlink`, `animateWinkLeft`, `animateWinkRight`, `animateSleep`, `animateWake`, `animateExpression`, `update`), `eyesee::PassthroughAnimationEngine`. Consumed by `BehaviorEngine` (Task 9).

- [ ] **Step 1: Create `lib/Animation/IAnimationEngine.h`**

```cpp
#pragma once

#include <cstdint>

#include "EyeController.h"

namespace eyesee {

/**
 * Converts command-layer intent (GazeTarget/Expression + duration) into a
 * sequence of EyePose updates over time. This is where "350ms, cubic
 * easing" lives — EyeController never sees timing.
 */
class IAnimationEngine {
public:
    virtual ~IAnimationEngine() = default;

    virtual void animateGaze(const GazeTarget& target) = 0;
    virtual void animateBlink(uint32_t durationMs) = 0;
    virtual void animateWinkLeft(uint32_t durationMs) = 0;
    virtual void animateWinkRight(uint32_t durationMs) = 0;
    virtual void animateSleep(uint32_t durationMs) = 0;
    virtual void animateWake(uint32_t durationMs) = 0;
    virtual void animateExpression(Expression expression, uint32_t durationMs) = 0;

    /** Advances any in-progress animation; calls EyeController::applyPose() as needed. */
    virtual void update(uint32_t deltaMs) = 0;
};

}  // namespace eyesee
```

- [ ] **Step 2: Create `lib/Animation/PassthroughAnimationEngine.h`**

```cpp
#pragma once

#include "IAnimationEngine.h"

namespace eyesee {

/**
 * IAnimationEngine implementation with no real interpolation — every
 * animate*() call applies its target pose immediately via EyeController.
 * The seam is real even though the easing/timing math isn't (docs/ROADMAP.md v0.2).
 */
class PassthroughAnimationEngine : public IAnimationEngine {
public:
    explicit PassthroughAnimationEngine(EyeController& eyeController);

    void animateGaze(const GazeTarget& target) override;
    void animateBlink(uint32_t durationMs) override;
    void animateWinkLeft(uint32_t durationMs) override;
    void animateWinkRight(uint32_t durationMs) override;
    void animateSleep(uint32_t durationMs) override;
    void animateWake(uint32_t durationMs) override;
    void animateExpression(Expression expression, uint32_t durationMs) override;
    void update(uint32_t deltaMs) override;

private:
    EyeController& eyeController_;
};

}  // namespace eyesee
```

- [ ] **Step 3: Create `lib/Animation/PassthroughAnimationEngine.cpp`**

```cpp
#include "PassthroughAnimationEngine.h"

namespace eyesee {

PassthroughAnimationEngine::PassthroughAnimationEngine(EyeController& eyeController)
    : eyeController_(eyeController) {}

void PassthroughAnimationEngine::animateGaze(const GazeTarget& target) {
    // TODO: honor target.speed/blinkOnArrival/hold via real interpolation (docs/ROADMAP.md v0.2).
    eyeController_.look(target.x, target.y);
}

void PassthroughAnimationEngine::animateBlink(uint32_t durationMs) {
    (void)durationMs;  // TODO: reopen after durationMs (docs/ROADMAP.md v0.3)
    eyeController_.blink();
}

void PassthroughAnimationEngine::animateWinkLeft(uint32_t durationMs) {
    (void)durationMs;
    eyeController_.winkLeft();
}

void PassthroughAnimationEngine::animateWinkRight(uint32_t durationMs) {
    (void)durationMs;
    eyeController_.winkRight();
}

void PassthroughAnimationEngine::animateSleep(uint32_t durationMs) {
    (void)durationMs;
    eyeController_.sleep();
}

void PassthroughAnimationEngine::animateWake(uint32_t durationMs) {
    (void)durationMs;
    eyeController_.wake();
}

void PassthroughAnimationEngine::animateExpression(Expression expression, uint32_t durationMs) {
    (void)durationMs;
    eyeController_.setExpression(expression);
}

void PassthroughAnimationEngine::update(uint32_t deltaMs) {
    (void)deltaMs;
    // TODO: reserved — nothing in-progress to advance until real interpolation exists.
}

}  // namespace eyesee
```

- [ ] **Step 4: Create `lib/Animation/README.md`**

```markdown
# Animation

**Purpose:** Own the "how" of eye motion — timing and easing — so
`EyeController` never has to.

**Responsibilities:** `IAnimationEngine` converts `GazeTarget`/`Expression` +
duration into `EyePose` updates over time.

**Planned features:** Linear, ease-in/out, cubic, and eventually spline
interpolation (docs/ROADMAP.md v0.2); animated blink/wink with real
open/close timing (v0.3).

**Future work:** Micro-saccades, idle scanning motion, expression blending.
```

- [ ] **Step 5: Create `test/test_native/test_animation_engine.cpp`**

```cpp
#include <unity.h>

#include "CalibrationManager.h"
#include "EyeController.h"
#include "PassthroughAnimationEngine.h"
#include "support/FakeServoOutput.h"

using eyesee::CalibrationManager;
using eyesee::EyeController;
using eyesee::GazeTarget;
using eyesee::PassthroughAnimationEngine;

void test_animate_gaze_applies_pose_immediately() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    PassthroughAnimationEngine animation(controller);

    GazeTarget target;
    target.x = 0.3f;
    target.y = -0.2f;
    animation.animateGaze(target);

    TEST_ASSERT_EQUAL_FLOAT(0.3f, controller.currentPose().lookX);
    TEST_ASSERT_EQUAL_FLOAT(-0.2f, controller.currentPose().lookY);
    TEST_ASSERT_EQUAL_INT(1, output.writeCallCount);
}

void test_animate_blink_closes_eyelids_immediately() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    PassthroughAnimationEngine animation(controller);

    animation.animateBlink(150);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperRightLid);
}

```

No `main()` in this file — same reason as Task 6 (one `main()` per
`test/test_native/` PlatformIO test suite).

- [ ] **Step 5b: Append this task's tests to `test/test_native/test_main.cpp`**

```cpp
extern void test_animate_gaze_applies_pose_immediately();
extern void test_animate_blink_closes_eyelids_immediately();
```

```cpp
    RUN_TEST(test_animate_gaze_applies_pose_immediately);
    RUN_TEST(test_animate_blink_closes_eyelids_immediately);
```

(same placement convention as Task 6's Step 5b)

- [ ] **Step 6: Modify `src/main.cpp`**

```cpp
#include <Arduino.h>

#include "CalibrationManager.h"
#include "EyeController.h"
#include "Logger.h"
#include "Pca9685ServoOutput.h"
#include "PassthroughAnimationEngine.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
Pca9685ServoOutput servoOutput;
CalibrationManager calibration;
EyeController eyeController(servoOutput, calibration);
PassthroughAnimationEngine animationEngine(eyeController);
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
    eyeController.setIdle();
}

void loop() {
}
```

- [ ] **Step 7: Run the native tests**

Run: `pio test -e native`
Expected: `12 Tests 0 Failures 0 Ignored` — `PASSED`

- [ ] **Step 8: Verify `esp32dev` still builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS`

- [ ] **Step 9: Commit**

```bash
git add lib/Animation test/test_native/test_animation_engine.cpp test/test_native/test_main.cpp src/main.cpp
git commit -m "Add Animation module"
```

---

### Task 8: Behavior data layer (EyeState, EyeCommand, CommandQueue)

**Files:**
- Create: `lib/Behavior/EyeState.h`
- Create: `lib/Behavior/EyeCommand.h`
- Create: `lib/Behavior/CommandQueue.h`
- Create: `lib/Behavior/CommandQueue.cpp`
- Create: `lib/Behavior/README.md`
- Create: `test/test_native/test_command_queue.cpp`
- Modify: `test/test_native/test_main.cpp` (append this task's tests — see Step 6b)

**Interfaces:**
- Consumes: `eyesee::GazeTarget`, `eyesee::Expression` (Task 6).
- Produces: `eyesee::EyeState`, `eyesee::CommandType`, `eyesee::CommandPriority`, `eyesee::EyeCommand`, `eyesee::CommandQueue` (`push`, `pop`, `clear`, `size`, `empty`, `full`, static `kCapacity`). Consumed by `BehaviorEngine` (Task 9).

- [ ] **Step 1: Create `lib/Behavior/EyeState.h`**

```cpp
#pragma once

namespace eyesee {

/** System-level behavior state, owned by BehaviorEngine. */
enum class EyeState {
    Startup,
    Calibration,
    Manual,
    Idle,
    Tracking,
    Sleeping,
    Disabled,
    Error
};

}  // namespace eyesee
```

- [ ] **Step 2: Create `lib/Behavior/EyeCommand.h`**

```cpp
#pragma once

#include <cstdint>

#include "EyeController.h"

namespace eyesee {

enum class CommandType { Look, Blink, WinkLeft, WinkRight, Sleep, Wake, SetExpression };
enum class CommandPriority { Low, Normal, High };

/**
 * What every control-plane input (Web UI, REST, WebSocket, Bluetooth,
 * Serial) pushes into CommandQueue. BehaviorEngine is the only consumer.
 */
struct EyeCommand {
    CommandType type = CommandType::Look;
    CommandPriority priority = CommandPriority::Normal;
    uint32_t durationMs = 0;
    GazeTarget gazeTarget{};   // valid when type == Look
    Expression expression = Expression::Neutral;  // valid when type == SetExpression
};

}  // namespace eyesee
```

- [ ] **Step 3: Create `lib/Behavior/CommandQueue.h`**

```cpp
#pragma once

#include <array>
#include <cstddef>

#include "EyeCommand.h"

namespace eyesee {

/** Fixed-capacity FIFO ring buffer of EyeCommand. No heap allocation. */
class CommandQueue {
public:
    static constexpr size_t kCapacity = 16;

    bool push(const EyeCommand& command);
    bool pop(EyeCommand& outCommand);
    void clear();

    size_t size() const;
    bool empty() const;
    bool full() const;

private:
    std::array<EyeCommand, kCapacity> buffer_{};
    size_t head_ = 0;
    size_t count_ = 0;
};

}  // namespace eyesee
```

- [ ] **Step 4: Create `lib/Behavior/CommandQueue.cpp`**

```cpp
#include "CommandQueue.h"

namespace eyesee {

bool CommandQueue::push(const EyeCommand& command) {
    if (full()) {
        return false;
    }
    const size_t tail = (head_ + count_) % kCapacity;
    buffer_[tail] = command;
    ++count_;
    return true;
}

bool CommandQueue::pop(EyeCommand& outCommand) {
    if (empty()) {
        return false;
    }
    outCommand = buffer_[head_];
    head_ = (head_ + 1) % kCapacity;
    --count_;
    return true;
}

void CommandQueue::clear() {
    head_ = 0;
    count_ = 0;
}

size_t CommandQueue::size() const {
    return count_;
}

bool CommandQueue::empty() const {
    return count_ == 0;
}

bool CommandQueue::full() const {
    return count_ == kCapacity;
}

}  // namespace eyesee
```

- [ ] **Step 5: Create `lib/Behavior/README.md`**

```markdown
# Behavior

**Purpose:** The single arbiter between the outside world and the eye. Every
input source pushes an `EyeCommand`; nothing else calls `IAnimationEngine`
or `EyeController` directly, so inputs never fight each other.

**Responsibilities (this file):** `EyeState` (system-level state),
`EyeCommand`/`CommandQueue` (fixed-capacity FIFO command buffer).

**Planned features:** `IBehavior`/`BehaviorEngine` — see below, added in the
next task.

**Future work:** Command priority handling (currently stored but not acted
on), `EyeState`-driven behavior switching (docs/ROADMAP.md v0.3).
```

- [ ] **Step 6: Create `test/test_native/test_command_queue.cpp`**

```cpp
#include <unity.h>

#include "CommandQueue.h"

using eyesee::CommandQueue;
using eyesee::CommandType;
using eyesee::EyeCommand;

void test_push_pop_preserves_fifo_order() {
    CommandQueue queue;
    EyeCommand blink;
    blink.type = CommandType::Blink;
    EyeCommand wake;
    wake.type = CommandType::Wake;

    TEST_ASSERT_TRUE(queue.push(blink));
    TEST_ASSERT_TRUE(queue.push(wake));

    EyeCommand out;
    TEST_ASSERT_TRUE(queue.pop(out));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Blink), static_cast<int>(out.type));
    TEST_ASSERT_TRUE(queue.pop(out));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Wake), static_cast<int>(out.type));
    TEST_ASSERT_FALSE(queue.pop(out));
}

void test_push_fails_when_full() {
    CommandQueue queue;
    EyeCommand command;
    for (size_t i = 0; i < CommandQueue::kCapacity; ++i) {
        TEST_ASSERT_TRUE(queue.push(command));
    }
    TEST_ASSERT_TRUE(queue.full());
    TEST_ASSERT_FALSE(queue.push(command));
}

void test_clear_empties_queue() {
    CommandQueue queue;
    EyeCommand command;
    queue.push(command);
    queue.clear();
    TEST_ASSERT_TRUE(queue.empty());
    TEST_ASSERT_EQUAL_UINT32(0, static_cast<uint32_t>(queue.size()));
}

// No main() in this file — same reason as Task 6 (one main() per
// test/test_native/ PlatformIO test suite; see Step 6b below).
```

- [ ] **Step 6b: Append this task's tests to `test/test_native/test_main.cpp`**

```cpp
extern void test_push_pop_preserves_fifo_order();
extern void test_push_fails_when_full();
extern void test_clear_empties_queue();
```

```cpp
    RUN_TEST(test_push_pop_preserves_fifo_order);
    RUN_TEST(test_push_fails_when_full);
    RUN_TEST(test_clear_empties_queue);
```

(same placement convention as Task 6's Step 5b)

- [ ] **Step 7: Run the native tests**

Run: `pio test -e native`
Expected: `15 Tests 0 Failures 0 Ignored` — `PASSED`

- [ ] **Step 8: Commit**

```bash
git add lib/Behavior test/test_native/test_command_queue.cpp test/test_native/test_main.cpp
git commit -m "Add Behavior data layer (EyeState, EyeCommand, CommandQueue)"
```

---

### Task 9: Behavior engine layer (IBehavior, IdleBehaviorStub, BehaviorEngine)

**Files:**
- Create: `lib/Behavior/IBehavior.h`
- Create: `lib/Behavior/IdleBehaviorStub.h`
- Create: `lib/Behavior/IdleBehaviorStub.cpp`
- Create: `lib/Behavior/IBehaviorEngine.h`
- Create: `lib/Behavior/BehaviorEngine.h`
- Create: `lib/Behavior/BehaviorEngine.cpp`
- Create: `test/test_native/support/FakeAnimationEngine.h`
- Create: `test/test_native/test_behavior_engine.cpp`
- Modify: `test/test_native/test_main.cpp` (append this task's tests — see Step 9b)
- Modify: `lib/Behavior/README.md`
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: `eyesee::EyeState`, `eyesee::CommandQueue`, `eyesee::EyeCommand`, `eyesee::CommandType` (Task 8); `eyesee::IAnimationEngine` (Task 7).
- Produces: `eyesee::IBehavior`, `eyesee::IdleBehaviorStub`, `eyesee::IBehaviorEngine`, `eyesee::BehaviorEngine` (`setState`, `state`, `update`). Consumed by `src/main.cpp` only.

- [ ] **Step 1: Create `lib/Behavior/IBehavior.h`**

```cpp
#pragma once

#include <cstdint>

#include "EyeState.h"

namespace eyesee {

class IAnimationEngine;  // forward declaration — only a reference is needed here

/** One pluggable behavior strategy (Idle, Tracking, Curious, Random, Sleep...). */
class IBehavior {
public:
    virtual ~IBehavior() = default;
    virtual void update(uint32_t deltaMs, IAnimationEngine& animation) = 0;
    virtual EyeState state() const = 0;
};

}  // namespace eyesee
```

- [ ] **Step 2: Create `lib/Behavior/IdleBehaviorStub.h`**

```cpp
#pragma once

#include "IBehavior.h"

namespace eyesee {

/**
 * The only IBehavior implementation this pass. TrackingBehavior,
 * CuriousBehavior, RandomBehavior, SleepBehavior are planned
 * (docs/ROADMAP.md v0.3) but not scaffolded as empty files yet.
 */
class IdleBehaviorStub : public IBehavior {
public:
    void update(uint32_t deltaMs, IAnimationEngine& animation) override;
    EyeState state() const override;
};

}  // namespace eyesee
```

- [ ] **Step 3: Create `lib/Behavior/IdleBehaviorStub.cpp`**

```cpp
#include "IdleBehaviorStub.h"

#include "IAnimationEngine.h"

namespace eyesee {

void IdleBehaviorStub::update(uint32_t deltaMs, IAnimationEngine& animation) {
    (void)deltaMs;
    (void)animation;
    // TODO: idle scanning / micro-saccades (docs/ROADMAP.md v0.3). No-op this pass.
}

EyeState IdleBehaviorStub::state() const {
    return EyeState::Idle;
}

}  // namespace eyesee
```

- [ ] **Step 4: Create `lib/Behavior/IBehaviorEngine.h`**

```cpp
#pragma once

#include <cstdint>

#include "EyeState.h"

namespace eyesee {

class IBehaviorEngine {
public:
    virtual ~IBehaviorEngine() = default;
    virtual void setState(EyeState state) = 0;
    virtual EyeState state() const = 0;
    virtual void update(uint32_t deltaMs) = 0;
};

}  // namespace eyesee
```

- [ ] **Step 5: Create `lib/Behavior/BehaviorEngine.h`**

```cpp
#pragma once

#include "CommandQueue.h"
#include "IAnimationEngine.h"
#include "IBehavior.h"
#include "IBehaviorEngine.h"

namespace eyesee {

/**
 * Drains CommandQueue every frame, dispatching each EyeCommand to
 * IAnimationEngine (never to EyeController directly), then delegates to
 * whichever IBehavior is active for the current EyeState.
 */
class BehaviorEngine : public IBehaviorEngine {
public:
    BehaviorEngine(IAnimationEngine& animation, CommandQueue& commandQueue, IBehavior& defaultBehavior);

    void setState(EyeState state) override;
    EyeState state() const override;
    void update(uint32_t deltaMs) override;

private:
    IAnimationEngine& animation_;
    CommandQueue& commandQueue_;
    IBehavior& activeBehavior_;
    EyeState state_;

    void dispatch(const EyeCommand& command);
};

}  // namespace eyesee
```

- [ ] **Step 6: Create `lib/Behavior/BehaviorEngine.cpp`**

```cpp
#include "BehaviorEngine.h"

namespace eyesee {

BehaviorEngine::BehaviorEngine(IAnimationEngine& animation, CommandQueue& commandQueue,
                                IBehavior& defaultBehavior)
    : animation_(animation),
      commandQueue_(commandQueue),
      activeBehavior_(defaultBehavior),
      state_(EyeState::Startup) {}

void BehaviorEngine::setState(EyeState state) {
    state_ = state;
    // TODO: swap activeBehavior_ based on state once more IBehavior
    // implementations exist (docs/ROADMAP.md v0.3). Single behavior this pass.
}

EyeState BehaviorEngine::state() const {
    return state_;
}

void BehaviorEngine::update(uint32_t deltaMs) {
    EyeCommand command;
    while (commandQueue_.pop(command)) {
        dispatch(command);
    }
    activeBehavior_.update(deltaMs, animation_);
}

void BehaviorEngine::dispatch(const EyeCommand& command) {
    switch (command.type) {
        case CommandType::Look:
            animation_.animateGaze(command.gazeTarget);
            break;
        case CommandType::Blink:
            animation_.animateBlink(command.durationMs);
            break;
        case CommandType::WinkLeft:
            animation_.animateWinkLeft(command.durationMs);
            break;
        case CommandType::WinkRight:
            animation_.animateWinkRight(command.durationMs);
            break;
        case CommandType::Sleep:
            animation_.animateSleep(command.durationMs);
            break;
        case CommandType::Wake:
            animation_.animateWake(command.durationMs);
            break;
        case CommandType::SetExpression:
            animation_.animateExpression(command.expression, command.durationMs);
            break;
    }
}

}  // namespace eyesee
```

- [ ] **Step 7: Update `lib/Behavior/README.md`** — append:

```markdown

**IBehaviorEngine / BehaviorEngine:** Drains `CommandQueue`, dispatches each
`EyeCommand` to `IAnimationEngine`, then delegates per-frame `update` to the
active `IBehavior` (only `IdleBehaviorStub` exists this pass).
```

- [ ] **Step 8: Create `test/test_native/support/FakeAnimationEngine.h`**

```cpp
#pragma once

#include "IAnimationEngine.h"

class FakeAnimationEngine : public eyesee::IAnimationEngine {
public:
    int animateGazeCallCount = 0;
    eyesee::GazeTarget lastGazeTarget{};
    int animateBlinkCallCount = 0;
    int animateWinkLeftCallCount = 0;
    int animateWinkRightCallCount = 0;
    int animateSleepCallCount = 0;
    int animateWakeCallCount = 0;
    int animateExpressionCallCount = 0;
    eyesee::Expression lastExpression = eyesee::Expression::Neutral;

    void animateGaze(const eyesee::GazeTarget& target) override {
        lastGazeTarget = target;
        ++animateGazeCallCount;
    }
    void animateBlink(uint32_t) override { ++animateBlinkCallCount; }
    void animateWinkLeft(uint32_t) override { ++animateWinkLeftCallCount; }
    void animateWinkRight(uint32_t) override { ++animateWinkRightCallCount; }
    void animateSleep(uint32_t) override { ++animateSleepCallCount; }
    void animateWake(uint32_t) override { ++animateWakeCallCount; }
    void animateExpression(eyesee::Expression expression, uint32_t) override {
        lastExpression = expression;
        ++animateExpressionCallCount;
    }
    void update(uint32_t) override {}
};
```

- [ ] **Step 9: Create `test/test_native/test_behavior_engine.cpp`**

```cpp
#include <unity.h>

#include "BehaviorEngine.h"
#include "IdleBehaviorStub.h"
#include "support/FakeAnimationEngine.h"

using namespace eyesee;

void test_update_drains_queue_and_dispatches_look_command() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    IdleBehaviorStub idleBehavior;
    BehaviorEngine engine(animation, queue, idleBehavior);

    EyeCommand lookCommand;
    lookCommand.type = CommandType::Look;
    lookCommand.gazeTarget.x = 0.4f;
    lookCommand.gazeTarget.y = -0.2f;
    queue.push(lookCommand);

    engine.update(16);

    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);
    TEST_ASSERT_EQUAL_FLOAT(0.4f, animation.lastGazeTarget.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.2f, animation.lastGazeTarget.y);
    TEST_ASSERT_TRUE(queue.empty());
}

void test_update_dispatches_blink_command() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    IdleBehaviorStub idleBehavior;
    BehaviorEngine engine(animation, queue, idleBehavior);

    EyeCommand blinkCommand;
    blinkCommand.type = CommandType::Blink;
    blinkCommand.durationMs = 150;
    queue.push(blinkCommand);

    engine.update(16);

    TEST_ASSERT_EQUAL_INT(1, animation.animateBlinkCallCount);
}

void test_update_dispatches_all_remaining_command_types() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    IdleBehaviorStub idleBehavior;
    BehaviorEngine engine(animation, queue, idleBehavior);

    EyeCommand winkLeft;
    winkLeft.type = CommandType::WinkLeft;
    EyeCommand winkRight;
    winkRight.type = CommandType::WinkRight;
    EyeCommand sleepCmd;
    sleepCmd.type = CommandType::Sleep;
    EyeCommand wakeCmd;
    wakeCmd.type = CommandType::Wake;
    EyeCommand expressionCmd;
    expressionCmd.type = CommandType::SetExpression;
    expressionCmd.expression = Expression::Happy;

    queue.push(winkLeft);
    queue.push(winkRight);
    queue.push(sleepCmd);
    queue.push(wakeCmd);
    queue.push(expressionCmd);

    engine.update(16);

    TEST_ASSERT_EQUAL_INT(1, animation.animateWinkLeftCallCount);
    TEST_ASSERT_EQUAL_INT(1, animation.animateWinkRightCallCount);
    TEST_ASSERT_EQUAL_INT(1, animation.animateSleepCallCount);
    TEST_ASSERT_EQUAL_INT(1, animation.animateWakeCallCount);
    TEST_ASSERT_EQUAL_INT(1, animation.animateExpressionCallCount);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(Expression::Happy), static_cast<int>(animation.lastExpression));
}

void test_set_state_updates_reported_state() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    IdleBehaviorStub idleBehavior;
    BehaviorEngine engine(animation, queue, idleBehavior);

    engine.setState(EyeState::Manual);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeState::Manual), static_cast<int>(engine.state()));
}

// No main() in this file — same reason as Task 6 (one main() per
// test/test_native/ PlatformIO test suite; see Step 9b below).
```

- [ ] **Step 9b: Append this task's tests to `test/test_native/test_main.cpp`**

```cpp
extern void test_update_drains_queue_and_dispatches_look_command();
extern void test_update_dispatches_blink_command();
extern void test_update_dispatches_all_remaining_command_types();
extern void test_set_state_updates_reported_state();
```

```cpp
    RUN_TEST(test_update_drains_queue_and_dispatches_look_command);
    RUN_TEST(test_update_dispatches_blink_command);
    RUN_TEST(test_update_dispatches_all_remaining_command_types);
    RUN_TEST(test_set_state_updates_reported_state);
```

(same placement convention as Task 6's Step 5b)

- [ ] **Step 10: Modify `src/main.cpp`**

```cpp
#include <Arduino.h>

#include "BehaviorEngine.h"
#include "CalibrationManager.h"
#include "CommandQueue.h"
#include "EyeController.h"
#include "IdleBehaviorStub.h"
#include "Logger.h"
#include "Pca9685ServoOutput.h"
#include "PassthroughAnimationEngine.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
Pca9685ServoOutput servoOutput;
CalibrationManager calibration;
EyeController eyeController(servoOutput, calibration);
PassthroughAnimationEngine animationEngine(eyeController);
CommandQueue commandQueue;
IdleBehaviorStub idleBehavior;
BehaviorEngine behaviorEngine(animationEngine, commandQueue, idleBehavior);
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
    eyeController.setIdle();
    behaviorEngine.setState(EyeState::Startup);
}

void loop() {
}
```

- [ ] **Step 11: Run the native tests**

Run: `pio test -e native`
Expected: `19 Tests 0 Failures 0 Ignored` — `PASSED`

- [ ] **Step 12: Verify `esp32dev` still builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS`

- [ ] **Step 13: Commit**

```bash
git add lib/Behavior test/test_native src/main.cpp
git commit -m "Add Behavior engine layer (IBehavior, IdleBehaviorStub, BehaviorEngine)"
```

---

### Task 10: Storage module

**Files:**
- Create: `lib/Storage/IStorage.h`
- Create: `lib/Storage/PreferencesStore.h`
- Create: `lib/Storage/PreferencesStore.cpp`
- Create: `lib/Storage/README.md`
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: ESP32 `Preferences.h` (built into the Arduino core — no extra `lib_dep`).
- Produces: `eyesee::IStorage` (`begin`, `getUInt16`, `putUInt16`, `end`), `eyesee::PreferencesStore`. Not yet wired into `CalibrationManager` (persistence is `docs/ROADMAP.md` v0.5) — instantiated standalone this pass.

- [ ] **Step 1: Create `lib/Storage/IStorage.h`**

```cpp
#pragma once

#include <cstdint>

namespace eyesee {

/** Swappable key/value persistence backend. */
class IStorage {
public:
    virtual ~IStorage() = default;
    virtual bool begin(const char* namespaceName) = 0;
    virtual bool getUInt16(const char* key, uint16_t& outValue) const = 0;
    virtual bool putUInt16(const char* key, uint16_t value) = 0;
    virtual void end() = 0;
};

}  // namespace eyesee
```

- [ ] **Step 2: Create `lib/Storage/PreferencesStore.h`**

```cpp
#pragma once

#include <Preferences.h>

#include "IStorage.h"

namespace eyesee {

/** IStorage implementation wrapping the ESP32 Preferences API (flash-backed key/value store). */
class PreferencesStore : public IStorage {
public:
    bool begin(const char* namespaceName) override;
    bool getUInt16(const char* key, uint16_t& outValue) const override;
    bool putUInt16(const char* key, uint16_t value) override;
    void end() override;

private:
    mutable Preferences preferences_;
};

}  // namespace eyesee
```

- [ ] **Step 3: Create `lib/Storage/PreferencesStore.cpp`**

```cpp
#include "PreferencesStore.h"

namespace eyesee {

bool PreferencesStore::begin(const char* namespaceName) {
    return preferences_.begin(namespaceName, false);
}

bool PreferencesStore::getUInt16(const char* key, uint16_t& outValue) const {
    if (!preferences_.isKey(key)) {
        return false;
    }
    outValue = preferences_.getUShort(key);
    return true;
}

bool PreferencesStore::putUInt16(const char* key, uint16_t value) {
    return preferences_.putUShort(key, value) > 0;
}

void PreferencesStore::end() {
    preferences_.end();
}

}  // namespace eyesee
```

- [ ] **Step 4: Create `lib/Storage/README.md`**

```markdown
# Storage

**Purpose:** Swappable persistence backend behind `IStorage`.

**Responsibilities:** `PreferencesStore` wraps the ESP32 `Preferences` API
(flash-backed key/value store) for `uint16_t` values (pulse widths).

**Planned features:** None yet — not wired into `CalibrationManager`.

**Future work:** `CalibrationManager` persistence (save/load `EyeConfig` on
boot and on calibration change) — docs/ROADMAP.md v0.5.
```

- [ ] **Step 5: Modify `src/main.cpp`**

```cpp
#include <Arduino.h>

#include "BehaviorEngine.h"
#include "CalibrationManager.h"
#include "CommandQueue.h"
#include "EyeController.h"
#include "IdleBehaviorStub.h"
#include "Logger.h"
#include "Pca9685ServoOutput.h"
#include "PassthroughAnimationEngine.h"
#include "PreferencesStore.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
Pca9685ServoOutput servoOutput;
CalibrationManager calibration;
EyeController eyeController(servoOutput, calibration);
PassthroughAnimationEngine animationEngine(eyeController);
CommandQueue commandQueue;
IdleBehaviorStub idleBehavior;
BehaviorEngine behaviorEngine(animationEngine, commandQueue, idleBehavior);
PreferencesStore preferencesStore;
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
    preferencesStore.begin("eyesee");
    eyeController.setIdle();
    behaviorEngine.setState(EyeState::Startup);
}

void loop() {
}
```

- [ ] **Step 6: Verify it builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS`

- [ ] **Step 7: Commit**

```bash
git add lib/Storage src/main.cpp
git commit -m "Add Storage module (IStorage, PreferencesStore)"
```

---

### Task 11: Networking module

**Files:**
- Create: `lib/Networking/WebServer.h`
- Create: `lib/Networking/WebServer.cpp`
- Create: `lib/Networking/RestApi.h`
- Create: `lib/Networking/RestApi.cpp`
- Create: `lib/Networking/WebSocketServer.h`
- Create: `lib/Networking/WebSocketServer.cpp`
- Create: `lib/Networking/README.md`
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: nothing (no networking library added this pass).
- Produces: `eyesee::WebServer`, `eyesee::RestApi`, `eyesee::WebSocketServer`, each with `begin()`/`update(uint32_t deltaMs)`.

- [ ] **Step 1: Create `lib/Networking/WebServer.h`**

```cpp
#pragma once

#include <cstdint>

namespace eyesee {

/**
 * Placeholder for the Web UI static asset server. No functionality this
 * pass — see docs/ROADMAP.md v0.4.
 */
class WebServer {
public:
    void begin();
    void update(uint32_t deltaMs);
};

}  // namespace eyesee
```

- [ ] **Step 2: Create `lib/Networking/WebServer.cpp`**

```cpp
#include "WebServer.h"

namespace eyesee {

void WebServer::begin() {
    // TODO: start ESPAsyncWebServer, serve data/ web assets (docs/ROADMAP.md v0.4).
}

void WebServer::update(uint32_t deltaMs) {
    (void)deltaMs;
}

}  // namespace eyesee
```

- [ ] **Step 3: Create `lib/Networking/RestApi.h`**

```cpp
#pragma once

#include <cstdint>

namespace eyesee {

/**
 * Placeholder REST API. Planned versioned routes (docs/ROADMAP.md v0.4),
 * each pushing an EyeCommand into the shared CommandQueue:
 *   GET  /api/v1/status
 *   POST /api/v1/look
 *   POST /api/v1/blink
 *   POST /api/v1/expression
 *   POST /api/v1/config
 *   GET  /api/v1/config
 */
class RestApi {
public:
    void begin();
    void update(uint32_t deltaMs);
};

}  // namespace eyesee
```

- [ ] **Step 4: Create `lib/Networking/RestApi.cpp`**

```cpp
#include "RestApi.h"

namespace eyesee {

void RestApi::begin() {
    // TODO: register /api/v1/* routes (docs/ROADMAP.md v0.4).
}

void RestApi::update(uint32_t deltaMs) {
    (void)deltaMs;
}

}  // namespace eyesee
```

- [ ] **Step 5: Create `lib/Networking/WebSocketServer.h`**

```cpp
#pragma once

#include <cstdint>

namespace eyesee {

/**
 * Placeholder real-time eye-state broadcast. Target update rate 30-60Hz
 * (docs/ROADMAP.md v0.4). No functionality this pass.
 */
class WebSocketServer {
public:
    void begin();
    void update(uint32_t deltaMs);
};

}  // namespace eyesee
```

- [ ] **Step 6: Create `lib/Networking/WebSocketServer.cpp`**

```cpp
#include "WebSocketServer.h"

namespace eyesee {

void WebSocketServer::begin() {
    // TODO: start a WebSocket endpoint, push EyeController state at 30-60Hz (docs/ROADMAP.md v0.4).
}

void WebSocketServer::update(uint32_t deltaMs) {
    (void)deltaMs;
}

}  // namespace eyesee
```

- [ ] **Step 7: Create `lib/Networking/README.md`**

```markdown
# Networking

**Purpose:** Host the Web UI, the REST API, and a real-time WebSocket feed —
none implemented yet.

**Responsibilities:** `WebServer` (static assets), `RestApi` (versioned
`/api/v1/*` routes), `WebSocketServer` (30-60Hz state broadcast). Every
handler, once implemented, pushes `EyeCommand`s into the shared
`CommandQueue` rather than touching `EyeController`/`IAnimationEngine`
directly.

**Planned features:** See `docs/ROADMAP.md` v0.4.

**Future work:** No networking library is declared in `platformio.ini` yet
— adding one (e.g. ESPAsyncWebServer) is the first step of implementing
this module for real.
```

- [ ] **Step 8: Modify `src/main.cpp`**

```cpp
#include <Arduino.h>

#include "BehaviorEngine.h"
#include "CalibrationManager.h"
#include "CommandQueue.h"
#include "EyeController.h"
#include "IdleBehaviorStub.h"
#include "Logger.h"
#include "Pca9685ServoOutput.h"
#include "PassthroughAnimationEngine.h"
#include "PreferencesStore.h"
#include "RestApi.h"
#include "WebServer.h"
#include "WebSocketServer.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
Pca9685ServoOutput servoOutput;
CalibrationManager calibration;
EyeController eyeController(servoOutput, calibration);
PassthroughAnimationEngine animationEngine(eyeController);
CommandQueue commandQueue;
IdleBehaviorStub idleBehavior;
BehaviorEngine behaviorEngine(animationEngine, commandQueue, idleBehavior);
PreferencesStore preferencesStore;
WebServer webServer;
RestApi restApi;
WebSocketServer webSocketServer;
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
    preferencesStore.begin("eyesee");
    webServer.begin();
    restApi.begin();
    webSocketServer.begin();
    eyeController.setIdle();
    behaviorEngine.setState(EyeState::Startup);
}

void loop() {
}
```

- [ ] **Step 9: Verify it builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS`

- [ ] **Step 10: Commit**

```bash
git add lib/Networking src/main.cpp
git commit -m "Add Networking module (WebServer, RestApi, WebSocketServer placeholders)"
```

---

### Task 12: OTA module

**Files:**
- Create: `lib/OTA/OtaManager.h`
- Create: `lib/OTA/OtaManager.cpp`
- Create: `lib/OTA/README.md`
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: `eyesee::OtaManager` with `begin()`/`update(uint32_t deltaMs)`.

- [ ] **Step 1: Create `lib/OTA/OtaManager.h`**

```cpp
#pragma once

#include <cstdint>

namespace eyesee {

/** Placeholder OTA update manager. No functionality this pass — see docs/ROADMAP.md v0.5. */
class OtaManager {
public:
    void begin();
    void update(uint32_t deltaMs);
};

}  // namespace eyesee
```

- [ ] **Step 2: Create `lib/OTA/OtaManager.cpp`**

```cpp
#include "OtaManager.h"

namespace eyesee {

void OtaManager::begin() {
    // TODO: wrap ArduinoOTA (docs/ROADMAP.md v0.5).
}

void OtaManager::update(uint32_t deltaMs) {
    (void)deltaMs;
}

}  // namespace eyesee
```

- [ ] **Step 3: Create `lib/OTA/README.md`**

```markdown
# OTA

**Purpose:** Over-the-air firmware updates.

**Responsibilities:** `OtaManager` will wrap `ArduinoOTA` once implemented.

**Planned features:** See `docs/ROADMAP.md` v0.5.

**Future work:** Everything — this is a placeholder only.
```

- [ ] **Step 4: Modify `src/main.cpp`**

```cpp
#include <Arduino.h>

#include "BehaviorEngine.h"
#include "CalibrationManager.h"
#include "CommandQueue.h"
#include "EyeController.h"
#include "IdleBehaviorStub.h"
#include "Logger.h"
#include "OtaManager.h"
#include "Pca9685ServoOutput.h"
#include "PassthroughAnimationEngine.h"
#include "PreferencesStore.h"
#include "RestApi.h"
#include "WebServer.h"
#include "WebSocketServer.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
Pca9685ServoOutput servoOutput;
CalibrationManager calibration;
EyeController eyeController(servoOutput, calibration);
PassthroughAnimationEngine animationEngine(eyeController);
CommandQueue commandQueue;
IdleBehaviorStub idleBehavior;
BehaviorEngine behaviorEngine(animationEngine, commandQueue, idleBehavior);
PreferencesStore preferencesStore;
WebServer webServer;
RestApi restApi;
WebSocketServer webSocketServer;
OtaManager otaManager;
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
    preferencesStore.begin("eyesee");
    webServer.begin();
    restApi.begin();
    webSocketServer.begin();
    otaManager.begin();
    eyeController.setIdle();
    behaviorEngine.setState(EyeState::Startup);
}

void loop() {
}
```

- [ ] **Step 5: Verify it builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS`

- [ ] **Step 6: Commit**

```bash
git add lib/OTA src/main.cpp
git commit -m "Add OTA module placeholder"
```

---

### Task 13: Final frame-loop wiring

**Files:**
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: every module produced by Tasks 2–12.
- Produces: the complete, final composition root — the target end state described in `docs/superpowers/specs/2026-07-25-firmware-bootstrap-design.md`.

- [ ] **Step 1: Replace `src/main.cpp` with the final version**

```cpp
#include <Arduino.h>

#include "BehaviorEngine.h"
#include "CalibrationManager.h"
#include "CommandQueue.h"
#include "EyeController.h"
#include "IdleBehaviorStub.h"
#include "Logger.h"
#include "OtaManager.h"
#include "Pca9685ServoOutput.h"
#include "PassthroughAnimationEngine.h"
#include "PreferencesStore.h"
#include "RestApi.h"
#include "WebServer.h"
#include "WebSocketServer.h"

using namespace eyesee;

// This file is the firmware's composition root — the one place file-scope
// objects are acceptable (see docs/architecture.md). Everything below is
// wiring: no business logic lives here.
namespace {

constexpr const char* kLogTag = "main";

Pca9685ServoOutput servoOutput;
CalibrationManager calibration;
EyeController eyeController(servoOutput, calibration);
PassthroughAnimationEngine animationEngine(eyeController);
CommandQueue commandQueue;
IdleBehaviorStub idleBehavior;
BehaviorEngine behaviorEngine(animationEngine, commandQueue, idleBehavior);

PreferencesStore preferencesStore;
WebServer webServer;
RestApi restApi;
WebSocketServer webSocketServer;
OtaManager otaManager;

uint32_t lastFrameMillis = 0;

uint32_t tickDeltaMs() {
    const uint32_t nowMillis = millis();
    const uint32_t deltaMs = nowMillis - lastFrameMillis;  // wraps correctly via unsigned arithmetic
    lastFrameMillis = nowMillis;
    return deltaMs;
}

}  // namespace

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");

    servoOutput.init();
    preferencesStore.begin("eyesee");
    webServer.begin();
    restApi.begin();
    webSocketServer.begin();
    otaManager.begin();

    behaviorEngine.setState(EyeState::Startup);
    eyeController.setIdle();
    behaviorEngine.setState(EyeState::Idle);

    lastFrameMillis = millis();
    Logger::info(kLogTag, "EyeSee firmware ready");
}

void loop() {
    const uint32_t deltaMs = tickDeltaMs();

    behaviorEngine.update(deltaMs);
    animationEngine.update(deltaMs);
    eyeController.update(deltaMs);
    servoOutput.update(deltaMs);

    webServer.update(deltaMs);
    restApi.update(deltaMs);
    webSocketServer.update(deltaMs);
    otaManager.update(deltaMs);
}
```

- [ ] **Step 2: Verify `esp32dev` builds**

Run: `pio run -e esp32dev`
Expected: `SUCCESS`

- [ ] **Step 3: Verify native tests still pass (regression check — nothing in `lib/` changed this task)**

Run: `pio test -e native`
Expected: `19 Tests 0 Failures 0 Ignored` — `PASSED`

- [ ] **Step 4: Read `src/main.cpp` against `docs/architecture.md`'s frame-update diagram (Task 15) once that file exists**

This step is a manual sanity check, not a command: confirm `loop()`'s call order (`behaviorEngine` → `animationEngine` → `eyeController` → `servoOutput`) matches the documented pipeline. (If Task 15 hasn't run yet, compare instead against `docs/superpowers/specs/2026-07-25-firmware-bootstrap-design.md`'s "Frame update model" section.)

- [ ] **Step 5: Commit**

```bash
git add src/main.cpp
git commit -m "Wire the full frame-update loop in main.cpp"
```

---

### Task 14: CI workflow and formatting

**Files:**
- Create: `.github/workflows/ci.yml`
- Create: `.clang-format`

**Interfaces:**
- Consumes: `pio run -e esp32dev`, `pio test -e native` (all prior tasks).
- Produces: a CI pipeline; no firmware code changes.

- [ ] **Step 1: Create `.clang-format`**

```yaml
BasedOnStyle: Google
Language: Cpp
Standard: c++17
IndentWidth: 4
ColumnLimit: 100
PointerAlignment: Left
SortIncludes: false
```

- [ ] **Step 2: Create `.github/workflows/ci.yml`**

```yaml
name: CI

on:
  push:
  pull_request:

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - uses: actions/setup-python@v5
        with:
          python-version: "3.11"

      - name: Install PlatformIO
        run: pip install --upgrade platformio

      - name: Build firmware (esp32dev)
        run: pio run -e esp32dev

      - name: Run unit tests (native)
        run: pio test -e native

      - name: Install clang-format
        run: sudo apt-get update && sudo apt-get install -y clang-format

      - name: Check formatting
        run: clang-format --dry-run --Werror $(find lib src -name '*.h' -o -name '*.cpp')
```

`clang-tidy` is intentionally not included — see `docs/superpowers/specs/2026-07-25-firmware-bootstrap-design.md`'s Continuous Integration section for why, and `docs/ROADMAP.md` for where it's tracked.

- [ ] **Step 3: Verify formatting locally (if `clang-format` is installed)**

Run: `clang-format --dry-run --Werror $(find lib src -name '*.h' -o -name '*.cpp')`
Expected: no output, exit code 0. If any file fails, run `clang-format -i` on it and re-check.

If `clang-format` isn't installed locally, skip this step — CI will catch formatting issues on push.

- [ ] **Step 4: Commit**

```bash
git add .github/workflows/ci.yml .clang-format
git commit -m "Add CI workflow and clang-format config"
```

---

### Task 15: Documentation

**Files:**
- Modify: `README.md`
- Create: `docs/architecture.md`
- Create: `docs/ROADMAP.md`
- Create: `docs/Doxyfile`

**Interfaces:**
- Consumes: the completed module tree (Tasks 1–14).
- Produces: no code — documentation only.

- [ ] **Step 1: Rewrite the root `README.md`**

```markdown
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
| `lib/Networking` | Web UI, REST API, WebSocket (placeholders) |
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
```

- [ ] **Step 2: Create `docs/architecture.md`**

```markdown
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

```
Configuration (leaf: ServoConfig, EyeConfig, NetworkConfig, BehaviorConfig, CalibrationManager)
     ^
     |
MotionHardware (leaf: ServoOutput, IServoOutput, Pca9685ServoOutput)
     ^
     |
EyeController --------> depends on Configuration + MotionHardware
     ^
     |
Animation -------------> depends on EyeController
     ^
     |
Behavior --------------> depends on Animation + EyeController (GazeTarget/Expression)

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
`MotionHardware`, `Storage`, `Networking`, and `OTA` depend on
Arduino/hardware headers and are verified only by `pio run -e esp32dev`
compiling successfully.
```

- [ ] **Step 3: Create `docs/ROADMAP.md`**

```markdown
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
```

- [ ] **Step 4: Create `docs/Doxyfile`**

Run `doxygen -g docs/Doxyfile` to generate the full default config (thousands
of lines), then change exactly these keys (leave everything else default):

```
PROJECT_NAME           = "EyeSee Firmware"
OUTPUT_DIRECTORY       = docs/api
INPUT                  = lib src
RECURSIVE              = YES
EXTRACT_ALL            = YES
GENERATE_LATEX         = NO
```

If `doxygen` isn't installed locally, create `docs/Doxyfile` by copying a
default Doxyfile from any other project and applying the same six edits —
the CI/build system doesn't invoke Doxygen this pass, so this file only
needs to be correct enough for a contributor with Doxygen installed to run
`doxygen docs/Doxyfile` from the repo root successfully.

- [ ] **Step 5: Verify documentation builds (if Doxygen is installed)**

Run: `doxygen docs/Doxyfile`
Expected: exits 0, populates `docs/api/` (already gitignored from Task 1).

If Doxygen isn't installed, skip this step — there's no CI dependency on it.

- [ ] **Step 6: Commit**

```bash
git add README.md docs/architecture.md docs/ROADMAP.md docs/Doxyfile
git commit -m "Add project documentation (README, architecture, roadmap, Doxyfile)"
```

---

## Plan self-review

**Spec coverage:** Every module in the spec's directory layout has a task
(Configuration/CalibrationManager: Task 3–4; MotionHardware: Task 5;
EyeController: Task 6; Animation: Task 7; Behavior: Tasks 8–9; Storage:
Task 10; Networking: Task 11; OTA: Task 12; Logger: Task 2). Frame update
model, three invariants, `EyePose`/`ServoOutput` boundary, `GazeTarget`,
`CommandQueue`, versioned API, native test env, CI, and docs are all
covered (Tasks 3, 6, 7, 8, 14, 15).

**Placeholder scan:** No task step says "add appropriate logic" or leaves
a TBD — every stubbed method has concrete code with a `TODO` comment
pointing at a specific roadmap milestone.

**Type consistency:** `GazeTarget`/`Expression`/`EyePose` (Task 6) are used
identically in `IAnimationEngine` (Task 7) and `EyeCommand` (Task 8).
`IServoOutput`/`ServoOutput` (Task 5) match `EyeController`'s usage (Task 6)
and `FakeServoOutput`'s override signatures (Task 6). `CommandQueue`/
`EyeCommand`/`CommandType` (Task 8) match `BehaviorEngine::dispatch()`
(Task 9) exactly, including the four `IAnimationEngine` methods
(`animateWinkLeft`/`animateWinkRight`/`animateSleep`/`animateWake`) added
to cover every `CommandType`.
