#include <unity.h>

#include "CalibrationManager.h"
#include "EyeController.h"
#include "support/FakeServoOutput.h"

using eyesee::CalibrationManager;
using eyesee::EyeController;
using eyesee::EyePose;

void test_apply_pose_writes_neutral_pulses_and_calls_write_once() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.applyPose(EyePose{0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f});

    TEST_ASSERT_EQUAL_INT(1, output.writeCallCount);
    TEST_ASSERT_EQUAL_UINT16(calibration.eyeConfig().lr.neutralPulseUs, output.lastWrite.lr);
    TEST_ASSERT_EQUAL_UINT16(calibration.eyeConfig().ud.neutralPulseUs, output.lastWrite.ud);
}

void test_look_updates_gaze_but_preserves_eyelids() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.applyPose(EyePose{0.0f, 0.0f, 0.2f, 0.3f, 0.4f, 0.5f});
    controller.look(0.7f, -0.6f);

    const EyePose pose = controller.currentPose();
    TEST_ASSERT_EQUAL_FLOAT(0.7f, pose.lookX);
    TEST_ASSERT_EQUAL_FLOAT(-0.6f, pose.lookY);
    TEST_ASSERT_EQUAL_FLOAT(0.2f, pose.upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.3f, pose.lowerLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.4f, pose.upperRightLid);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, pose.lowerRightLid);
}

void test_blink_closes_all_four_lids() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.blink();

    const EyePose pose = controller.currentPose();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.lowerLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.upperRightLid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.lowerRightLid);
}

void test_wink_left_closes_only_left_lids() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.winkLeft();

    const EyePose pose = controller.currentPose();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.lowerLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, pose.upperRightLid);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, pose.lowerRightLid);
}

void test_set_idle_resets_to_default_pose() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.look(0.9f, 0.9f);
    controller.setIdle();

    const EyePose pose = controller.currentPose();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.lookX);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, pose.lookY);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, pose.upperLeftLid);
}
