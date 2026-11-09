
from flask import Flask, send_from_directory, jsonify
from pathlib import Path

app = Flask(__name__)

# Website folder (parent of backend)
WEBSITE_DIR = Path(__file__).resolve().parent.parent


@app.route("/")
def home():
    return send_from_directory(WEBSITE_DIR, "index.html")


@app.route("/style.css")
def stylesheet():
    return send_from_directory(WEBSITE_DIR, "style.css")


@app.route("/script.js")
def javascript():
    return send_from_directory(WEBSITE_DIR, "script.js")


@app.route("/api/health")
def health():
    return jsonify({
        "status": "ok",
        "message": "Backend is running!"
    })


if __name__ == "__main__":
    app.run(debug=True, port=5000)
