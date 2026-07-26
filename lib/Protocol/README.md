# Protocol

**Purpose:** Pure JSON encode/decode for the Networking layer, with zero
Arduino dependency (only ArduinoJson, which is portable C++) — so this
library's own field-level validation (missing/wrong-type required fields,
unrecognized enum names, clamping, defaulting) and all response building
are fully covered by native tests. Raw JSON-syntax parsing of the HTTP
request body (malformed input, wrong Content-Type, oversize payloads) is
handled by ESPAsyncWebServer's `AsyncCallbackJsonWebHandler` before a
request ever reaches this library -- that boundary is ESP32/library
behavior, not exercised by native tests.

**Responsibilities:** `EyeStateJson.h` — `EyeState`/`Expression` string
mappings and response builders (`buildStatusJson`, `buildBroadcastJson`,
`buildErrorJson`). `EyeCommandJson.h` — request body parsing into
`EyeCommand` (`parseLookCommand`, `parseBlinkCommand`, `parseWinkCommand`,
`parseExpressionCommand`), each returning a `ParseResult{ok, error}`.
`EyeConfigJson.h` — `EyeChannel` string mappings and calibration request/
response handling (`parseConfigUpdate`, `buildConfigJson`).

**Consumed by:** `lib/Networking/RestApi` (every `EyeCommandJson`/
`EyeStateJson`/`EyeConfigJson` function except `buildBroadcastJson`) and
`lib/Networking/WebSocketServer` (`buildBroadcastJson` only).
