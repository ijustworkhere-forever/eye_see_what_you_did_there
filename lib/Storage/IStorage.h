#pragma once

#include <cstdint>

namespace eyesee {

/** Swappable key/value persistence backend. Every key must be <=15 characters
 * (ESP32 NVS/Preferences' hard limit). */
class IStorage {
public:
    virtual ~IStorage() = default;
    virtual bool begin(const char* namespaceName) = 0;
    virtual bool getUInt16(const char* key, uint16_t& outValue) const = 0;
    virtual bool putUInt16(const char* key, uint16_t value) = 0;
    virtual bool getInt16(const char* key, int16_t& outValue) const = 0;
    virtual bool putInt16(const char* key, int16_t value) = 0;
    virtual bool getBool(const char* key, bool& outValue) const = 0;
    virtual bool putBool(const char* key, bool value) = 0;
    virtual bool getFloat(const char* key, float& outValue) const = 0;
    virtual bool putFloat(const char* key, float value) = 0;
    virtual void end() = 0;
};

}  // namespace eyesee
