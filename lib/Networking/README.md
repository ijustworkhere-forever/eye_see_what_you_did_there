# Networking

**Purpose:** Host the Web UI, the REST API, and a real-time WebSocket feed —
none implemented yet.

**Responsibilities:** `WebServer` (static assets), `RestApi` (versioned
`/api/v1/*` routes), `WebSocketServer` (30-60Hz state broadcast). Every
handler, once implemented, pushes `EyeCommand`s into the shared
`CommandQueue` rather than touching `EyeController`/`IAnimationEngine`
directly.

**Planned features:** See `docs/ROADMAP.md` v0.4.

**Future work:** No networking library is declared in `platformio.ini` yet
— adding one (e.g. ESPAsyncWebServer) is the first step of implementing
this module for real.
