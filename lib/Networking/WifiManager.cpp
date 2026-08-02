#include "WifiManager.h"

#include <WiFi.h>

#include "Logger.h"

namespace eyesee {

namespace {
constexpr const char* kLogTag = "WifiManager";
}  // namespace

void WifiManager::begin(const char* ssid, const char* password) {
    WiFi.mode(WIFI_STA);
    // Reduced from the 19.5dBm default: the radio's power-on/calibration burst
    // during WiFi.begin() was tripping the ESP32's brownout detector on
    // marginal power supplies. Lower TX power draws less current during that
    // burst -- plenty of range for a same-room hobby device. Raise this back
    // toward WIFI_POWER_19_5dBm if you've since fixed power delivery (a bulk
    // capacitor across 5V/GND, or a better-quality supply) and want full range.
    WiFi.setTxPower(WIFI_POWER_11dBm);

    // TEMPORARY DIAGNOSTIC: skip only the actual connection attempt, not
    // WiFi.mode() above -- that call alone brings up the TCP/IP stack (lwIP)
    // that AsyncTCP/ESPAsyncWebServer need at server.begin(), which a full
    // WiFi skip broke (assert failed: tcpip_api_call, "Invalid mbox"). This
    // isolates whether it's specifically WiFi.begin()'s RF connect/probe
    // burst -- not just the radio/network stack existing at all -- that
    // trips the brownout. Revert once confirmed either way.
    (void)ssid;
    (void)password;
    // WiFi.begin(ssid, password);
    Logger::info(kLogTag, "WiFi.begin() SKIPPED for diagnostic test (mode set, not connecting)");
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
