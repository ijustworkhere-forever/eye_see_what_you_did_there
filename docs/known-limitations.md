# Known Limitations

Deliberate scope boundaries this project has chosen not to close, each
with why it's intentional rather than an oversight. None of these are
planned for v1.0 — a stable release documents its limitations rather than
rushing to close all of them.

## Idle personality behaviors aren't wired in

`CuriousBehavior`/`RandomBehavior` exist and are tested (see
`lib/Behavior/README.md`) but neither is registered in `src/main.cpp` —
only `IdleBehavior` is. Swapping which idle personality runs is a future
config option, not something v1.0 exposes.

## `lookRangeDegrees` has no REST-exposed setter

`EyeConfig::lookRangeDegrees` persists correctly across reboots but can
currently only be set via `CalibrationManager`'s explicit-`EyeConfig`
constructor, not through `POST /api/v1/config` (see
`lib/Configuration/README.md` and `docs/api-reference.md`). Every other
calibration field is REST-configurable; this one isn't yet.

## OTA has no authentication

`ArduinoOTA`'s listener accepts any update with no password (see
`lib/OTA/README.md`). This matches this project's LAN-local-hobby-device
security posture (the same reasoning already applied to the REST/WebSocket
API since v0.4) — revisit together if a real threat model ever emerges for
this deployment context.

## MQTT client ID is hardcoded

The device always connects to its broker as client ID `"eyesee"` (see
`docs/api-reference.md`'s MQTT section). Two EyeSee devices pointed at the
same broker would collide — most MQTT brokers disconnect the older
connection when a duplicate client ID connects. Running more than one
device against one broker today requires giving each a different
`kMqttTopicPrefix` in its own `include/MqttCredentials.h` as a workaround;
a real per-device unique client ID is a future improvement.

## `POST /api/v1/config` has no partial patch

Every field (`minPulseUs`, `maxPulseUs`, `neutralPulseUs`,
`mechanicalOffset`, `inverted`, `mirrored`) is required on every config
update, for one channel at a time (see `docs/api-reference.md`) — you
cannot change just one field without re-sending all six.

## MQTT broker connection can still briefly block the frame loop

`PubSubClient::connect()` is synchronous. This is mitigated (1-second
connect and socket timeouts, gated on WiFi already being connected — see
`lib/Integrations/MqttBridge.h`'s doc-comment) but not eliminated: a
broker configured by a slow-to-resolve hostname, or one that accepts a TCP
connection but never completes the MQTT handshake, can still stall the
100Hz animation loop briefly. Deploy with the broker's IP literal, not a
hostname, to avoid the worst case.

## WiFi credentials are compile-time-only

`WifiManager::begin()` is handed a plain `ssid`/`password` pair from
`include/WifiCredentials.h` (gitignored, copied from
`WifiCredentials.h.example` and filled in with real values that get baked
into the binary at build time — see the root `README.md`'s Building
section). There is no runtime reconfiguration, no AP-mode fallback, and no
captive portal for first-time setup (see `lib/Networking/README.md`'s
"Future work" section). Moving the device to a different network requires
editing `WifiCredentials.h` and reflashing. This mirrors the project's
`include/*Credentials.h.example` pattern already used for both WiFi and
MQTT credentials, and matches this project's overall LAN-local-hobby-device
posture already stated above for OTA/REST/WebSocket auth.
