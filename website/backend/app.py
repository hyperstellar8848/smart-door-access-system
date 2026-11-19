
import os
import secrets
from datetime import datetime, timezone
from pathlib import Path

import serial
from flask import Flask, jsonify, request, send_from_directory, session
from webauthn import base64url_to_bytes

from auth import (
    RP_ID,
    ORIGIN,
    create_registration_options,
    verify_registration,
    create_authentication_options,
    verify_authentication,
)

from storage import load_data, save_data, add_access_log


app = Flask(__name__)

app.secret_key = os.environ.get("SECRET_KEY") or secrets.token_hex(32)

app.config.update(
    SESSION_COOKIE_HTTPONLY=True,
    SESSION_COOKIE_SECURE=False,
    SESSION_COOKIE_SAMESITE="Lax",
)

WEBSITE_DIR = Path(__file__).resolve().parent.parent

# Wokwi serial connection.
SERIAL_PORT = "rfc2217://localhost:4000"
SERIAL_BAUDRATE = 115200


def send_hardware_command(command):
    try:
        with serial.serial_for_url(
            SERIAL_PORT,
            baudrate=SERIAL_BAUDRATE,
            timeout=3,
        ) as connection:
            connection.write(f"{command}\n".encode("utf-8"))
            connection.flush()
            response = connection.readline().decode(
                "utf-8", errors="replace"
            ).strip()

        if not response:
            raise RuntimeError("ESP32 did not respond to the command.")

        return response

    except (serial.SerialException, OSError) as error:
        raise RuntimeError(
            f"Could not communicate with the ESP32: {error}"
        ) from error


@app.before_request
def check_request_origin():
    if request.method == "POST" and request.path.startswith("/api/"):
        origin = request.headers.get("Origin")

        if origin and origin != ORIGIN:
            return jsonify({"message": "Invalid request origin."}), 403


def get_json():
    data = request.get_json(silent=True)
    return data if isinstance(data, dict) else {}


def has_permission(username, door_id):
    permissions = load_data("permissions")
    return door_id in permissions.get(username, [])


def get_user_and_door(data):
    username = str(data.get("username", "")).strip()
    door_id = str(data.get("door_id", "")).strip()

    if not username or not door_id:
        return None, None

    users = load_data("users")

    if username not in users:
        return None, None

    if not has_permission(username, door_id):
        return None, None

    return username, door_id


def log_event(username, door_id, event, result):
    add_access_log({
        "username": username,
        "door_id": door_id,
        "event": event,
        "result": result,
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "mode": "wokwi",
    })


# Website routes

@app.route("/")
def home():
    return send_from_directory(WEBSITE_DIR, "index.html")


@app.route("/style.css")
def stylesheet():
    return send_from_directory(WEBSITE_DIR, "style.css")


@app.route("/script.js")
def javascript():
    return send_from_directory(WEBSITE_DIR, "script.js")


@app.route("/manifest.json")
def web_app_manifest():
    return send_from_directory(
        WEBSITE_DIR,
        "manifest.json",
        mimetype="application/manifest+json",
    )


@app.route("/service-worker.js")
def service_worker():
    response = send_from_directory(
        WEBSITE_DIR, "service-worker.js"
    )
    response.headers["Service-Worker-Allowed"] = "/"
    response.headers["Cache-Control"] = "no-cache"
    return response


@app.route("/icon.png")
def app_icon():
    return send_from_directory(
        WEBSITE_DIR, "icon.png", mimetype="image/png"
    )


# Backend health check

@app.route("/api/health")
def health():
    return jsonify({
        "status": "ok",
        "message": "Backend is running!"
    })


# Passkey registration

@app.route(
    "/api/v1/webauthn/register/options",
    methods=["POST"]
)
def registration_options():
    data = get_json()
    username, door_id = get_user_and_door(data)

    if not username:
        return jsonify({
            "message": "Account not found or no permission for this door."
        }), 403

    users = load_data("users")
    passkeys = load_data("passkeys")

    user_id = base64url_to_bytes(users[username]["user_id"])

    existing_credentials = [
        item["credential_id"]
        for item in passkeys.get(username, [])
    ]

    result = create_registration_options(
        username,
        user_id,
        existing_credentials,
    )

    session["registration"] = {
        "challenge": result["challenge"],
        "username": username,
        "door_id": door_id,
    }

    return jsonify(result["options"])


