#!/bin/bash
# Install Vdownloader Native Messaging Host for Chrome and Firefox

NATIVE_HOST_PATH="/usr/lib/vxap-downloader/vdl-native-host.py"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "=== Vdownloader Native Messaging Host Installer ==="

# Install native host script
sudo mkdir -p /usr/lib/vxap-downloader
sudo cp "$SCRIPT_DIR/vdl-native-host.py" "$NATIVE_HOST_PATH"
sudo chmod +x "$NATIVE_HOST_PATH"

# Chrome/Chromium
CHROME_DIRS=(
  "/etc/chromium/native-messaging-hosts"
  "/etc/opt/chrome/native-messaging-hosts"
  "$HOME/.config/google-chrome/NativeMessagingHosts"
  "$HOME/.config/chromium/NativeMessagingHosts"
)

for dir in "${CHROME_DIRS[@]}"; do
  mkdir -p "$dir" 2>/dev/null
  if [ -d "$dir" ]; then
    cp "$SCRIPT_DIR/org.vaxp.downloader.chrome.json" "$dir/org.vaxp.downloader.json"
    # Update path in manifest
    sed -i "s|/usr/lib/vxap-downloader/vdl-native-host.py|$NATIVE_HOST_PATH|g" "$dir/org.vaxp.downloader.json"
    echo "✓ Installed for Chrome/Chromium: $dir"
  fi
done

# Firefox
FIREFOX_DIRS=(
  "/usr/lib/mozilla/native-messaging-hosts"
  "$HOME/.mozilla/native-messaging-hosts"
)

for dir in "${FIREFOX_DIRS[@]}"; do
  mkdir -p "$dir" 2>/dev/null
  if [ -d "$dir" ]; then
    cp "$SCRIPT_DIR/org.vaxp.downloader.firefox.json" "$dir/org.vaxp.downloader.json"
    sed -i "s|/usr/lib/vxap-downloader/vdl-native-host.py|$NATIVE_HOST_PATH|g" "$dir/org.vaxp.downloader.json"
    echo "✓ Installed for Firefox: $dir"
  fi
done

echo ""
echo "=== Installation complete! ==="
echo "Restart your browser for changes to take effect."
