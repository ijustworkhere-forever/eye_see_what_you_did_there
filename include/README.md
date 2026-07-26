# include/

Project-wide public headers shared across `lib/` modules and
`src/main.cpp`. Holds two committed templates — copy each to its
gitignored real filename with real values before building for real
hardware:

- `WifiCredentials.h.example` -> `WifiCredentials.h`: `kWifiSsid`,
  `kWifiPassword`.
- `MqttCredentials.h.example` -> `MqttCredentials.h`: `kMqttBrokerHost`,
  `kMqttBrokerPort`, `kMqttTopicPrefix`.

CI provides placeholder values for both when building `esp32dev`, so
neither is required for the build to succeed — only for the device to
actually reach a real WiFi network or MQTT broker at runtime.

Also holds `Version.h` (not gitignored — this one is a real, committed
source file, not a template): defines `kFirmwareVersion`, the single
source of truth surfaced in every status response (see
`docs/api-reference.md`).
