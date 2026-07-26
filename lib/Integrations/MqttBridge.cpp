#include "MqttBridge.h"

#ifdef ARDUINO

#include "EyeStateJson.h"
#include "Logger.h"
#include "MqttCommandJson.h"

namespace eyesee {

namespace {
constexpr const char* kLogTag = "MqttBridge";
}  // namespace

MqttBridge::MqttBridge(CommandQueue& commandQueue, const IBehaviorEngine& behaviorEngine,
                       const EyeController& eyeController, const WifiManager& wifiManager)
    : commandQueue_(commandQueue),
      behaviorEngine_(behaviorEngine),
      eyeController_(eyeController),
      wifiManager_(wifiManager),
      mqttClient_(wifiClient_) {
    // Cuts the worst-case blocking TCP connect attempt (inside PubSubClient::connect(),
    // fully synchronous) from WiFiClient's 3000ms default down to 1000ms, so an
    // unreachable broker can't freeze the 100Hz animation pipeline for as long.
    wifiClient_.setTimeout(1);
    // PubSubClient's own read-response wait (after TCP connects) busy-loops with no
    // yield() for up to socketTimeout seconds -- cap that too, or a broker that accepts
    // the TCP connection but never speaks MQTT would still freeze the frame loop for the
    // 15s default.
    mqttClient_.setSocketTimeout(1);
}

void MqttBridge::begin(const char* brokerHost, uint16_t brokerPort, const char* topicPrefix) {
    commandTopic_ = std::string(topicPrefix) + "/command";
    statusTopic_ = std::string(topicPrefix) + "/status";

    mqttClient_.setServer(brokerHost, brokerPort);
    mqttClient_.setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
        handleMessage(topic, payload, length);
    });
}

void MqttBridge::update(uint32_t deltaMs) {
    if (!mqttClient_.connected()) {
        msSinceLastReconnectAttempt_ += deltaMs;
        if (msSinceLastReconnectAttempt_ >= kReconnectIntervalMs) {
            msSinceLastReconnectAttempt_ = 0;
            if (!wifiManager_.isConnected()) {
                return;
            }
            if (mqttClient_.connect("eyesee")) {
                Logger::info(kLogTag, "MQTT connected");
                if (!mqttClient_.subscribe(commandTopic_.c_str())) {
                    Logger::error(kLogTag, "MQTT subscribe failed");
                }
            } else {
                Logger::info(kLogTag, "MQTT connect failed, will retry");
            }
        }
        return;
    }
    msSinceLastReconnectAttempt_ = 0;
    mqttClient_.loop();

    msSinceLastStatusPublish_ += deltaMs;
    if (msSinceLastStatusPublish_ >= kStatusPublishPeriodMs) {
        msSinceLastStatusPublish_ = 0;
        const std::string body = buildStatusJson(
            behaviorEngine_.state(), eyeController_.currentPose(), wifiManager_.isConnected());
        if (!mqttClient_.publish(statusTopic_.c_str(), body.c_str())) {
            Logger::error(kLogTag, "MQTT status publish failed");
        }
    }
}

void MqttBridge::handleMessage(char* topic, uint8_t* payload, unsigned int length) {
    (void)topic;  // only one topic is ever subscribed to
    JsonDocument doc;
    if (deserializeJson(doc, payload, length)) {
        Logger::error(kLogTag, "malformed MQTT payload");
        return;
    }
    const MqttParseResult result = parseMqttCommand(doc.as<JsonVariantConst>());
    if (!result.ok) {
        Logger::error(kLogTag, result.error.c_str());
        return;
    }
    if (!commandQueue_.push(result.command)) {
        Logger::error(kLogTag, "command queue full, dropped MQTT command");
    }
}

}  // namespace eyesee

#endif  // ARDUINO
