#include <unity.h>

#include "Easing.h"

using eyesee::ease;
using eyesee::EasingType;

void test_linear_easing_is_identity() {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ease(EasingType::Linear, 0.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, ease(EasingType::Linear, 0.5f));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, ease(EasingType::Linear, 1.0f));
}

void test_ease_in_out_starts_and_ends_at_bounds() {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ease(EasingType::EaseInOut, 0.0f));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, ease(EasingType::EaseInOut, 1.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, ease(EasingType::EaseInOut, 0.5f));
}

void test_cubic_starts_and_ends_at_bounds() {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ease(EasingType::Cubic, 0.0f));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, ease(EasingType::Cubic, 1.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, ease(EasingType::Cubic, 0.5f));
}

void test_cubic_starts_slower_than_linear_and_ease_in_out() {
    // At 25% progress, cubic ease-in-out (4t^3) is well below the linear
    // and quadratic curves — this is what actually distinguishes it.
    const float cubicAt25 = ease(EasingType::Cubic, 0.25f);
    TEST_ASSERT_TRUE(cubicAt25 < 0.15f);
    TEST_ASSERT_TRUE(cubicAt25 < ease(EasingType::EaseInOut, 0.25f));
}

void test_ease_clamps_out_of_range_t() {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ease(EasingType::Linear, -0.5f));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, ease(EasingType::Linear, 1.5f));
}
