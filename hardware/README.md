# hardware/

3D-printable design files for this project's physical build.

- `EyeMech_sled_FINAL_v7.stl` — **print this one.** The current version of
  a custom support sled that holds an ESP32 mini and the PCA9685 driver
  board, designed to mount alongside the eye mechanism itself.
- `EyeMech_sled_compact.stl` — an earlier iteration of the same sled,
  kept for reference. Superseded by `_FINAL_v7`.

Neither is part of the original MakerWorld/Instructables eye mechanism
design (see the root `README.md`'s "Physical eye mechanism" note) — both
are additions specific to this build, sized for an ESP32 mini + PCA9685
rather than whatever controller board the original design assumed.

See `docs/hardware.md` for print/slicer settings notes from this project's
own build.
