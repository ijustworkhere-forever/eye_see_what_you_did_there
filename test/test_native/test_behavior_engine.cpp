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
    TEST_ASSERT_EQUAL_INT(static_cast<int>(Expression::Happy),
                          static_cast<int>(animation.lastExpression));
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
