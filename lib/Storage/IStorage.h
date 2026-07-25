#pragma once

#include <cstdint>

namespace eyesee {

/** Swappable key/value persistence backend. */
class IStorage {
public:
    virtual ~IStorage() = default;
    virtual bool begin(const char* namespaceName) = 0;
    virtual bool getUInt16(const char* key, uint16_t& outValue) const = 0;
    virtual bool putUInt16(const char* key, uint16_t value) = 0;
    virtual void end() = 0;
};

}  // namespace eyesee
