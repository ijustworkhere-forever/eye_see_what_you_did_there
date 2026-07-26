#!/usr/bin/env bash
# Example REST requests against a running EyeSee device.
# Usage: EYESEE_HOST=192.168.1.50 ./rest-requests.sh
set -euo pipefail

HOST="${EYESEE_HOST:?Set EYESEE_HOST to the device IP or hostname, e.g. EYESEE_HOST=192.168.1.50 ./rest-requests.sh}"
BASE="http://${HOST}/api/v1"

# Full diagnostics snapshot: state, wifi status, firmware version, current pose.
curl -s "${BASE}/status" && echo

# Look toward the lower-left, without blinking on arrival.
curl -s -X POST "${BASE}/look" -H "Content-Type: application/json" \
  -d '{"x": -0.5, "y": -0.5}' && echo

# One blink, default 150ms duration.
curl -s -X POST "${BASE}/blink" -H "Content-Type: application/json" -d '{}' && echo

# Wink the left eye.
curl -s -X POST "${BASE}/wink" -H "Content-Type: application/json" \
  -d '{"side": "left"}' && echo

# Play the Happy expression.
curl -s -X POST "${BASE}/expression" -H "Content-Type: application/json" \
  -d '{"expression": "Happy"}' && echo

# Simulate a tracked face at center, holding (won't auto-recenter).
curl -s -X POST "${BASE}/track" -H "Content-Type: application/json" \
  -d '{"x": 0.0, "y": 0.0, "hold": true}' && echo

# Sleep (close), then wake (open).
curl -s -X POST "${BASE}/sleep" && echo
curl -s -X POST "${BASE}/wake" && echo

# Read the current per-channel calibration.
curl -s "${BASE}/config" && echo

# Calibrate the left/right look channel (see calibration.json.example).
curl -s -X POST "${BASE}/config" -H "Content-Type: application/json" \
  -d @calibration.json.example && echo
