#pragma once

#include <cstdint>

#ifdef ARDUINO

#include <PubSubClient.h>
#include <WiFi.h>

#include "CommandQueue.h"
#include "EyeController.h"
#include "IBehaviorEngine.h"
#include "WifiManager.h"

namespace eyesee {

/** Subscribes to <topicPrefix>/command for EyeCommands (parsed via
 * lib/Protocol/MqttCommandJson), publishes to <topicPrefix>/status at 1Hz. */
class MqttBridge {
public:
    MqttBridge(CommandQueue& commandQueue, const IBehaviorEngine& behaviorEngine,
               const EyeController& eyeController, const WifiManager& wifiManager);

    /** Configures the client and attempts an initial connection. Call once from
     * setup(), any time after WifiManager::begin(). */
    void begin(const char* brokerHost, uint16_t brokerPort, const char* topicPrefix);
    /** Services the connection: reconnects with backoff if dropped, pumps incoming
     * messages, and periodically publishes a status update. */
    void update(uint32_t deltaMs);

private:
    CommandQueue& commandQueue_;
    const IBehaviorEngine& behaviorEngine_;
    const EyeController& eyeController_;
    const WifiManager& wifiManager_;
    WiFiClient wifiClient_;
    PubSubClient mqttClient_;
    std::string commandTopic_;
    std::string statusTopic_;
    uint32_t msSinceLastReconnectAttempt_ = 0;
    uint32_t msSinceLastStatusPublish_ = 0;
    static constexpr uint32_t kReconnectIntervalMs = 5000;
    static constexpr uint32_t kStatusPublishPeriodMs = 1000;

    void handleMessage(char* topic, uint8_t* payload, unsigned int length);
};

}  // namespace eyesee

#endif  // ARDUINO
