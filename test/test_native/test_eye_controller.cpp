#include <unity.h>

#include "CalibrationManager.h"
#include "EyeController.h"
#include "support/FakeServoOutput.h"

using eyesee::CalibrationManager;
using eyesee::Expression;
using eyesee::EyeChannel;
using eyesee::EyeController;
using eyesee::EyePose;
using eyesee::ServoConfig;

void test_apply_pose_writes_calibrated_pulses_and_calls_write_once() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.applyPose(EyePose{0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f});

    TEST_ASSERT_EQUAL_INT(1, output.writeCallCount);
    // lookX=0.5 -> halfway from neutral(1500) to max(2000) = 1750
    TEST_ASSERT_EQUAL_UINT16(1750, output.lastWrite.lr);
    // lookY=-0.5 -> halfway from neutral(1500) to min(1000) = 1250
    TEST_ASSERT_EQUAL_UINT16(1250, output.lastWrite.ud);
    // all four lids closed (0.0) -> min pulse (1000)
    TEST_ASSERT_EQUAL_UINT16(1000, output.lastWrite.tl);
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

void test_set_expression_applies_expression_target_but_preserves_gaze() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.look(0.6f, -0.3f);

    controller.setExpression(Expression::Sleepy);

    const EyePose pose = controller.currentPose();
    TEST_ASSERT_EQUAL_FLOAT(0.6f, pose.lookX);
    TEST_ASSERT_EQUAL_FLOAT(-0.3f, pose.lookY);
    TEST_ASSERT_EQUAL_FLOAT(0.3f, pose.upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, pose.lowerLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(0.3f, pose.upperRightLid);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, pose.lowerRightLid);
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

void test_gaze_channel_full_positive_maps_to_max_pulse() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.look(1.0f, 0.0f);

    TEST_ASSERT_EQUAL_UINT16(calibration.eyeConfig().lr.maxPulseUs, output.lastWrite.lr);
}

void test_gaze_channel_full_negative_maps_to_min_pulse() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.look(-1.0f, 0.0f);

    TEST_ASSERT_EQUAL_UINT16(calibration.eyeConfig().lr.minPulseUs, output.lastWrite.lr);
}

void test_inverted_channel_flips_gaze_direction() {
    FakeServoOutput output;
    CalibrationManager calibration;
    ServoConfig invertedLr = calibration.servoConfig(EyeChannel::Lr);
    invertedLr.inverted = true;
    calibration.setServoConfig(EyeChannel::Lr, invertedLr);
    EyeController controller(output, calibration);

    controller.look(1.0f, 0.0f);

    TEST_ASSERT_EQUAL_UINT16(calibration.eyeConfig().lr.minPulseUs, output.lastWrite.lr);
}

void test_inverted_and_mirrored_together_cancel_out() {
    FakeServoOutput output;
    CalibrationManager calibration;
    ServoConfig both = calibration.servoConfig(EyeChannel::Lr);
    both.inverted = true;
    both.mirrored = true;
    calibration.setServoConfig(EyeChannel::Lr, both);
    EyeController controller(output, calibration);

    controller.look(1.0f, 0.0f);

    TEST_ASSERT_EQUAL_UINT16(calibration.eyeConfig().lr.maxPulseUs, output.lastWrite.lr);
}

void test_eyelid_open_maps_to_max_pulse() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);

    controller.wake();

    TEST_ASSERT_EQUAL_UINT16(calibration.eyeConfig().tl.maxPulseUs, output.lastWrite.tl);
}

void test_mechanical_offset_shifts_pulse_within_clamp() {
    FakeServoOutput output;
    CalibrationManager calibration;
    ServoConfig offsetLr = calibration.servoConfig(EyeChannel::Lr);
    offsetLr.mechanicalOffset = 50;
    calibration.setServoConfig(EyeChannel::Lr, offsetLr);
    EyeController controller(output, calibration);

    controller.look(0.0f, 0.0f);

    TEST_ASSERT_EQUAL_UINT16(1550, output.lastWrite.lr);  // neutral(1500) + offset(50)
}
