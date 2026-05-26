#!/bin/bash
set -e

APP_NAME="vxap-downloader"
VERSION="1.0.0"
ARCH=$(dpkg --print-architecture)
DEB_DIR="${APP_NAME}_${VERSION}_${ARCH}"

echo ">>> Checking and installing required build dependencies..."
BUILD_DEPS=(
    "build-essential"
    "meson"
    "ninja-build"
    "pkg-config"
    "libgtk-4-dev"
    "libadwaita-1-dev"
    "libcurl4-openssl-dev"
    "libjson-glib-dev"
    "libnotify-dev"
)

MISSING_DEPS=()
for dep in "${BUILD_DEPS[@]}"; do
    if ! dpkg -s "$dep" >/dev/null 2>&1; then
        MISSING_DEPS+=("$dep")
    fi
done

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo ">>> Missing dependencies found: ${MISSING_DEPS[*]}"
    echo ">>> Installing missing dependencies (requires sudo privileges)..."
    sudo apt-get update
    sudo apt-get install -y "${MISSING_DEPS[@]}"
else
    echo ">>> All build dependencies are already installed."
fi

echo ">>> Cleaning up previous builds..."
rm -rf build
rm -rf "$DEB_DIR"
rm -f "${DEB_DIR}.deb"

echo ">>> Building the application using Meson..."
meson setup build --prefix=/usr
meson compile -C build

echo ">>> Installing to staging directory..."
DESTDIR="$PWD/$DEB_DIR" meson install -C build

echo ">>> Creating DEBIAN directory and control file..."
mkdir -p "$DEB_DIR/DEBIAN"

cat <<EOF > "$DEB_DIR/DEBIAN/control"
Package: $APP_NAME
Version: $VERSION
Architecture: $ARCH
Maintainer: Vaxp Developer <developer@vaxp.com>
Description: Vaxp Download Manager
 A fast and lightweight download manager built with GTK4 and C.
Depends: libgtk-4-1, libadwaita-1-0, libcurl4, libjson-glib-1.0-0, libnotify4, aria2
EOF

# Ensure appropriate permissions
chmod 755 "$DEB_DIR/DEBIAN"
chmod 644 "$DEB_DIR/DEBIAN/control"

echo ">>> Building Debian package..."
dpkg-deb --build "$DEB_DIR"

echo ">>> Package created successfully: ${DEB_DIR}.deb"
