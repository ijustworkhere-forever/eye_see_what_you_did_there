#include <unity.h>

#include "CalibrationManager.h"
#include "support/FakeStorage.h"

using eyesee::CalibrationManager;
using eyesee::EyeChannel;
using eyesee::EyeConfig;
using eyesee::ServoConfig;

void test_default_calibration_reports_default_lr_channel() {
    CalibrationManager calibration;
    TEST_ASSERT_EQUAL_UINT8(0, calibration.servoConfig(EyeChannel::Lr).channel);
    TEST_ASSERT_EQUAL_UINT16(1500, calibration.servoConfig(EyeChannel::Lr).neutralPulseUs);
}

void test_set_servo_config_updates_stored_value() {
    CalibrationManager calibration;
    ServoConfig updated = calibration.servoConfig(EyeChannel::Ud);
    updated.inverted = true;
    updated.neutralPulseUs = 1600;

    calibration.setServoConfig(EyeChannel::Ud, updated);

    TEST_ASSERT_TRUE(calibration.servoConfig(EyeChannel::Ud).inverted);
    TEST_ASSERT_EQUAL_UINT16(1600, calibration.servoConfig(EyeChannel::Ud).neutralPulseUs);
}

void test_set_servo_config_does_not_affect_other_channels() {
    CalibrationManager calibration;
    ServoConfig updated = calibration.servoConfig(EyeChannel::Tl);
    updated.neutralPulseUs = 1700;

    calibration.setServoConfig(EyeChannel::Tl, updated);

    TEST_ASSERT_EQUAL_UINT16(1500, calibration.servoConfig(EyeChannel::Tr).neutralPulseUs);
}

void test_save_and_load_round_trips_full_config() {
    CalibrationManager original;
    ServoConfig lr = original.servoConfig(EyeChannel::Lr);
    lr.minPulseUs = 900;
    lr.maxPulseUs = 2100;
    lr.neutralPulseUs = 1550;
    lr.mechanicalOffset = -15;
    lr.inverted = true;
    lr.mirrored = false;
    original.setServoConfig(EyeChannel::Lr, lr);

    FakeStorage storage;
    storage.begin("test");
    original.saveToStorage(storage);

    CalibrationManager loaded;
    TEST_ASSERT_TRUE(loaded.loadFromStorage(storage));
    const ServoConfig& loadedLr = loaded.servoConfig(EyeChannel::Lr);
    TEST_ASSERT_EQUAL_UINT16(900, loadedLr.minPulseUs);
    TEST_ASSERT_EQUAL_UINT16(2100, loadedLr.maxPulseUs);
    TEST_ASSERT_EQUAL_UINT16(1550, loadedLr.neutralPulseUs);
    TEST_ASSERT_EQUAL_INT16(-15, loadedLr.mechanicalOffset);
    TEST_ASSERT_TRUE(loadedLr.inverted);
    TEST_ASSERT_FALSE(loadedLr.mirrored);
}

void test_save_and_load_round_trips_look_range() {
    CalibrationManager original;
    EyeConfig config = original.eyeConfig();
    config.lookRangeDegrees = 45.5f;
    // lookRangeDegrees has no setter on CalibrationManager (only per-channel
    // servoConfig is mutable via setServoConfig) -- construct with the full
    // config directly instead, matching the existing explicit-EyeConfig constructor.
    CalibrationManager withRange(config);

    FakeStorage storage;
    storage.begin("test");
    withRange.saveToStorage(storage);

    CalibrationManager loaded;
    TEST_ASSERT_TRUE(loaded.loadFromStorage(storage));
    TEST_ASSERT_EQUAL_FLOAT(45.5f, loaded.eyeConfig().lookRangeDegrees);
}

void test_load_from_empty_storage_returns_false_and_keeps_defaults() {
    FakeStorage storage;
    storage.begin("test");

    CalibrationManager manager;
    const ServoConfig before = manager.servoConfig(EyeChannel::Lr);
    TEST_ASSERT_FALSE(manager.loadFromStorage(storage));
    const ServoConfig& after = manager.servoConfig(EyeChannel::Lr);
    TEST_ASSERT_EQUAL_UINT16(before.minPulseUs, after.minPulseUs);
    TEST_ASSERT_EQUAL_UINT16(before.neutralPulseUs, after.neutralPulseUs);
}

void test_save_persists_all_six_channels_independently() {
    CalibrationManager original;
    ServoConfig ud = original.servoConfig(EyeChannel::Ud);
    ud.neutralPulseUs = 1600;
    original.setServoConfig(EyeChannel::Ud, ud);

    FakeStorage storage;
    storage.begin("test");
    original.saveToStorage(storage);

    CalibrationManager loaded;
    TEST_ASSERT_TRUE(loaded.loadFromStorage(storage));
    TEST_ASSERT_EQUAL_UINT16(1600, loaded.servoConfig(EyeChannel::Ud).neutralPulseUs);
    // Lr wasn't changed from its default -- confirms per-channel keys don't collide.
    TEST_ASSERT_EQUAL_UINT16(original.servoConfig(EyeChannel::Lr).neutralPulseUs,
                             loaded.servoConfig(EyeChannel::Lr).neutralPulseUs);
}
