#include <unity.h>

#include "BehaviorEngine.h"
#include "IdleBehavior.h"
#include "support/FakeAnimationEngine.h"
#include "support/FakeBehavior.h"
#include "support/FakeRandomSource.h"

using namespace eyesee;

void test_update_drains_queue_and_dispatches_look_command() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    FakeRandomSource random;
    IdleBehavior idleBehavior(random);
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
    FakeRandomSource random;
    IdleBehavior idleBehavior(random);
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
    FakeRandomSource random;
    IdleBehavior idleBehavior(random);
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
    TEST_ASSERT_EQUAL_INT(static_cast<int>(Expression::Happy),
                          static_cast<int>(animation.lastExpression));
}

void test_set_state_updates_reported_state() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    FakeRandomSource random;
    IdleBehavior idleBehavior(random);
    BehaviorEngine engine(animation, queue, idleBehavior);

    engine.setState(EyeState::Manual);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeState::Manual), static_cast<int>(engine.state()));
}

void test_register_behavior_and_set_state_switches_active_behavior() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    FakeBehavior idleBehavior(EyeState::Idle);
    FakeBehavior trackingBehavior(EyeState::Tracking);
    BehaviorEngine engine(animation, queue, idleBehavior);
    engine.registerBehavior(EyeState::Tracking, trackingBehavior);

    engine.setState(EyeState::Tracking);
    engine.update(16);

    TEST_ASSERT_EQUAL_INT(1, trackingBehavior.updateCallCount);
    TEST_ASSERT_EQUAL_INT(0, idleBehavior.updateCallCount);
}

void test_set_state_calls_on_exit_then_on_enter() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    FakeBehavior idleBehavior(EyeState::Idle);
    FakeBehavior trackingBehavior(EyeState::Tracking);
    BehaviorEngine engine(animation, queue, idleBehavior);
    engine.registerBehavior(EyeState::Idle, idleBehavior);
    engine.registerBehavior(EyeState::Tracking, trackingBehavior);

    engine.setState(EyeState::Idle);
    engine.setState(EyeState::Tracking);

    TEST_ASSERT_EQUAL_INT(1, idleBehavior.onExitCallCount);
    TEST_ASSERT_EQUAL_INT(1, trackingBehavior.onEnterCallCount);
}

void test_set_state_to_same_state_does_not_retrigger_lifecycle() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    FakeBehavior idleBehavior(EyeState::Idle);
    BehaviorEngine engine(animation, queue, idleBehavior);
    engine.registerBehavior(EyeState::Idle, idleBehavior);

    engine.setState(EyeState::Idle);
    engine.setState(EyeState::Idle);

    TEST_ASSERT_EQUAL_INT(1, idleBehavior.onEnterCallCount);
}

void test_unregistered_state_falls_back_to_fallback_behavior() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    FakeBehavior idleBehavior(EyeState::Idle);
    BehaviorEngine engine(animation, queue, idleBehavior);

    engine.setState(EyeState::Calibration);  // never registered
    engine.update(16);

    TEST_ASSERT_EQUAL_INT(1, idleBehavior.updateCallCount);
}

void test_sleep_command_transitions_state_to_sleeping() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    FakeBehavior idleBehavior(EyeState::Idle);
    BehaviorEngine engine(animation, queue, idleBehavior);

    EyeCommand sleepCmd;
    sleepCmd.type = CommandType::Sleep;
    queue.push(sleepCmd);
    engine.update(16);

    TEST_ASSERT_EQUAL_INT(1, animation.animateSleepCallCount);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeState::Sleeping), static_cast<int>(engine.state()));
}

void test_wake_command_transitions_state_to_idle() {
    CommandQueue queue;
    FakeAnimationEngine animation;
    FakeBehavior idleBehavior(EyeState::Idle);
    BehaviorEngine engine(animation, queue, idleBehavior);
    engine.setState(EyeState::Sleeping);

    EyeCommand wakeCmd;
    wakeCmd.type = CommandType::Wake;
    queue.push(wakeCmd);
    engine.update(16);

    TEST_ASSERT_EQUAL_INT(1, animation.animateWakeCallCount);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeState::Idle), static_cast<int>(engine.state()));
}

// No main() here — test/test_native/test_main.cpp is the sole file with main()
// (see its own comment for why).
