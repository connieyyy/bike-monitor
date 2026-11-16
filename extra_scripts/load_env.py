from pathlib import Path
import shlex

Import("env")


ENV_KEYS = (
    "WIFI_SSID",
    "WIFI_PASS",
    "MQTT_HOST",
    "MQTT_USER",
    "MQTT_PASS",
)


def load_dotenv(path):
    values = {}
    if not path.is_file():
        raise RuntimeError(
            f"Missing {path}. Create it from .env.example before building."
        )

    for line_number, line in enumerate(path.read_text().splitlines(), start=1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("export "):
            line = line[7:].lstrip()
        if "=" not in line:
            raise RuntimeError(f"Invalid .env entry on line {line_number}.")

        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip()
        if value[:1] in ("'", '"'):
            value = shlex.split(value, comments=False)[0]
        else:
            value = value.split(" #", 1)[0].rstrip()
        values[key] = value

    missing = [key for key in ENV_KEYS if not values.get(key)]
    if missing:
        raise RuntimeError(
            "Missing required .env values: " + ", ".join(missing)
        )
    return values


dotenv = load_dotenv(Path(env["PROJECT_DIR"]) / ".env")
env.Append(
    CPPDEFINES=[
        (f"BIKE_{key}", env.StringifyMacro(dotenv[key]))
        for key in ENV_KEYS
    ]
)
