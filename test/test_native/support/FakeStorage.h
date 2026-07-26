#pragma once

#include <map>
#include <string>

#include "../../../lib/Storage/IStorage.h"

/** In-memory IStorage double: four parallel maps, one per type, keyed by the
 * exact string key -- no cross-type key collisions possible since each type
 * has its own map, matching how ESP32 Preferences itself tracks a type per key. */
class FakeStorage : public eyesee::IStorage {
public:
    bool beginCalled = false;

    bool begin(const char* namespaceName) override {
        (void)namespaceName;
        beginCalled = true;
        return true;
    }

    bool getUInt16(const char* key, uint16_t& outValue) const override {
        auto it = uint16Values_.find(key);
        if (it == uint16Values_.end()) return false;
        outValue = it->second;
        return true;
    }
    bool putUInt16(const char* key, uint16_t value) override {
        uint16Values_[key] = value;
        return true;
    }

    bool getInt16(const char* key, int16_t& outValue) const override {
        auto it = int16Values_.find(key);
        if (it == int16Values_.end()) return false;
        outValue = it->second;
        return true;
    }
    bool putInt16(const char* key, int16_t value) override {
        int16Values_[key] = value;
        return true;
    }

    bool getBool(const char* key, bool& outValue) const override {
        auto it = boolValues_.find(key);
        if (it == boolValues_.end()) return false;
        outValue = it->second;
        return true;
    }
    bool putBool(const char* key, bool value) override {
        boolValues_[key] = value;
        return true;
    }

    bool getFloat(const char* key, float& outValue) const override {
        auto it = floatValues_.find(key);
        if (it == floatValues_.end()) return false;
        outValue = it->second;
        return true;
    }
    bool putFloat(const char* key, float value) override {
        floatValues_[key] = value;
        return true;
    }

    void end() override {
    }

private:
    std::map<std::string, uint16_t> uint16Values_;
    std::map<std::string, int16_t> int16Values_;
    std::map<std::string, bool> boolValues_;
    std::map<std::string, float> floatValues_;
};
