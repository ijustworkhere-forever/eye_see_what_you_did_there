#include "CalibrationManager.h"

namespace eyesee {

CalibrationManager::CalibrationManager() : config_(makeDefaultEyeConfig()) {}

CalibrationManager::CalibrationManager(const EyeConfig& initialConfig) : config_(initialConfig) {}

const EyeConfig& CalibrationManager::eyeConfig() const {
    return config_;
}

const ServoConfig& CalibrationManager::servoConfig(EyeChannel channel) const {
    switch (channel) {
        case EyeChannel::LR: return config_.lr;
        case EyeChannel::UD: return config_.ud;
        case EyeChannel::TL: return config_.tl;
        case EyeChannel::BL: return config_.bl;
        case EyeChannel::TR: return config_.tr;
        case EyeChannel::BR: return config_.br;
    }
    return config_.lr;
}

void CalibrationManager::setServoConfig(EyeChannel channel, const ServoConfig& config) {
    mutableServoConfig(channel) = config;
    // TODO: persist via IStorage (docs/ROADMAP.md v0.5) — in-memory only this pass.
}

ServoConfig& CalibrationManager::mutableServoConfig(EyeChannel channel) {
    switch (channel) {
        case EyeChannel::LR: return config_.lr;
        case EyeChannel::UD: return config_.ud;
        case EyeChannel::TL: return config_.tl;
        case EyeChannel::BL: return config_.bl;
        case EyeChannel::TR: return config_.tr;
        case EyeChannel::BR: return config_.br;
    }
    return config_.lr;
}

}  // namespace eyesee
