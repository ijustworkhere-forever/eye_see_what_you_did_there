#include <unity.h>

#include "support/FakeStorage.h"

void test_fake_storage_round_trips_every_type() {
    FakeStorage storage;
    TEST_ASSERT_TRUE(storage.begin("test"));
    TEST_ASSERT_TRUE(storage.beginCalled);

    TEST_ASSERT_TRUE(storage.putUInt16("u", 1500));
    uint16_t u = 0;
    TEST_ASSERT_TRUE(storage.getUInt16("u", u));
    TEST_ASSERT_EQUAL_UINT16(1500, u);

    TEST_ASSERT_TRUE(storage.putInt16("i", -42));
    int16_t i = 0;
    TEST_ASSERT_TRUE(storage.getInt16("i", i));
    TEST_ASSERT_EQUAL_INT16(-42, i);

    TEST_ASSERT_TRUE(storage.putBool("b", true));
    bool b = false;
    TEST_ASSERT_TRUE(storage.getBool("b", b));
    TEST_ASSERT_TRUE(b);

    TEST_ASSERT_TRUE(storage.putFloat("f", 30.5f));
    float f = 0.0f;
    TEST_ASSERT_TRUE(storage.getFloat("f", f));
    TEST_ASSERT_EQUAL_FLOAT(30.5f, f);
}

void test_fake_storage_missing_key_returns_false() {
    FakeStorage storage;
    uint16_t u = 0;
    TEST_ASSERT_FALSE(storage.getUInt16("missing", u));
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
