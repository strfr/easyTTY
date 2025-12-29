#!/bin/bash
#
# EasyTTY Release Builder
# Builds all release artifacts for GitHub release
#

set -e

VERSION="1.0.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
RELEASE_DIR="$PROJECT_DIR/release"

echo "Building EasyTTY Release v${VERSION}"
echo "=================================="

# Clean release directory
rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR"

# Build the project
echo "Building project..."
cd "$PROJECT_DIR"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Copy standalone binary
echo "Creating standalone binary..."
cp easyTTY "$RELEASE_DIR/easytty-${VERSION}-linux-amd64"
chmod +x "$RELEASE_DIR/easytty-${VERSION}-linux-amd64"

# Build .deb package
echo "Building .deb package..."
cd "$PROJECT_DIR"
bash packaging/build-deb.sh
cp dist/*.deb "$RELEASE_DIR/"

# Create source tarball
echo "Creating source tarball..."
cd "$PROJECT_DIR/.."
tar --exclude='easyTTY/build' \
    --exclude='easyTTY/dist' \
    --exclude='easyTTY/release' \
    --exclude='easyTTY/.git' \
    -czvf "$RELEASE_DIR/easytty-${VERSION}-source.tar.gz" easyTTY

# Create checksums
echo "Creating checksums..."
cd "$RELEASE_DIR"
sha256sum * > SHA256SUMS.txt

echo ""
echo "Release artifacts created in: $RELEASE_DIR"
ls -la "$RELEASE_DIR"
