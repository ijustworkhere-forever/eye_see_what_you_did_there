#include <unity.h>

#include "CalibrationManager.h"
#include "EyeController.h"
#include "RealAnimationEngine.h"
#include "support/FakeServoOutput.h"

using eyesee::CalibrationManager;
using eyesee::Expression;
using eyesee::EyeController;
using eyesee::EyePose;
using eyesee::GazeTarget;
using eyesee::RealAnimationEngine;

void test_animate_gaze_does_not_reach_target_before_duration_elapses() {
    FakeServoOutput output;
    CalibrationManager calibration;  // lookRangeDegrees defaults to 30.0
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 300.0f;  // duration = (1.0 * 30 / 300) * 1000 = 100ms
    animation.animateGaze(target);

    animation.update(50);  // halfway through the 100ms transition

    TEST_ASSERT_TRUE(controller.currentPose().lookX > 0.0f);
    TEST_ASSERT_TRUE(controller.currentPose().lookX < 1.0f);
}

void test_animate_gaze_reaches_target_after_full_duration() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 300.0f;  // 100ms duration
    animation.animateGaze(target);

    animation.update(60);
    animation.update(60);  // 120ms total, past the 100ms duration

    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().lookX);
}

void test_animate_gaze_uses_cubic_easing_not_linear() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 300.0f;  // 100ms duration
    animation.animateGaze(target);

    animation.update(25);  // 25% of the way through: cubic(0.25) = 0.0625, ease-in-out(0.25) =
                           // 0.125, linear(0.25) = 0.25 — 0.10 is below both non-cubic values, so
                           // this actually pins Cubic rather than merely ruling out Linear.

    TEST_ASSERT_TRUE(controller.currentPose().lookX < 0.10f);
}

void test_eyelid_transition_uses_ease_in_out_not_cubic_or_linear() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateBlink(200);
    animation.update(50);  // 25% of the way through: ease-in-out(0.25) = 0.125, so lid = 0.875.
                           // cubic(0.25) = 0.0625 -> lid = 0.9375 (fails high); linear(0.25) =
                           // 0.25 -> lid = 0.75 (fails low). This pins EaseInOut specifically.

    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid > 0.80f);
    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid < 0.90f);
}

void test_interrupting_mid_flight_transition_continues_from_current_value() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateBlink(100);
    animation.update(50);  // lid now at 0.5, blink half-closed

    // Interrupts mid-flight with a transition whose target (1.0) differs from the blink's
    // target (0.0), so the assertion below can distinguish start=current from start=old-target.
    // Correct: start=0.5, target=1.0, EaseInOut(0.5)=0.5 -> 0.5 + 0.5*(1.0-0.5) = 0.75.
    // Broken (start=old target 0.0): 0.0 + 0.5*(1.0-0.0) = 0.5, which fails this assertion.
    animation.animateWake(100);
    animation.update(50);

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.75f, controller.currentPose().upperLeftLid);
}

void test_animate_blink_closes_lids_only_after_full_duration() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateBlink(100);
    animation.update(50);

    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid > 0.0f);

    animation.update(60);  // 110ms total, past the 100ms duration

    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);
}

void test_gaze_and_eyelid_transitions_compose_in_the_same_frame() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 300.0f;  // 100ms
    animation.animateGaze(target);
    animation.animateBlink(200);

    animation.update(50);

    TEST_ASSERT_TRUE(controller.currentPose().lookX > 0.0f);
    TEST_ASSERT_TRUE(controller.currentPose().lookX < 1.0f);
    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid > 0.0f);
    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid < 1.0f);
}

void test_wink_left_preserves_right_eyelids() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateWinkLeft(100);
    animation.update(100);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().upperRightLid);
}

void test_blink_on_arrival_triggers_blink_after_gaze_completes() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 300.0f;  // 100ms
    target.blinkOnArrival = true;
    animation.animateGaze(target);

    animation.update(100);  // completes gaze, should trigger a 150ms blink
    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().lookX);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().upperLeftLid);  // blink not yet advanced

    animation.update(150);  // advance the triggered blink to completion
    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);

    // blinkOnArrival must fire exactly once: reopen the eyes, then run another frame with
    // nothing commanded. A latched flag that re-fired would close the lids again here.
    animation.animateWake(100);
    animation.update(100);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().upperLeftLid);

    animation.update(50);  // nothing new commanded — no spurious second blink
    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().upperLeftLid);
}

void test_animate_gaze_with_zero_speed_snaps_instantly() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 0.0f;  // documented contract: speed <= 0 means instant snap, no animation
    animation.animateGaze(target);

    animation.update(1);  // a single tiny tick is enough

    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().lookX);
}

void test_animate_gaze_with_negative_speed_snaps_instantly() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = -5.0f;  // same contract as zero: no negative-duration or NaN math
    animation.animateGaze(target);

    animation.update(1);

    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().lookX);
}

void test_animate_blink_with_zero_duration_completes_in_one_frame() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateBlink(0);  // zero duration must not divide by zero
    animation.update(1);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);
}

void test_animate_blink_reopens_after_close_phase_completes() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateBlink(100);
    animation.update(100);  // completes close phase
    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);

    animation.update(50);  // reopen in progress
    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid > 0.0f);
    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid < 1.0f);

    animation.update(60);  // reopen completes (100ms reopen duration)
    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().upperLeftLid);

    animation.update(500);  // long after reopen completes: must not re-arm and reopen again
    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().upperLeftLid);
}

void test_animate_wink_left_does_not_auto_reopen() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateWinkLeft(100);
    animation.update(100);
    animation.update(500);  // long after completion

    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);
}

void test_animate_sleep_does_not_auto_reopen() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateSleep(100);
    animation.update(100);
    animation.update(500);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);
}

void test_animate_expression_blends_toward_target_eyelid_values() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateExpression(Expression::Sleepy, 100);
    animation.update(50);

    const EyePose midPose = controller.currentPose();
    TEST_ASSERT_TRUE(midPose.upperLeftLid > 0.3f);
    TEST_ASSERT_TRUE(midPose.upperLeftLid < 1.0f);

    animation.update(60);

    TEST_ASSERT_EQUAL_FLOAT(0.3f, controller.currentPose().upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, controller.currentPose().lowerLeftLid);
}

void test_animate_expression_does_not_affect_gaze() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget gazeTarget;
    gazeTarget.x = 0.5f;
    gazeTarget.speed = 1000.0f;
    animation.animateGaze(gazeTarget);
    animation.update(100);

    animation.animateExpression(Expression::Happy, 100);
    animation.update(100);

    TEST_ASSERT_EQUAL_FLOAT(0.5f, controller.currentPose().lookX);
}

// No main() here — test/test_native/test_main.cpp is the sole file with
// main() (see its own comment for why).
