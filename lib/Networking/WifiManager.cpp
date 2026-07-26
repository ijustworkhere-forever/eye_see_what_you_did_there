#include "WifiManager.h"

#include <WiFi.h>

#include "Logger.h"

namespace eyesee {

namespace {
constexpr const char* kLogTag = "WifiManager";
}  // namespace

void WifiManager::begin(const char* ssid, const char* password) {
    WiFi.mode(WIFI_STA);
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
