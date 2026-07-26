# Networking

**Purpose:** Host the Web UI, the REST API, and a real-time WebSocket feed.

**Responsibilities:** `WifiManager` (STA connect + retry), `WebServer`
(LittleFS static assets), `RestApi` (`/api/v1/*` routes — `status`, `look`,
`blink`, `wink`, `expression`, `sleep`, `wake`, `config`), `WebSocketServer` (~30Hz
state broadcast at `/ws`). Every route reads existing read-only state
(`IBehaviorEngine::state()`, `EyeController::currentPose()`), pushes an
`EyeCommand` into the shared `CommandQueue`, or (one exception, `config`
only) calls `CalibrationManager::setServoConfig()` directly — calibration
is static tuning, not a live-motion command, so it doesn't belong in the
`CommandQueue` pipeline (see `docs/architecture.md`'s Persistence & OTA
section). No route touches `IAnimationEngine`/hardware directly. All four
classes share one `AsyncWebServer` instance, constructed in `main.cpp`.

**JSON handling:** request parsing and response building are pure logic in
`lib/Protocol` (native-tested); this module only wires that logic to
ESPAsyncWebServer's routes and callbacks, and is itself native-ignored
(`platformio.ini`'s `lib_ignore`) since ESPAsyncWebServer/AsyncTCP are
ESP32-only — verified by `pio run -e esp32dev` compiling successfully.

**Future work:** WiFi credentials via `PreferencesStore` instead of
compile-time `WifiCredentials.h` — deferred again past v0.5, since nothing
in this milestone's actual scope required it; AP-mode fallback/captive
portal for first-time setup (no milestone yet).
