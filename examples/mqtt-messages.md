# Example MQTT messages

Assumes the default topic prefix `eyesee` (set via
`include/MqttCredentials.h`'s `kMqttTopicPrefix`) and a broker reachable at
`$BROKER` (e.g. `localhost` if using `examples/mosquitto-compose.yml`).

## Sending commands (publish to `eyesee/command`)

Look:
```bash
mosquitto_pub -h "$BROKER" -t eyesee/command -m '{"type": "look", "x": 0.5, "y": -0.25}'
```

Blink:
```bash
mosquitto_pub -h "$BROKER" -t eyesee/command -m '{"type": "blink"}'
```

Wink:
```bash
mosquitto_pub -h "$BROKER" -t eyesee/command -m '{"type": "wink", "side": "right"}'
```

Expression:
```bash
mosquitto_pub -h "$BROKER" -t eyesee/command -m '{"type": "expression", "expression": "Curious"}'
```

Track (face-tracking bridge):
```bash
mosquitto_pub -h "$BROKER" -t eyesee/command -m '{"type": "track", "x": 0.2, "y": 0.1, "hold": true}'
```

Sleep / wake:
```bash
mosquitto_pub -h "$BROKER" -t eyesee/command -m '{"type": "sleep"}'
mosquitto_pub -h "$BROKER" -t eyesee/command -m '{"type": "wake"}'
```

## Receiving status (subscribe to `eyesee/status`)

```bash
mosquitto_sub -h "$BROKER" -t eyesee/status
```

Publishes once per second:

```json
{"state": "Idle", "wifiConnected": true, "firmwareVersion": "1.0.0", "pose": {"lookX": 0.0, "lookY": 0.0, "upperLeftLid": 1.0, "lowerLeftLid": 1.0, "upperRightLid": 1.0, "lowerRightLid": 1.0}}
```
