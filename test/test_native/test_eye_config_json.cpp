#include <unity.h>

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

#include "EyeConfigJson.h"

using eyesee::channelFromString;
using eyesee::channelToString;
using eyesee::ConfigParseResult;
using eyesee::EyeChannel;
using eyesee::EyeConfig;
using eyesee::parseConfigUpdate;
using eyesee::ServoConfig;
using eyesee::buildConfigJson;

void test_channel_string_round_trips_every_value() {
    const EyeChannel values[] = {EyeChannel::Lr, EyeChannel::Ud, EyeChannel::Tl,
                                  EyeChannel::Bl, EyeChannel::Tr, EyeChannel::Br};
    for (EyeChannel value : values) {
        const char* name = channelToString(value);
        EyeChannel parsed = EyeChannel::Lr;
        TEST_ASSERT_TRUE(channelFromString(name, parsed));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(value), static_cast<int>(parsed));
    }
}

void test_channel_from_string_rejects_unknown_name() {
    EyeChannel parsed = EyeChannel::Lr;
    TEST_ASSERT_FALSE(channelFromString("xx", parsed));
}

void test_parse_config_update_reads_all_fields() {
    JsonDocument doc;
    doc["channel"] = "ud";
    doc["minPulseUs"] = 950;
    doc["maxPulseUs"] = 2050;
    doc["neutralPulseUs"] = 1500;
    doc["mechanicalOffset"] = -10;
    doc["inverted"] = true;
    doc["mirrored"] = false;

    const ConfigParseResult result = parseConfigUpdate(doc.as<JsonVariantConst>());

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeChannel::Ud), static_cast<int>(result.channel));
    TEST_ASSERT_EQUAL_UINT16(950, result.servoConfig.minPulseUs);
    TEST_ASSERT_EQUAL_UINT16(2050, result.servoConfig.maxPulseUs);
    TEST_ASSERT_EQUAL_UINT16(1500, result.servoConfig.neutralPulseUs);
    TEST_ASSERT_EQUAL_INT16(-10, result.servoConfig.mechanicalOffset);
    TEST_ASSERT_TRUE(result.servoConfig.inverted);
    TEST_ASSERT_FALSE(result.servoConfig.mirrored);
}

void test_parse_config_update_rejects_unknown_channel() {
    JsonDocument doc;
    doc["channel"] = "zz";
    doc["minPulseUs"] = 1000;
    doc["maxPulseUs"] = 2000;
    doc["neutralPulseUs"] = 1500;
    doc["mechanicalOffset"] = 0;
    doc["inverted"] = false;
    doc["mirrored"] = false;

    const ConfigParseResult result = parseConfigUpdate(doc.as<JsonVariantConst>());

    TEST_ASSERT_FALSE(result.ok);
}

void test_parse_config_update_rejects_missing_required_field() {
    JsonDocument doc;
    doc["channel"] = "lr";
    doc["minPulseUs"] = 1000;
    // maxPulseUs missing

    const ConfigParseResult result = parseConfigUpdate(doc.as<JsonVariantConst>());

    TEST_ASSERT_FALSE(result.ok);
    TEST_ASSERT_TRUE(result.error.size() > 0);
}

void test_parse_config_update_rejects_out_of_range_pulse_values() {
    JsonDocument doc;
    doc["channel"] = "lr";
    doc["minPulseUs"] = 65535;
    doc["maxPulseUs"] = 65535;
    doc["neutralPulseUs"] = 65535;
    doc["mechanicalOffset"] = 0;
    doc["inverted"] = false;
    doc["mirrored"] = false;

    const ConfigParseResult result = parseConfigUpdate(doc.as<JsonVariantConst>());

    TEST_ASSERT_FALSE(result.ok);
}

void test_build_config_json_reports_all_six_channels_and_look_range() {
    EyeConfig config;
    config.lr.minPulseUs = 900;
    config.ud.neutralPulseUs = 1600;
    config.lookRangeDegrees = 45.0f;

    const std::string json = buildConfigJson(config);

    JsonDocument doc;
    TEST_ASSERT_FALSE(deserializeJson(doc, json));
    TEST_ASSERT_EQUAL_UINT16(900, doc["lr"]["minPulseUs"].as<uint16_t>());
    TEST_ASSERT_EQUAL_UINT16(1600, doc["ud"]["neutralPulseUs"].as<uint16_t>());
    TEST_ASSERT_EQUAL_FLOAT(45.0f, doc["lookRangeDegrees"].as<float>());
    // Spot-check every channel key is present.
    TEST_ASSERT_FALSE(doc["tl"].isNull());
    TEST_ASSERT_FALSE(doc["bl"].isNull());
    TEST_ASSERT_FALSE(doc["tr"].isNull());
    TEST_ASSERT_FALSE(doc["br"].isNull());
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
