#include <unity.h>

#include "CuriousBehavior.h"
#include "support/FakeAnimationEngine.h"
#include "support/FakeRandomSource.h"

using eyesee::CuriousBehavior;
using eyesee::Expression;
using eyesee::EyeState;

void test_curious_behavior_sets_curious_expression_on_enter() {
    FakeRandomSource random;
    random.uintSequence = {2000};
    FakeAnimationEngine animation;
    CuriousBehavior behavior(random);

    behavior.onEnter(animation);

    TEST_ASSERT_EQUAL_INT(1, animation.animateExpressionCallCount);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(Expression::Curious),
                          static_cast<int>(animation.lastExpression));
}

void test_curious_behavior_waits_for_full_interval_before_glancing() {
    FakeRandomSource random;
    random.uintSequence = {2000};
    random.floatSequence = {0.4f, -0.2f};
    FakeAnimationEngine animation;
    CuriousBehavior behavior(random);

    behavior.onEnter(animation);
    behavior.update(1000, animation);
    TEST_ASSERT_EQUAL_INT(0, animation.animateGazeCallCount);

    behavior.update(1000, animation);
    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);
}

void test_curious_behavior_reports_idle_state() {
    FakeRandomSource random;
    CuriousBehavior behavior(random);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeState::Idle), static_cast<int>(behavior.state()));
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
