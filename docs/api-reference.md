# API Reference

This is the single source of truth for EyeSee's three network APIs: REST,
WebSocket, and MQTT. Most commands across REST and MQTT push `EyeCommand`s
into the same `CommandQueue` (see `docs/architecture.md`), but WebSocket is
broadcast-only and never processes client messages, and `POST /api/v1/config`
routes directly to `CalibrationManager` for static servo tuning. The
differences below are primarily in transport and payload shape; those that
do use the `CommandQueue` execute the same underlying behaviors.

## Versioning & stability

`/api/v1/` is frozen as of v1.0: no breaking change (a removed field, a
renamed field, a changed type, changed status-code semantics) will ever
land under the `v1` path. A breaking change ships as `/api/v2/` alongside
the still-working `v1`. Additive, backward-compatible changes (a new
optional request field, a new response field, a new route) may still land
under `v1` — the `firmwareVersion` field added in v1.0 is exactly this kind
of change. The MQTT and WebSocket protocols follow the same policy even
though they have no path-based version literal: "frozen" for them means
the same additive-only rule.

Every `GET /api/v1/status` response and every MQTT `<prefix>/status`
publish includes a `firmwareVersion` field (e.g. `"1.0.0"`) so a client can
confirm which frozen API version a given device is actually running.

## REST API

All routes are under `/api/v1/`. Every route that accepts a JSON body caps
it at 512 bytes; if exceeded, returns `413 Payload Too Large` with no body.
Malformed JSON syntax (not just a wrong shape) is rejected by the underlying
web server before any route handler runs, with no custom error body.

### `GET /api/v1/status`

No request body. `200 OK`:

```json
{
  "state": "Idle",
  "wifiConnected": true,
  "firmwareVersion": "1.0.0",
  "pose": {
    "lookX": 0.0, "lookY": 0.0,
    "upperLeftLid": 1.0, "lowerLeftLid": 1.0,
    "upperRightLid": 1.0, "lowerRightLid": 1.0
  }
}
```

`state` is one of: `Startup`, `Calibration`, `Manual`, `Idle`, `Tracking`,
`Sleeping`, `Disabled`, `Error`.

### `POST /api/v1/look`

Request body: `{"x": <float, required>, "y": <float, required>, "speed"?: <float, default 300.0>, "blinkOnArrival"?: <bool, default false>, "hold"?: <bool, default false>}`.
`x`/`y` are clamped to `[-1.0, 1.0]` server-side, not rejected if out of
range.

`202 Accepted`: `{"queued": true}`.
`400 Bad Request`: `{"error": "missing required field: x and y must be numbers"}` if either is missing or not numeric.
`503 Service Unavailable`: `{"error": "command queue full"}` if the 16-slot `CommandQueue` is full.

### `POST /api/v1/blink`

Request body: `{"durationMs"?: <uint32, default 150>}` — always succeeds, no required fields.

`202 Accepted`: `{"queued": true}`.
`503 Service Unavailable`: same shape as above, queue-full only.

### `POST /api/v1/wink`

Request body: `{"side": <"left"|"right", required>, "durationMs"?: <uint32, default 150>}`.

`202 Accepted`: `{"queued": true}`.
`400 Bad Request`: `{"error": "missing required field: side"}` or `{"error": "side must be \"left\" or \"right\""}`.
`503 Service Unavailable`: queue-full.

### `POST /api/v1/expression`

Request body: `{"expression": <string, required>, "durationMs"?: <uint32, default 200>}`.
`expression` is one of: `Neutral`, `Happy`, `Curious`, `Sleepy`, `Angry`, `Surprised`.

`202 Accepted`: `{"queued": true}`.
`400 Bad Request`: `{"error": "missing required field: expression"}` or `{"error": "unrecognized expression name"}`.
`503 Service Unavailable`: queue-full.

### `POST /api/v1/track`

Request body: `{"x": <float, required>, "y": <float, required>, "hold"?: <bool, default true>}`.
`x`/`y` clamped to `[-1.0, 1.0]` server-side, same as `look`. Note `hold`
defaults to `true` here, unlike `look`'s `false` default — this is
deliberate: a face-tracking client's target is assumed to still be tracked
unless it explicitly says otherwise, while a one-shot `look` is assumed
transient. See `docs/architecture.md`'s "Integrations (v0.6)" section for
the staleness-timeout behavior this drives.

`202 Accepted`: `{"queued": true}`.
`400 Bad Request`: `{"error": "missing required field: x and y must be numbers"}`.
`503 Service Unavailable`: queue-full.

