# Protocol

**Purpose:** Pure JSON encode/decode for the Networking layer, with zero
Arduino dependency (only ArduinoJson, which is portable C++) — so request
parsing and response building are fully covered by native tests, unlike
`lib/Networking`'s ESPAsyncWebServer-bound wiring, which can't run on the
host.

**Responsibilities:** `EyeStateJson.h` — `EyeState`/`Expression` string
mappings and response builders (`buildStatusJson`, `buildBroadcastJson`,
`buildErrorJson`). `EyeCommandJson.h` — request body parsing into
`EyeCommand` (`parseLookCommand`, `parseBlinkCommand`, `parseWinkCommand`,
`parseExpressionCommand`), each returning a `ParseResult{ok, error}`.

**Consumed by:** `lib/Networking/RestApi` (all six) and
`lib/Networking/WebSocketServer` (`buildBroadcastJson` only).
