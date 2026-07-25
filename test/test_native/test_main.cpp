#include <unity.h>

// Configuration tests
extern void test_default_eye_config_has_six_distinct_channels();
extern void test_default_servo_config_is_centered_and_uninverted();

// CalibrationManager tests
extern void test_default_calibration_reports_default_lr_channel();
extern void test_set_servo_config_updates_stored_value();
extern void test_set_servo_config_does_not_affect_other_channels();

// EyeController tests
extern void test_apply_pose_writes_neutral_pulses_and_calls_write_once();
extern void test_look_updates_gaze_but_preserves_eyelids();
extern void test_blink_closes_all_four_lids();
extern void test_wink_left_closes_only_left_lids();
extern void test_set_idle_resets_to_default_pose();

// Animation tests
extern void test_animate_gaze_applies_pose_immediately();
extern void test_animate_blink_closes_eyelids_immediately();

// CommandQueue tests (Behavior module)
extern void test_push_pop_preserves_fifo_order();
extern void test_push_fails_when_full();
extern void test_clear_empties_queue();

// BehaviorEngine tests (Behavior module)
extern void test_update_drains_queue_and_dispatches_look_command();
extern void test_update_dispatches_blink_command();
extern void test_update_dispatches_all_remaining_command_types();
extern void test_set_state_updates_reported_state();

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_default_eye_config_has_six_distinct_channels);
    RUN_TEST(test_default_servo_config_is_centered_and_uninverted);
    RUN_TEST(test_default_calibration_reports_default_lr_channel);
    RUN_TEST(test_set_servo_config_updates_stored_value);
    RUN_TEST(test_set_servo_config_does_not_affect_other_channels);
    RUN_TEST(test_apply_pose_writes_neutral_pulses_and_calls_write_once);
    RUN_TEST(test_look_updates_gaze_but_preserves_eyelids);
    RUN_TEST(test_blink_closes_all_four_lids);
    RUN_TEST(test_wink_left_closes_only_left_lids);
    RUN_TEST(test_set_idle_resets_to_default_pose);
    RUN_TEST(test_animate_gaze_applies_pose_immediately);
    RUN_TEST(test_animate_blink_closes_eyelids_immediately);
    RUN_TEST(test_push_pop_preserves_fifo_order);
    RUN_TEST(test_push_fails_when_full);
    RUN_TEST(test_clear_empties_queue);
    RUN_TEST(test_update_drains_queue_and_dispatches_look_command);
    RUN_TEST(test_update_dispatches_blink_command);
    RUN_TEST(test_update_dispatches_all_remaining_command_types);
    RUN_TEST(test_set_state_updates_reported_state);
    return UNITY_END();
}
