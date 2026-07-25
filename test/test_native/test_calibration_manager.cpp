#include <unity.h>

#include "CalibrationManager.h"

using eyesee::CalibrationManager;
using eyesee::EyeChannel;
using eyesee::ServoConfig;

void test_default_calibration_reports_default_lr_channel() {
    CalibrationManager calibration;
    TEST_ASSERT_EQUAL_UINT8(0, calibration.servoConfig(EyeChannel::Lr).channel);
    TEST_ASSERT_EQUAL_UINT16(1500, calibration.servoConfig(EyeChannel::Lr).neutralPulseUs);
}

void test_set_servo_config_updates_stored_value() {
    CalibrationManager calibration;
    ServoConfig updated = calibration.servoConfig(EyeChannel::Ud);
    updated.inverted = true;
    updated.neutralPulseUs = 1600;

    calibration.setServoConfig(EyeChannel::Ud, updated);

    TEST_ASSERT_TRUE(calibration.servoConfig(EyeChannel::Ud).inverted);
    TEST_ASSERT_EQUAL_UINT16(1600, calibration.servoConfig(EyeChannel::Ud).neutralPulseUs);
}

void test_set_servo_config_does_not_affect_other_channels() {
    CalibrationManager calibration;
    ServoConfig updated = calibration.servoConfig(EyeChannel::Tl);
    updated.neutralPulseUs = 1700;

    calibration.setServoConfig(EyeChannel::Tl, updated);

    TEST_ASSERT_EQUAL_UINT16(1500, calibration.servoConfig(EyeChannel::Tr).neutralPulseUs);
}
