#pragma once

#include <Preferences.h>

#include "IStorage.h"

namespace eyesee {

/** IStorage implementation wrapping the ESP32 Preferences API (flash-backed key/value store). */
class PreferencesStore : public IStorage {
public:
    bool begin(const char* namespaceName) override;
    bool getUInt16(const char* key, uint16_t& outValue) const override;
    bool putUInt16(const char* key, uint16_t value) override;
    bool getInt16(const char* key, int16_t& outValue) const override;
    bool putInt16(const char* key, int16_t value) override;
    bool getBool(const char* key, bool& outValue) const override;
    bool putBool(const char* key, bool value) override;
    bool getFloat(const char* key, float& outValue) const override;
    bool putFloat(const char* key, float value) override;
    void end() override;

private:
    mutable Preferences preferences_;
};

}  // namespace eyesee
