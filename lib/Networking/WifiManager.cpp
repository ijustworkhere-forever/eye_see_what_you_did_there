#include "WifiManager.h"

#include <WiFi.h>
#include <soc/rtc_cntl_reg.h>
#include <soc/soc.h>

#include "Logger.h"

namespace eyesee {

namespace {
constexpr const char* kLogTag = "WifiManager";
}  // namespace

void WifiManager::begin(const char* ssid, const char* password) {
    // WiFi.mode()'s radio power-on causes a very brief voltage dip that this
    // board's brownout detector treats as a fault, even though the supply
    // recovers within milliseconds -- confirmed empirically (this exact call
    // alone trips it, independent of TX power or any connection attempt) and
    // widely reported for this exact scenario (see e.g.
    // espressif/arduino-esp32#863). Disable the detector only around this one
    // call, then restore its prior state immediately after -- this does NOT
    // disable brownout protection for the rest of runtime, so a genuine
    // sustained undervoltage later (six servos drawing heavily, a real supply
    // fault) is still caught. If this firmware still resets during normal
    // operation (not at boot), that's real undervoltage, not this transient --
    // see docs/hardware.md's power-budget section.
    const uint32_t previousBrownoutConfig = READ_PERI_REG(RTC_CNTL_BROWN_OUT_REG);
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    WiFi.mode(WIFI_STA);
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, previousBrownoutConfig);

    // Also reduces steady-state radio power draw a bit -- harmless for range
    // on a same-room hobby device, but not what fixes the boot-time brownout
    // above (that's the radio's power-on transient, not transmit power).
    WiFi.setTxPower(WIFI_POWER_11dBm);
    WiFi.begin(ssid, password);
    Logger::info(kLogTag, "Connecting to WiFi");
}

void WifiManager::update(uint32_t deltaMs) {
    if (isConnected()) {
        msSinceLastAttempt_ = 0;
        return;
    }
    msSinceLastAttempt_ += deltaMs;
    if (msSinceLastAttempt_ >= kRetryIntervalMs) {
        msSinceLastAttempt_ = 0;
        Logger::info(kLogTag, "WiFi not connected, retrying");
        WiFi.reconnect();
    }
}

bool WifiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

}  // namespace eyesee
