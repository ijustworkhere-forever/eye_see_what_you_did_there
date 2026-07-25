#include <unity.h>

#include "CommandQueue.h"

using eyesee::CommandQueue;
using eyesee::CommandType;
using eyesee::EyeCommand;

void test_push_pop_preserves_fifo_order() {
    CommandQueue queue;
    EyeCommand blink;
    blink.type = CommandType::Blink;
    EyeCommand wake;
    wake.type = CommandType::Wake;

    TEST_ASSERT_TRUE(queue.push(blink));
    TEST_ASSERT_TRUE(queue.push(wake));

    EyeCommand out;
    TEST_ASSERT_TRUE(queue.pop(out));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Blink), static_cast<int>(out.type));
    TEST_ASSERT_TRUE(queue.pop(out));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Wake), static_cast<int>(out.type));
    TEST_ASSERT_FALSE(queue.pop(out));
}

void test_push_fails_when_full() {
    CommandQueue queue;
    EyeCommand command;
    for (size_t i = 0; i < CommandQueue::kCapacity; ++i) {
        TEST_ASSERT_TRUE(queue.push(command));
    }
    TEST_ASSERT_TRUE(queue.full());
    TEST_ASSERT_FALSE(queue.push(command));
}

void test_clear_empties_queue() {
    CommandQueue queue;
    EyeCommand command;
    queue.push(command);
    queue.clear();
    TEST_ASSERT_TRUE(queue.empty());
    TEST_ASSERT_EQUAL_UINT32(0, static_cast<uint32_t>(queue.size()));
}

// No main() here — test/test_native/test_main.cpp is the sole file with main()
// (see its own comment for why).
