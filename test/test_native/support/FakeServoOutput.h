#pragma once

#include "IServoOutput.h"

class FakeServoOutput : public eyesee::IServoOutput {
public:
    eyesee::ServoOutput lastWrite{};
    int writeCallCount = 0;

    void write(const eyesee::ServoOutput& output) override {
        lastWrite = output;
        ++writeCallCount;
    }
    void moveServo(uint8_t, float) override {}
    void setAngle(uint8_t, float) override {}
    void setPulse(uint8_t, uint16_t) override {}
    void update(uint32_t) override {}
};
