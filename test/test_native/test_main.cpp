#include <unity.h>

// PlatformIO links every .cpp under test/test_native/ into a single test
// binary, so exactly one translation unit may define main(). This is that file:
// the other test_*.cpp files contain test functions only, and each one is
// declared extern and registered below.

// Configuration tests
extern void test_default_eye_config_has_six_distinct_channels();
extern void test_default_servo_config_is_centered_and_uninverted();

// CalibrationManager tests
extern void test_default_calibration_reports_default_lr_channel();
extern void test_set_servo_config_updates_stored_value();
extern void test_set_servo_config_does_not_affect_other_channels();

// EyeController tests
extern void test_apply_pose_writes_calibrated_pulses_and_calls_write_once();
extern void test_gaze_channel_full_positive_maps_to_max_pulse();
extern void test_gaze_channel_full_negative_maps_to_min_pulse();
extern void test_inverted_channel_flips_gaze_direction();
extern void test_inverted_and_mirrored_together_cancel_out();
extern void test_eyelid_open_maps_to_max_pulse();
extern void test_mechanical_offset_shifts_pulse_within_clamp();
extern void test_look_updates_gaze_but_preserves_eyelids();
extern void test_blink_closes_all_four_lids();
extern void test_wink_left_closes_only_left_lids();
extern void test_set_expression_does_not_change_pose();
extern void test_set_idle_resets_to_default_pose();

// Easing tests
extern void test_linear_easing_is_identity();
extern void test_ease_in_out_starts_and_ends_at_bounds();
extern void test_cubic_starts_and_ends_at_bounds();
extern void test_cubic_starts_slower_than_linear_and_ease_in_out();
extern void test_ease_clamps_out_of_range_t();

// Animation tests
extern void test_animate_gaze_does_not_reach_target_before_duration_elapses();
extern void test_animate_gaze_reaches_target_after_full_duration();
extern void test_animate_gaze_uses_cubic_easing_not_linear();
extern void test_animate_blink_closes_lids_only_after_full_duration();
extern void test_gaze_and_eyelid_transitions_compose_in_the_same_frame();
extern void test_wink_left_preserves_right_eyelids();
extern void test_blink_on_arrival_triggers_blink_after_gaze_completes();
extern void test_eyelid_transition_uses_ease_in_out_not_cubic_or_linear();
extern void test_interrupting_mid_flight_transition_continues_from_current_value();
extern void test_animate_gaze_with_zero_speed_snaps_instantly();
extern void test_animate_gaze_with_negative_speed_snaps_instantly();
extern void test_animate_blink_with_zero_duration_completes_in_one_frame();

// CommandQueue tests (Behavior module)
extern void test_push_pop_preserves_fifo_order();
extern void test_push_fails_when_full();
extern void test_clear_empties_queue();

// BehaviorEngine tests (Behavior module)
extern void test_update_drains_queue_and_dispatches_look_command();
extern void test_update_dispatches_blink_command();
extern void test_update_dispatches_all_remaining_command_types();
extern void test_set_state_updates_reported_state();
extern void test_register_behavior_and_set_state_switches_active_behavior();
extern void test_set_state_calls_on_exit_then_on_enter();
extern void test_set_state_to_same_state_does_not_retrigger_lifecycle();
extern void test_unregistered_state_falls_back_to_fallback_behavior();
extern void test_sleep_command_transitions_state_to_sleeping();
extern void test_wake_command_transitions_state_to_idle();

// IdleBehavior tests
extern void test_idle_behavior_waits_for_full_interval_before_glancing();
extern void test_idle_behavior_glance_stays_within_micro_saccade_range();
extern void test_idle_behavior_reports_idle_state();