@app.route(
    "/api/v1/webauthn/register/verify",
    methods=["POST"]
)
def registration_verify():
    data = get_json()
    username = str(data.get("username", "")).strip()
    door_id = str(data.get("door_id", "")).strip()
    credential = data.get("credential")

    pending = session.get("registration")

    if not pending or not isinstance(credential, dict):
        return jsonify({
            "message": "Registration session expired."
        }), 400

    if pending["username"] != username or pending["door_id"] != door_id:
        return jsonify({
            "message": "Registration session mismatch."
        }), 400

    if not has_permission(username, door_id):
        return jsonify({
            "message": "Door permission denied."
        }), 403

    try:
        result = verify_registration(
            credential,
            pending["challenge"],
        )
    except Exception:
        session.pop("registration", None)
        return jsonify({
            "message": "Passkey registration verification failed."
        }), 400

    passkeys = load_data("passkeys")
    user_passkeys = passkeys.get(username, [])

    if any(
        item["credential_id"] == result["credential_id"]
        for item in user_passkeys
    ):
        return jsonify({
            "message": "This passkey is already registered."
        }), 409

    user_passkeys.append({
        **result,
        "transports": credential.get(
            "response", {}
        ).get("transports", []),
    })

    passkeys[username] = user_passkeys
    save_data("passkeys", passkeys)

    session.pop("registration", None)

    return jsonify({
        "message": "Passkey registered successfully."
    })


# Passkey authentication options

@app.route(
    "/api/v1/webauthn/authenticate/options",
    methods=["POST"]
)
def authentication_options():
    data = get_json()
    username, door_id = get_user_and_door(data)

    if not username:
        try:
            send_hardware_command("DENIED")
        except RuntimeError:
            pass

        return jsonify({
            "message": "Account not found or no permission for this door."
        }), 403

    passkeys = load_data("passkeys")
    user_passkeys = passkeys.get(username, [])

    if not user_passkeys:
        return jsonify({
            "message": "No passkey registered for this account."
        }), 404

    credential_ids = [
        item["credential_id"]
        for item in user_passkeys
    ]

    result = create_authentication_options(credential_ids)

    session["authentication"] = {
        "challenge": result["challenge"],
        "username": username,
        "door_id": door_id,
    }

    return jsonify(result["options"])


# Verify authentication and unlock

@app.route(
    "/api/v1/webauthn/authenticate/verify",
    methods=["POST"]
)
def authentication_verify():
    data = get_json()
    username = str(data.get("username", "")).strip()
    door_id = str(data.get("door_id", "")).strip()
    credential = data.get("credential")

    pending = session.get("authentication")

    if not pending or not isinstance(credential, dict):
        return jsonify({
            "message": "Authentication session expired."
        }), 400

    if pending["username"] != username or pending["door_id"] != door_id:
        return jsonify({
            "message": "Authentication session mismatch."
        }), 400

    if not has_permission(username, door_id):
        try:
            send_hardware_command("DENIED")
        except RuntimeError:
            pass

        return jsonify({"message": "Access denied."}), 403

    credential_id = credential.get("id") or credential.get("rawId")

    passkeys = load_data("passkeys")
    user_passkeys = passkeys.get(username, [])

    stored_credential = next(
        (
            item for item in user_passkeys
            if item["credential_id"] == credential_id
        ),
        None,
    )

    if not stored_credential:
        try:
            send_hardware_command("DENIED")
        except RuntimeError:
            pass

        return jsonify({"message": "Unknown passkey."}), 403

    try:
        result = verify_authentication(
            credential,
            pending["challenge"],
            stored_credential,
        )
    except Exception:
        session.pop("authentication", None)

        try:
            send_hardware_command("DENIED")
        except RuntimeError:
            pass

        return jsonify({
            "message": "Authentication verification failed."
        }), 401

    stored_credential["sign_count"] = result["new_sign_count"]
    save_data("passkeys", passkeys)

    session.pop("authentication", None)

    try:
        hardware_response = send_hardware_command("GRANTED")
    except RuntimeError as error:
        return jsonify({
            "message": (
                "Identity verified, but the door controller "
                f"is unavailable: {error}"
            )
        }), 503

    session.clear()
    session["authenticated_user"] = username
    session["authenticated_door"] = door_id

    log_event(username, door_id, "UNLOCK", hardware_response)

    return jsonify({
        "status": "granted",
        "message": "Identity verified. Door unlocked successfully."
    })


# Lock door

@app.route("/api/v1/access/lock", methods=["POST"])
def lock_door():
    data = get_json()
    door_id = str(data.get("door_id", "")).strip()

    username = session.get("authenticated_user")
    authenticated_door = session.get("authenticated_door")

    if not username or authenticated_door != door_id:
        return jsonify({
            "message": "Authentication required."
        }), 401

    if not has_permission(username, door_id):
        session.clear()
        return jsonify({
            "message": "Door permission denied."
        }), 403

    try:
        hardware_response = send_hardware_command("LOCK")
    except RuntimeError as error:
        return jsonify({
            "message": f"The door controller is unavailable: {error}"
        }), 503

    log_event(username, door_id, "LOCK", hardware_response)

    session.clear()

    return jsonify({
        "message": "Door locked successfully."
    })


# Start Flask and allow connections from other devices on the LAN

if __name__ == "__main__":
    app.run(
        debug=False,
        host="0.0.0.0",
        port=5000,
    )
