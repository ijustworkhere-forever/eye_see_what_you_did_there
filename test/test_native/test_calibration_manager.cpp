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
