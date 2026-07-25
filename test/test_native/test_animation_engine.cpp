#include <unity.h>

#include "CalibrationManager.h"
#include "EyeController.h"
#include "PassthroughAnimationEngine.h"
#include "support/FakeServoOutput.h"

using eyesee::CalibrationManager;
using eyesee::EyeController;
using eyesee::GazeTarget;
using eyesee::PassthroughAnimationEngine;

void test_animate_gaze_applies_pose_immediately() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    PassthroughAnimationEngine animation(controller);

    GazeTarget target;
    target.x = 0.3f;
    target.y = -0.2f;
    animation.animateGaze(target);

    TEST_ASSERT_EQUAL_FLOAT(0.3f, controller.currentPose().lookX);
    TEST_ASSERT_EQUAL_FLOAT(-0.2f, controller.currentPose().lookY);
    TEST_ASSERT_EQUAL_INT(1, output.writeCallCount);
}

void test_animate_blink_closes_eyelids_immediately() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    PassthroughAnimationEngine animation(controller);

    animation.animateBlink(150);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperRightLid);
}
