#include <unity.h>

#include "GamepadInputMapper.h"

using eyesee::CommandType;
using eyesee::EyeCommand;
using eyesee::Expression;
using eyesee::GamepadState;
using eyesee::kMaxCommandsPerFrame;
using eyesee::mapGamepadState;

void test_centered_stick_and_no_buttons_emits_nothing() {
    GamepadState current;
    GamepadState previous;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    const size_t count = mapGamepadState(current, previous, out);

    TEST_ASSERT_EQUAL_size_t(0, count);
}

void test_stick_beyond_deadzone_emits_look() {
    GamepadState current;
    current.leftStickX = 0.5f;
    current.leftStickY = -0.3f;
    GamepadState previous;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    const size_t count = mapGamepadState(current, previous, out);

    TEST_ASSERT_EQUAL_size_t(1, count);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Look), static_cast<int>(out[0].type));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, out[0].gazeTarget.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.3f, out[0].gazeTarget.y);
}

void test_stick_within_deadzone_emits_nothing() {
    GamepadState current;
    current.leftStickX = 0.05f;
    current.leftStickY = -0.05f;
    GamepadState previous;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    const size_t count = mapGamepadState(current, previous, out);

    TEST_ASSERT_EQUAL_size_t(0, count);
}

void test_button_a_press_edge_emits_blink_once() {
    GamepadState previous;
    GamepadState pressed = previous;
    pressed.buttonA = true;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    const size_t count = mapGamepadState(pressed, previous, out);

    TEST_ASSERT_EQUAL_size_t(1, count);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Blink), static_cast<int>(out[0].type));
    TEST_ASSERT_EQUAL_UINT32(150, out[0].durationMs);
}

void test_button_a_held_does_not_repeat() {
    GamepadState held;
    held.buttonA = true;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    // previous frame ALSO has buttonA held -- no press edge this frame
    const size_t count = mapGamepadState(held, held, out);

    TEST_ASSERT_EQUAL_size_t(0, count);
}

void test_left_bumper_press_emits_wink_left() {
    GamepadState previous;
    GamepadState pressed = previous;
    pressed.leftBumper = true;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    const size_t count = mapGamepadState(pressed, previous, out);

    TEST_ASSERT_EQUAL_size_t(1, count);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::WinkLeft), static_cast<int>(out[0].type));
    TEST_ASSERT_EQUAL_UINT32(150, out[0].durationMs);
}

void test_right_bumper_press_emits_wink_right() {
    GamepadState previous;
    GamepadState pressed = previous;
    pressed.rightBumper = true;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    const size_t count = mapGamepadState(pressed, previous, out);

    TEST_ASSERT_EQUAL_size_t(1, count);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::WinkRight), static_cast<int>(out[0].type));
    TEST_ASSERT_EQUAL_UINT32(150, out[0].durationMs);
}

void test_button_y_press_emits_happy_expression() {
    GamepadState previous;
    GamepadState pressed = previous;
    pressed.buttonY = true;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    const size_t count = mapGamepadState(pressed, previous, out);

    TEST_ASSERT_EQUAL_size_t(1, count);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::SetExpression), static_cast<int>(out[0].type));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(Expression::Happy), static_cast<int>(out[0].expression));
    TEST_ASSERT_EQUAL_UINT32(200, out[0].durationMs);
}

void test_button_b_press_emits_sleep() {
    GamepadState previous;
    GamepadState pressed = previous;
    pressed.buttonB = true;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    const size_t count = mapGamepadState(pressed, previous, out);

    TEST_ASSERT_EQUAL_size_t(1, count);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Sleep), static_cast<int>(out[0].type));
    TEST_ASSERT_EQUAL_UINT32(500, out[0].durationMs);
}

void test_button_x_press_emits_wake() {
    GamepadState previous;
    GamepadState pressed = previous;
    pressed.buttonX = true;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    const size_t count = mapGamepadState(pressed, previous, out);

    TEST_ASSERT_EQUAL_size_t(1, count);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Wake), static_cast<int>(out[0].type));
    TEST_ASSERT_EQUAL_UINT32(500, out[0].durationMs);
}

void test_stick_and_two_buttons_pressed_simultaneously_emits_three_commands() {
    GamepadState previous;
    GamepadState current = previous;
    current.leftStickX = 0.5f;
    current.buttonA = true;
    current.leftBumper = true;
    std::array<EyeCommand, kMaxCommandsPerFrame> out{};

    const size_t count = mapGamepadState(current, previous, out);

    TEST_ASSERT_EQUAL_size_t(3, count);
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