// SleepBehavior tests
extern void test_sleep_behavior_closes_eyes_on_enter();
extern void test_sleep_behavior_update_does_nothing();
extern void test_sleep_behavior_reports_sleeping_state();

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_default_eye_config_has_six_distinct_channels);
    RUN_TEST(test_default_servo_config_is_centered_and_uninverted);
    RUN_TEST(test_default_calibration_reports_default_lr_channel);
    RUN_TEST(test_set_servo_config_updates_stored_value);
    RUN_TEST(test_set_servo_config_does_not_affect_other_channels);
    RUN_TEST(test_apply_pose_writes_calibrated_pulses_and_calls_write_once);
    RUN_TEST(test_gaze_channel_full_positive_maps_to_max_pulse);
    RUN_TEST(test_gaze_channel_full_negative_maps_to_min_pulse);
    RUN_TEST(test_inverted_channel_flips_gaze_direction);
    RUN_TEST(test_inverted_and_mirrored_together_cancel_out);
    RUN_TEST(test_eyelid_open_maps_to_max_pulse);
    RUN_TEST(test_mechanical_offset_shifts_pulse_within_clamp);
    RUN_TEST(test_look_updates_gaze_but_preserves_eyelids);
    RUN_TEST(test_blink_closes_all_four_lids);
    RUN_TEST(test_wink_left_closes_only_left_lids);
    RUN_TEST(test_set_expression_does_not_change_pose);
    RUN_TEST(test_set_idle_resets_to_default_pose);
    RUN_TEST(test_linear_easing_is_identity);
    RUN_TEST(test_ease_in_out_starts_and_ends_at_bounds);
    RUN_TEST(test_cubic_starts_and_ends_at_bounds);
    RUN_TEST(test_cubic_starts_slower_than_linear_and_ease_in_out);
    RUN_TEST(test_ease_clamps_out_of_range_t);
    RUN_TEST(test_animate_gaze_does_not_reach_target_before_duration_elapses);
    RUN_TEST(test_animate_gaze_reaches_target_after_full_duration);
    RUN_TEST(test_animate_gaze_uses_cubic_easing_not_linear);
    RUN_TEST(test_animate_blink_closes_lids_only_after_full_duration);
    RUN_TEST(test_gaze_and_eyelid_transitions_compose_in_the_same_frame);
    RUN_TEST(test_wink_left_preserves_right_eyelids);
    RUN_TEST(test_blink_on_arrival_triggers_blink_after_gaze_completes);
    RUN_TEST(test_eyelid_transition_uses_ease_in_out_not_cubic_or_linear);
    RUN_TEST(test_interrupting_mid_flight_transition_continues_from_current_value);
    RUN_TEST(test_animate_gaze_with_zero_speed_snaps_instantly);
    RUN_TEST(test_animate_gaze_with_negative_speed_snaps_instantly);
    RUN_TEST(test_animate_blink_with_zero_duration_completes_in_one_frame);
    RUN_TEST(test_push_pop_preserves_fifo_order);
    RUN_TEST(test_push_fails_when_full);
    RUN_TEST(test_clear_empties_queue);
    RUN_TEST(test_update_drains_queue_and_dispatches_look_command);
    RUN_TEST(test_update_dispatches_blink_command);
    RUN_TEST(test_update_dispatches_all_remaining_command_types);
    RUN_TEST(test_set_state_updates_reported_state);
    RUN_TEST(test_register_behavior_and_set_state_switches_active_behavior);
    RUN_TEST(test_set_state_calls_on_exit_then_on_enter);
    RUN_TEST(test_set_state_to_same_state_does_not_retrigger_lifecycle);
    RUN_TEST(test_unregistered_state_falls_back_to_fallback_behavior);
    RUN_TEST(test_sleep_command_transitions_state_to_sleeping);
    RUN_TEST(test_wake_command_transitions_state_to_idle);
    RUN_TEST(test_idle_behavior_waits_for_full_interval_before_glancing);
    RUN_TEST(test_idle_behavior_glance_stays_within_micro_saccade_range);
    RUN_TEST(test_idle_behavior_reports_idle_state);
    RUN_TEST(test_sleep_behavior_closes_eyes_on_enter);
    RUN_TEST(test_sleep_behavior_update_does_nothing);
    RUN_TEST(test_sleep_behavior_reports_sleeping_state);
    return UNITY_END();
}
