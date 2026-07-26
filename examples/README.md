# examples/

Runnable example configs and scripts demonstrating the firmware's REST,
MQTT, and calibration APIs (full reference: `docs/api-reference.md`).

- `calibration.json.example` — one complete `POST /api/v1/config` request
  body, ready to `curl -d @calibration.json.example`.
- `rest-requests.sh` — one example `curl` invocation per REST route;
  `EYESEE_HOST=<device-ip> ./rest-requests.sh` runs them all against a
  real device.
- `mqtt-messages.md` — sample `mosquitto_pub`/`mosquitto_sub` invocations
  for every MQTT command type and the status topic.
- `mosquitto-compose.yml` — a disposable local MQTT broker
  (`docker compose -f mosquitto-compose.yml up`) for testing the MQTT
  integration without a production broker. Anonymous access, local
  testing only — not a production deployment recommendation.
