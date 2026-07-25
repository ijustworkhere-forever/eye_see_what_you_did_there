#include <unity.h>

#include "CalibrationManager.h"
#include "Configuration.h"

using eyesee::CalibrationManager;
using eyesee::EyeChannel;
using eyesee::ServoConfig;
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

// CalibrationManager test declarations (defined in test_calibration_manager.cpp)
extern void test_default_calibration_reports_default_lr_channel();
extern void test_set_servo_config_updates_stored_value();
extern void test_set_servo_config_does_not_affect_other_channels();

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_default_eye_config_has_six_distinct_channels);
    RUN_TEST(test_default_servo_config_is_centered_and_uninverted);
    RUN_TEST(test_default_calibration_reports_default_lr_channel);
    RUN_TEST(test_set_servo_config_updates_stored_value);
    RUN_TEST(test_set_servo_config_does_not_affect_other_channels);
    return UNITY_END();
}