### `POST /api/v1/sleep` / `POST /api/v1/wake`

No request body. Both hardcode a 500ms ease duration server-side.

`202 Accepted`: `{"queued": true}`.
`503 Service Unavailable`: queue-full.

### `GET /api/v1/config`

No request body. `200 OK`:

```json
{
  "lr": {"minPulseUs": 1000, "maxPulseUs": 2000, "neutralPulseUs": 1500, "mechanicalOffset": 0, "inverted": false, "mirrored": false},
  "ud": { "...": "same shape" },
  "tl": { "...": "same shape" },
  "bl": { "...": "same shape" },
  "tr": { "...": "same shape" },
  "br": { "...": "same shape" },
  "lookRangeDegrees": 30.0
}
```

`lr`/`ud`/`tl`/`bl`/`tr`/`br` are the six servo channels (left/right eye,
up/down eye, top/bottom-left eyelid, top/bottom-right eyelid).

### `POST /api/v1/config`

Request body — **every field is required, one channel per request; there
is no partial patch** (see `docs/known-limitations.md`):

```json
{
  "channel": "lr",
  "minPulseUs": 1000,
  "maxPulseUs": 2000,
  "neutralPulseUs": 1500,
  "mechanicalOffset": 0,
  "inverted": false,
  "mirrored": false
}
```

`channel` is one of `lr`/`ud`/`tl`/`bl`/`tr`/`br`. Validated server-side:
`500 <= minPulseUs < maxPulseUs <= 2500` and `minPulseUs <= neutralPulseUs
<= maxPulseUs`. On success the change is persisted to flash immediately
(no separate "save" step).

`200 OK`: the full updated config, same shape as `GET /api/v1/config`.
`400 Bad Request`: `{"error": "missing required field: channel"}`,
`{"error": "unrecognized channel name"}`,
`{"error": "missing required field: minPulseUs, maxPulseUs, neutralPulseUs, mechanicalOffset, inverted, and mirrored are all required"}`,
or `{"error": "invalid pulse range: require 500 <= minPulseUs < maxPulseUs <= 2500 and minPulseUs <= neutralPulseUs <= maxPulseUs"}`.

## WebSocket

Single endpoint: `ws://<device-ip>/ws`. Broadcast-only — the server never
processes any message a client sends to it. Broadcasts at up to 30Hz (skipped
entirely while no client is connected), one message per tick:

```json
{
  "state": "Idle",
  "uptimeMs": 123456,
  "pose": {
    "lookX": 0.0, "lookY": 0.0,
    "upperLeftLid": 1.0, "lowerLeftLid": 1.0,
    "upperRightLid": 1.0, "lowerRightLid": 1.0
  }
}
```

Deliberately leaner than `GET /api/v1/status`: omits `wifiConnected` and
`firmwareVersion` (bandwidth at 30Hz vs. once per REST request) and adds
`uptimeMs` (not present in the REST status response).

## MQTT

Two topics, both derived from a configured `topicPrefix` (default
`"eyesee"`, set in `include/MqttCredentials.h`):

- **Subscribe: `<prefix>/command`** — the device listens here.
- **Publish: `<prefix>/status`** — the device publishes here at 1Hz.

### `<prefix>/command` payload

`{"type": <string, required>, ...same fields as the matching REST route above}`.
`type` is one of `"look"`, `"blink"`, `"wink"`, `"expression"`, `"track"`,
`"sleep"`, `"wake"` — the remaining fields for each are identical to that
command's REST body (e.g. an MQTT `"type": "look"` message takes the same
`x`/`y`/`speed`/`blinkOnArrival`/`hold` fields as `POST /api/v1/look`).
`sleep`/`wake` need no other fields. A malformed payload or an unknown/
missing `type` is logged device-side and silently dropped — no error is
ever published back to any topic.

Example:

```json
{"type": "look", "x": 0.5, "y": -0.25}
```

### `<prefix>/status` payload

Identical shape to `GET /api/v1/status`'s response (including
`firmwareVersion`), published every second.

### Known MQTT-specific limitations

The device's MQTT client ID is hardcoded (`"eyesee"`) — see
`docs/known-limitations.md` for what this means if you run more than one
device against the same broker. `PubSubClient::connect()`'s residual
synchronous-blocking behavior on a slow-to-resolve broker hostname is
documented in `lib/Integrations/MqttBridge.h`'s doc-comment; deploy with
the broker's IP literal, not a hostname, to avoid it.
