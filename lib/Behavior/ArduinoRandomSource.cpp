#include "ArduinoRandomSource.h"

#ifdef ARDUINO

#include <Arduino.h>

namespace eyesee {

float ArduinoRandomSource::nextFloat(float min, float max) {
    // random(min, max) only accepts longs; scale a wide-range random() call instead.
    const long scaled = random(0, 1000000L);
    const float t = static_cast<float>(scaled) / 1000000.0f;
    return min + t * (max - min);
}

uint32_t ArduinoRandomSource::nextUInt(uint32_t min, uint32_t max) {
    return static_cast<uint32_t>(random(static_cast<long>(min), static_cast<long>(max) + 1));
}

}  // namespace eyesee

#endif  // ARDUINO
