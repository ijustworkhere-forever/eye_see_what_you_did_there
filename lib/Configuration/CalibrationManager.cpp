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

ServoConfig& channelConfig(EyeConfig& config, EyeChannel channel) {
    switch (channel) {
        case EyeChannel::Lr: return config.lr;
        case EyeChannel::Ud: return config.ud;
        case EyeChannel::Tl: return config.tl;
        case EyeChannel::Bl: return config.bl;
        case EyeChannel::Tr: return config.tr;
        case EyeChannel::Br: return config.br;
    }
    return config.lr;
}

const ServoConfig& channelConfig(const EyeConfig& config, EyeChannel channel) {
    switch (channel) {
        case EyeChannel::Lr: return config.lr;
        case EyeChannel::Ud: return config.ud;
        case EyeChannel::Tl: return config.tl;
        case EyeChannel::Bl: return config.bl;
        case EyeChannel::Tr: return config.tr;
        case EyeChannel::Br: return config.br;
    }
    return config.lr;
}

}  // namespace

#ifdef ARDUINO
namespace {
/** RAII critical-section guard around the ESP32 portMUX spinlock (same pattern
 * as lib/Behavior/CommandQueue.cpp). */
class CriticalSection {
public:
    explicit CriticalSection(portMUX_TYPE& mux) : mux_(mux) {
        portENTER_CRITICAL(&mux_);
    }
    ~CriticalSection() {
        portEXIT_CRITICAL(&mux_);
    }

private:
    portMUX_TYPE& mux_;
};
}  // namespace
#define EYESEE_CALIBRATION_LOCK() CriticalSection lock(mux_)
#else
#define EYESEE_CALIBRATION_LOCK()
#endif

CalibrationManager::CalibrationManager() : config_(makeDefaultEyeConfig()) {
}

CalibrationManager::CalibrationManager(const EyeConfig& initialConfig) : config_(initialConfig) {
}

EyeConfig CalibrationManager::eyeConfig() const {
    EYESEE_CALIBRATION_LOCK();
    return config_;
}

ServoConfig CalibrationManager::servoConfig(EyeChannel channel) const {
    EYESEE_CALIBRATION_LOCK();
    return constServoConfig(channel);
}

const ServoConfig& CalibrationManager::constServoConfig(EyeChannel channel) const {
    return channelConfig(config_, channel);
}

void CalibrationManager::setServoConfig(EyeChannel channel, const ServoConfig& config) {
    EYESEE_CALIBRATION_LOCK();
    mutableServoConfig(channel) = config;
}

ServoConfig& CalibrationManager::mutableServoConfig(EyeChannel channel) {
    return channelConfig(config_, channel);
}

bool CalibrationManager::loadFromStorage(IStorage& storage) {
    // Snapshot config_ under a short lock, do all flash I/O against the local
    // copy with no lock held, then apply the result back under a second short
    // lock -- see saveToStorage()'s comment for why holding the lock across
    // flash I/O is unsafe. This method only ever runs once at boot before
    // WiFi/BLE are active, but it's kept symmetric with saveToStorage() rather
    // than relying on that being true forever.
    EyeConfig localConfig;
    {
        EYESEE_CALIBRATION_LOCK();
        localConfig = config_;
    }

    bool loadedAny = false;
    for (const ChannelKeys& keys : kChannelKeys) {
        ServoConfig config = channelConfig(localConfig, keys.channel);
        if (storage.getUInt16(keys.minKey, config.minPulseUs)) loadedAny = true;
        if (storage.getUInt16(keys.maxKey, config.maxPulseUs)) loadedAny = true;
        if (storage.getUInt16(keys.neutralKey, config.neutralPulseUs)) loadedAny = true;
        if (storage.getInt16(keys.offsetKey, config.mechanicalOffset)) loadedAny = true;
        if (storage.getBool(keys.invertedKey, config.inverted)) loadedAny = true;
        if (storage.getBool(keys.mirroredKey, config.mirrored)) loadedAny = true;
        channelConfig(localConfig, keys.channel) = config;
    }
    if (storage.getFloat(kLookRangeKey, localConfig.lookRangeDegrees)) loadedAny = true;

    EYESEE_CALIBRATION_LOCK();
    config_ = localConfig;
    return loadedAny;
}

void CalibrationManager::saveToStorage(IStorage& storage) const {
    // Snapshot config_ under a short lock, then do all flash I/O outside it.
    // portENTER_CRITICAL disables interrupts (and spins the other core) on
    // ESP32 -- holding it across 37 sequential flash writes (each of
    // PreferencesStore's put*() calls commits to NVS individually) starves
    // WiFi/BLE's interrupt-driven timing for the whole duration. Confirmed on
    // real hardware: saving calibration while WiFi/BLE were active hung the
    // device badly enough to need a hard power cycle, not just a reset. This
    // lock's only job is protecting config_ itself, matching
    // eyeConfig()/servoConfig()'s existing by-value-snapshot pattern -- it
    // must never wrap I/O.
    //
    // Trade-off: a concurrent setServoConfig() between the snapshot and the
    // writes below would persist a stale value. Nothing in this codebase
    // calls setServoConfig() concurrently with saveToStorage() today (both
    // run sequentially on RestApi's single config-POST handler), so this is
    // an acceptable, deliberate trade against the alternative of freezing the
    // radio stack.
    EyeConfig snapshot;
    {
        EYESEE_CALIBRATION_LOCK();
        snapshot = config_;
    }
    for (const ChannelKeys& keys : kChannelKeys) {
        const ServoConfig& config = channelConfig(snapshot, keys.channel);
        storage.putUInt16(keys.minKey, config.minPulseUs);
        storage.putUInt16(keys.maxKey, config.maxPulseUs);
        storage.putUInt16(keys.neutralKey, config.neutralPulseUs);
        storage.putInt16(keys.offsetKey, config.mechanicalOffset);
        storage.putBool(keys.invertedKey, config.inverted);
        storage.putBool(keys.mirroredKey, config.mirrored);
    }
    storage.putFloat(kLookRangeKey, snapshot.lookRangeDegrees);
}

}  // namespace eyesee
