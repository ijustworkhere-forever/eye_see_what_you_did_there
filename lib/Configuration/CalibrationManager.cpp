#include "CalibrationManager.h"

namespace eyesee {

namespace {

struct ChannelKeys {
    EyeChannel channel;
    const char* minKey;
    const char* maxKey;
    const char* neutralKey;
    const char* offsetKey;
    const char* invertedKey;
    const char* mirroredKey;
};

constexpr ChannelKeys kChannelKeys[] = {
    {EyeChannel::Lr, "lr_min", "lr_max", "lr_neu", "lr_off", "lr_inv", "lr_mir"},
    {EyeChannel::Ud, "ud_min", "ud_max", "ud_neu", "ud_off", "ud_inv", "ud_mir"},
    {EyeChannel::Tl, "tl_min", "tl_max", "tl_neu", "tl_off", "tl_inv", "tl_mir"},
    {EyeChannel::Bl, "bl_min", "bl_max", "bl_neu", "bl_off", "bl_inv", "bl_mir"},
    {EyeChannel::Tr, "tr_min", "tr_max", "tr_neu", "tr_off", "tr_inv", "tr_mir"},
    {EyeChannel::Br, "br_min", "br_max", "br_neu", "br_off", "br_inv", "br_mir"},
};

constexpr const char* kLookRangeKey = "lookRange";

}  // namespace

CalibrationManager::CalibrationManager() : config_(makeDefaultEyeConfig()) {
}

CalibrationManager::CalibrationManager(const EyeConfig& initialConfig) : config_(initialConfig) {
}

const EyeConfig& CalibrationManager::eyeConfig() const {
    return config_;
}

const ServoConfig& CalibrationManager::servoConfig(EyeChannel channel) const {
    switch (channel) {
        case EyeChannel::Lr: return config_.lr;
        case EyeChannel::Ud: return config_.ud;
        case EyeChannel::Tl: return config_.tl;
        case EyeChannel::Bl: return config_.bl;
        case EyeChannel::Tr: return config_.tr;
        case EyeChannel::Br: return config_.br;
    }
    return config_.lr;
}

void CalibrationManager::setServoConfig(EyeChannel channel, const ServoConfig& config) {
    mutableServoConfig(channel) = config;
    // TODO: persist via IStorage (docs/ROADMAP.md v0.5) — in-memory only this pass.
}

ServoConfig& CalibrationManager::mutableServoConfig(EyeChannel channel) {
    switch (channel) {
        case EyeChannel::Lr: return config_.lr;
        case EyeChannel::Ud: return config_.ud;
        case EyeChannel::Tl: return config_.tl;
        case EyeChannel::Bl: return config_.bl;
        case EyeChannel::Tr: return config_.tr;
        case EyeChannel::Br: return config_.br;
    }
    return config_.lr;
}

bool CalibrationManager::loadFromStorage(IStorage& storage) {
    bool loadedAny = false;
    for (const ChannelKeys& keys : kChannelKeys) {
        ServoConfig config = servoConfig(keys.channel);
        if (storage.getUInt16(keys.minKey, config.minPulseUs)) loadedAny = true;
        if (storage.getUInt16(keys.maxKey, config.maxPulseUs)) loadedAny = true;
        if (storage.getUInt16(keys.neutralKey, config.neutralPulseUs)) loadedAny = true;
        if (storage.getInt16(keys.offsetKey, config.mechanicalOffset)) loadedAny = true;
        if (storage.getBool(keys.invertedKey, config.inverted)) loadedAny = true;
        if (storage.getBool(keys.mirroredKey, config.mirrored)) loadedAny = true;
        mutableServoConfig(keys.channel) = config;
    }
    if (storage.getFloat(kLookRangeKey, config_.lookRangeDegrees)) loadedAny = true;
    return loadedAny;
}

void CalibrationManager::saveToStorage(IStorage& storage) const {
    for (const ChannelKeys& keys : kChannelKeys) {
        const ServoConfig& config = servoConfig(keys.channel);
        storage.putUInt16(keys.minKey, config.minPulseUs);
        storage.putUInt16(keys.maxKey, config.maxPulseUs);
        storage.putUInt16(keys.neutralKey, config.neutralPulseUs);
        storage.putInt16(keys.offsetKey, config.mechanicalOffset);
        storage.putBool(keys.invertedKey, config.inverted);
        storage.putBool(keys.mirroredKey, config.mirrored);
    }
    storage.putFloat(kLookRangeKey, config_.lookRangeDegrees);
}

}  // namespace eyesee
