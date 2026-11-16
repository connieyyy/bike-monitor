# bike-monitor

## Configuration

PlatformIO loads the cloud credentials from a local `.env` file at build time.
Copy `.env.example` to `.env`, replace the placeholder values, and build or
upload the firmware:

```sh
cp .env.example .env
pio run -t upload
```

The `.env` file is ignored by Git. Values are compiled into the firmware and
used as defaults only when no credentials are already stored in the ESP32's
NVS. Use the `PROVISION` serial command to replace them, or `FORGET` followed by
a restart to make the current build defaults apply again.