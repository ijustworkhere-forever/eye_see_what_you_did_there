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
