#!/usr/bin/env python3
"""
Vdownloader Native Messaging Host
Bridges the browser extension with the Vdownloader application.
Reads JSON messages from stdin, sends download URLs to the app via D-Bus or
command line, and responds with status.
"""

import json
import struct
import sys
import subprocess
import os


def read_message():
    """Read a native messaging message from stdin."""
    raw_length = sys.stdin.buffer.read(4)
    if not raw_length:
        return None
    length = struct.unpack("=I", raw_length)[0]
    data = sys.stdin.buffer.read(length)
    return json.loads(data.decode("utf-8"))


def send_message(msg):
    """Send a native messaging message to stdout."""
    encoded = json.dumps(msg).encode("utf-8")
    sys.stdout.buffer.write(struct.pack("=I", len(encoded)))
    sys.stdout.buffer.write(encoded)
    sys.stdout.buffer.flush()


def handle_download(msg):
    """Handle a download request from the extension."""
    url = msg.get("url", "")
    filename = msg.get("filename", "")
    referrer = msg.get("referrer", "")

    if not url:
        send_message({"status": "error", "message": "No URL provided"})
        return

    # Find the Vdownloader executable
    app_paths = [
        "/usr/bin/vxap-downloader",
        "/usr/local/bin/vxap-downloader",
        os.path.expanduser("~/Desktop/VXAPDownloader/builddir/vxap-downloader"),
    ]

    app_path = None
    for p in app_paths:
        if os.path.isfile(p) and os.access(p, os.X_OK):
            app_path = p
            break

    if app_path:
        try:
            # Launch the app with --add-url argument
            subprocess.Popen(
                [app_path, "--add-url", url],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            send_message({
                "status": "started",
                "filename": filename,
                "url": url,
            })
        except Exception as e:
            send_message({"status": "error", "message": str(e)})
    else:
        # Fallback: write to a queue file that the app can watch
        queue_dir = os.path.expanduser("~/.local/share/vxap-downloader")
        os.makedirs(queue_dir, exist_ok=True)
        queue_file = os.path.join(queue_dir, "pending_downloads.json")

        pending = []
        if os.path.exists(queue_file):
            try:
                with open(queue_file, "r") as f:
                    pending = json.load(f)
            except (json.JSONDecodeError, IOError):
                pending = []

        pending.append({
            "url": url,
            "filename": filename,
            "referrer": referrer,
        })

        with open(queue_file, "w") as f:
            json.dump(pending, f, indent=2)

        send_message({
            "status": "queued",
            "filename": filename,
            "message": "Queued for download (app not running)",
        })


def main():
    """Main loop: read messages and handle them."""
    while True:
        msg = read_message()
        if msg is None:
            break

        action = msg.get("action", "")
        if action == "download":
            handle_download(msg)
        elif action == "ping":
            send_message({"status": "pong", "version": "1.0.0"})
        else:
            send_message({"status": "error", "message": f"Unknown action: {action}"})


if __name__ == "__main__":
    main()
