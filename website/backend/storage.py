import json
from pathlib import Path

DATA_DIR = Path(__file__).resolve().parent / "data"

FILES = {
    "users": "users.json",
    "passkeys": "passkeys.json",
    "permissions": "permissions.json",
    "access_logs": "access_logs.json",
}


def load_data(name):
    if name not in FILES:
        raise ValueError("Unknown data file")

    file_path = DATA_DIR / FILES[name]

    if not file_path.exists():
        return [] if name == "access_logs" else {}

    with open(file_path, "r", encoding="utf-8") as file:
        return json.load(file)


def save_data(name, data):
    if name not in FILES:
        raise ValueError("Unknown data file")

    DATA_DIR.mkdir(parents=True, exist_ok=True)
    file_path = DATA_DIR / FILES[name]

    with open(file_path, "w", encoding="utf-8") as file:
        json.dump(data, file, indent=4, ensure_ascii=False)


def add_access_log(entry):
    logs = load_data("access_logs")
    logs.append(entry)
    save_data("access_logs", logs)
