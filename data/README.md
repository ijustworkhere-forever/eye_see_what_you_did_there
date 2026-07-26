# data/

LittleFS-served Web UI control panel assets (`index.html`, `app.js`,
`style.css` — added in Task 9 of `docs/superpowers/plans/2026-07-25-v0.4-connectivity.md`).

Flashing the filesystem image is a separate step from flashing firmware:

```bash
pio run -t uploadfs -e esp32dev
```

Run this once after any change to a file in this directory (firmware
flashes via the normal `pio run -t upload -e esp32dev` don't touch the
filesystem partition).
