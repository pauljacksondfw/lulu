# LULU Dashboard usermod

This usermod stores the four LULU dashboard button names and WLED preset assignments on the controller.

## Enable it

Add `lulu_dashboard` to `custom_usermods` for the environment you build.

For an `esp32dev` build, a simple `platformio_override.ini` can contain:

```ini
[env:esp32dev]
custom_usermods = ${common.default_usermods} lulu_dashboard
```

If you already have a `custom_usermods` line, keep the existing entries and add `lulu_dashboard`.

## How it works

- The dashboard reads the button configuration from `GET /json/state`.
- The dashboard reads all saved WLED presets from `/presets.json`.
- Saving the form posts the four button assignments back to `/json/state`.
- This usermod receives the LULU object and schedules one `serializeConfig()` call from `loop()`.
- WLED then stores the values under `um.LULU` in `cfg.json`.

The configuration therefore belongs to the controller, not to a phone or browser.
