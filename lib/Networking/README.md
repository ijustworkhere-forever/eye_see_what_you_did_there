# Networking

**Purpose:** Host the Web UI, the REST API, and a real-time WebSocket feed.

**Responsibilities:** `WifiManager` (STA connect + retry), `WebServer`
(LittleFS static assets), `RestApi` (`/api/v1/*` routes — `status`, `look`,
`blink`, `wink`, `expression`, `sleep`, `wake`), `WebSocketServer` (~30Hz
state broadcast at `/ws`). Every route either reads existing read-only
state (`IBehaviorEngine::state()`, `EyeController::currentPose()`) or
pushes an `EyeCommand` into the shared `CommandQueue` — never touching
`IAnimationEngine`/hardware directly. All four classes share one
`AsyncWebServer` instance, constructed in `main.cpp`.

**JSON handling:** request parsing and response building are pure logic in
`lib/Protocol` (native-tested); this module only wires that logic to
ESPAsyncWebServer's routes and callbacks, and is itself native-ignored
(`platformio.ini`'s `lib_ignore`) since ESPAsyncWebServer/AsyncTCP are
ESP32-only — verified by `pio run -e esp32dev` compiling successfully.

**Future work:** `GET`/`POST /api/v1/config` (v0.5, once `CalibrationManager`
persists via `PreferencesStore`); WiFi credentials via `PreferencesStore`
instead of compile-time `WifiCredentials.h` (v0.5, same reason); AP-mode
fallback/captive portal for first-time setup (no milestone yet).
