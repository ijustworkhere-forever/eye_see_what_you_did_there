#include <unity.h>

#include "ExpressionPose.h"

using eyesee::Expression;
using eyesee::expressionEyelidTarget;

void test_neutral_expression_is_relaxed_open() {
    const auto target = expressionEyelidTarget(Expression::Neutral);
    TEST_ASSERT_EQUAL_FLOAT(0.85f, target.upperLid);
    TEST_ASSERT_EQUAL_FLOAT(0.85f, target.lowerLid);
}

void test_sleepy_expression_is_mostly_closed() {
    const auto target = expressionEyelidTarget(Expression::Sleepy);
    TEST_ASSERT_EQUAL_FLOAT(0.3f, target.upperLid);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, target.lowerLid);
}

void test_surprised_expression_is_fully_open() {
    const auto target = expressionEyelidTarget(Expression::Surprised);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, target.upperLid);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, target.lowerLid);
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
