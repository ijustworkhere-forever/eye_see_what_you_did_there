#include "OtaManager.h"

#include <ArduinoOTA.h>

namespace eyesee {

void OtaManager::begin() {
    ArduinoOTA.setHostname("eyesee");
    ArduinoOTA.begin();
}

void OtaManager::update(uint32_t deltaMs) {
    (void)deltaMs;
    ArduinoOTA.handle();
}

}  // namespace eyesee
