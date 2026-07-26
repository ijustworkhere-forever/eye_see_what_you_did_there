#pragma once

#include "IRandomSource.h"

// Arduino-only — this class has no native/host build, matching
// Pca9685ServoOutput's pattern for a library folder that's otherwise
// native-testable (lib/Behavior/ is not ignored by the native environment).
#ifdef ARDUINO

namespace eyesee {

/** IRandomSource backed by Arduino's random(). */
class ArduinoRandomSource : public IRandomSource {
public:
    float nextFloat(float min, float max) override;
    uint32_t nextUInt(uint32_t min, uint32_t max) override;
};

}  // namespace eyesee

#endif  // ARDUINO
