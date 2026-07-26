# OTA

**Purpose:** Over-the-air firmware updates — reflash without a USB cable.

**Responsibilities:** `OtaManager` wraps the global `ArduinoOTA` singleton:
`begin()` sets the hostname and starts the listener; `update()` calls
`ArduinoOTA.handle()` every frame (cheap no-op when idle).

**Future work:** No password/authentication on the OTA listener — matches
this project's LAN-local-hobby-device security posture (see
`docs/superpowers/specs/2026-07-25-v0.4-connectivity-design.md`'s
equivalent REST/WebSocket deferral); revisit together if a real threat
model ever emerges.
