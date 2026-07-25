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
