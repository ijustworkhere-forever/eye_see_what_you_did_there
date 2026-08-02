const joystickPad = document.getElementById('joystick-pad');
const joystickKnob = document.getElementById('joystick-knob');

let dragging = false;
let lastSentAt = 0;
const kSendThrottleMs = 50; // 20Hz max, well under CommandQueue's 16-slot capacity

function postJson(path, body) {
    return fetch(path, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body || {}),
    })
        .then((response) => {
            if (!response.ok) {
                console.error(`${path} failed: HTTP ${response.status}`);
            }
            return response;
        })
        .catch((error) => {
            console.error(`${path} failed: ${error}`);
        });
}

function sendLook(normalizedX, normalizedY) {
    const now = Date.now();
    if (now - lastSentAt < kSendThrottleMs) {
        return;
    }
    lastSentAt = now;
    postJson('/api/v1/look', { x: normalizedX, y: normalizedY });
}

function updateKnobPosition(normalizedX, normalizedY) {
    const rect = joystickPad.getBoundingClientRect();
    const centerX = rect.width / 2;
    const centerY = rect.height / 2;
    joystickKnob.style.left = `${centerX + normalizedX * centerX}px`;
    joystickKnob.style.top = `${centerY + normalizedY * centerY}px`;
}

function handlePointerMove(event) {
    if (!dragging) {
        return;
    }
    const rect = joystickPad.getBoundingClientRect();
    const relativeX = (event.clientX - rect.left - rect.width / 2) / (rect.width / 2);
    const relativeY = (event.clientY - rect.top - rect.height / 2) / (rect.height / 2);
    const clampedX = Math.max(-1, Math.min(1, relativeX));
    const clampedY = Math.max(-1, Math.min(1, relativeY));
    updateKnobPosition(clampedX, clampedY);
    sendLook(clampedX, clampedY);
}

joystickPad.addEventListener('pointerdown', (event) => {
    dragging = true;
    joystickPad.setPointerCapture(event.pointerId);
    handlePointerMove(event);
});
joystickPad.addEventListener('pointermove', handlePointerMove);
joystickPad.addEventListener('pointerup', () => {
    dragging = false;
    updateKnobPosition(0, 0);
    sendLook(0, 0);
});

document.getElementById('blink-btn').addEventListener('click', () => postJson('/api/v1/blink'));
document.getElementById('wink-left-btn').addEventListener('click', () => postJson('/api/v1/wink', { side: 'left' }));
document.getElementById('wink-right-btn').addEventListener('click', () => postJson('/api/v1/wink', { side: 'right' }));
document.getElementById('sleep-btn').addEventListener('click', () => postJson('/api/v1/sleep'));
document.getElementById('wake-btn').addEventListener('click', () => postJson('/api/v1/wake'));

document.querySelectorAll('#expression-section button').forEach((button) => {
    button.addEventListener('click', () => {
        postJson('/api/v1/expression', { expression: button.dataset.expression });
    });
});

const calibrationButtons = document.querySelectorAll('#calibration-channels button');
const calibrationForm = document.getElementById('calibration-form');
const calibrationStatus = document.getElementById('calibration-status');
const calMin = document.getElementById('cal-min');
const calMax = document.getElementById('cal-max');
const calNeutral = document.getElementById('cal-neutral');
const calOffset = document.getElementById('cal-offset');
const calInverted = document.getElementById('cal-inverted');
const calMirrored = document.getElementById('cal-mirrored');

let selectedChannel = null;

function selectChannel(channel) {
    selectedChannel = channel;
    calibrationButtons.forEach((button) => {
        button.classList.toggle('selected', button.dataset.channel === channel);
    });
    calibrationStatus.textContent = 'Loading...';
    fetch('/api/v1/config')
        .then((response) => response.json())
        .then((config) => {
            const servo = config[channel];
            calMin.value = servo.minPulseUs;
            calMax.value = servo.maxPulseUs;
            calNeutral.value = servo.neutralPulseUs;
            calOffset.value = servo.mechanicalOffset;
            calInverted.checked = servo.inverted;
            calMirrored.checked = servo.mirrored;
            calibrationStatus.textContent = '';
        })
        .catch((error) => {
            calibrationStatus.textContent = `Failed to load: ${error}`;
        });
}

calibrationButtons.forEach((button) => {
    button.addEventListener('click', () => selectChannel(button.dataset.channel));
});

calibrationForm.addEventListener('submit', (event) => {
    event.preventDefault();
    if (!selectedChannel) {
        return;
    }
    const body = {
        channel: selectedChannel,
        minPulseUs: Number(calMin.value),
        maxPulseUs: Number(calMax.value),
        neutralPulseUs: Number(calNeutral.value),
        mechanicalOffset: Number(calOffset.value),
        inverted: calInverted.checked,
        mirrored: calMirrored.checked,
    };
    calibrationStatus.textContent = 'Saving...';
    fetch('/api/v1/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body),
    })
        .then((response) => {
            if (!response.ok) {
                return response.json().then((error) => {
                    throw new Error(error.error || `HTTP ${response.status}`);
                });
            }
            calibrationStatus.textContent = 'Saved -- change is live and persisted to flash.';
        })
        .catch((error) => {
            calibrationStatus.textContent = `Save failed: ${error.message}`;
        });
});

// Default to LR selected so the form shows real values immediately, matching
// the rest of this page's no-click-required-to-see-state convention.
selectChannel('lr');

const diagConnection = document.getElementById('diag-connection');
const diagState = document.getElementById('diag-state');
const diagLook = document.getElementById('diag-look');
const diagLids = document.getElementById('diag-lids');
const diagUptime = document.getElementById('diag-uptime');

function connectWebSocket() {
    const socket = new WebSocket(`ws://${window.location.host}/ws`);

    socket.addEventListener('open', () => {
        diagConnection.textContent = 'connected';
    });

    socket.addEventListener('close', () => {
        diagConnection.textContent = 'disconnected';
        setTimeout(connectWebSocket, 2000); // WiFi/AP drop shouldn't require a page reload
    });

    socket.addEventListener('message', (event) => {
        const state = JSON.parse(event.data);
        diagState.textContent = state.state;
        diagLook.textContent = `${state.pose.lookX.toFixed(2)}, ${state.pose.lookY.toFixed(2)}`;
        diagLids.textContent = [
            state.pose.upperLeftLid,
            state.pose.lowerLeftLid,
            state.pose.upperRightLid,
            state.pose.lowerRightLid,
        ]
            .map((value) => value.toFixed(2))
            .join(' / ');
        diagUptime.textContent = `${(state.uptimeMs / 1000).toFixed(1)}s`;
    });
}

connectWebSocket();
