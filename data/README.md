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

Anyone with a device already flashed under the old partition table must
re-flash both firmware and filesystem after a partition table change
(`pio run -t upload -e esp32dev` then `pio run -t uploadfs -e esp32dev`),
since a partition table change moves where every partition's data lives
on flash, making old filesystem/OTA contents at the previous offsets
invalid at the new ones; this is a one-time transition, and ordinary OTA
updates after this point work normally again.
