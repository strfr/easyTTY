#!/bin/bash
#
# EasyTTY Debian Package Builder
# Creates a .deb package for easy distribution
#

set -e

VERSION="1.0.0"
PACKAGE_NAME="easytty"
ARCH="amd64"
MAINTAINER="Your Name <your.email@example.com>"
DESCRIPTION="USB Device Naming Utility - KConfig-style TUI for managing persistent USB serial device names"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
DEB_DIR="$PROJECT_DIR/dist/${PACKAGE_NAME}_${VERSION}_${ARCH}"

echo -e "${YELLOW}Building EasyTTY Debian Package v${VERSION}${NC}"

# Check dependencies
check_deps() {
    echo "Checking dependencies..."
    for cmd in cmake make dpkg-deb; do
        if ! command -v $cmd &> /dev/null; then
            echo -e "${RED}Error: $cmd is required but not installed${NC}"
            exit 1
        fi
    done
    echo -e "${GREEN}✓ Dependencies OK${NC}"
}

# Build the project
build_project() {
    echo "Building project..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    echo -e "${GREEN}✓ Build complete${NC}"
}

# Create package structure
create_package() {
    echo "Creating package structure..."
    
    # Clean previous build
    rm -rf "$DEB_DIR"
    
    # Create directory structure
    mkdir -p "$DEB_DIR/DEBIAN"
    mkdir -p "$DEB_DIR/usr/bin"
    mkdir -p "$DEB_DIR/usr/share/doc/$PACKAGE_NAME"
    mkdir -p "$DEB_DIR/usr/share/man/man1"
    
    # Copy binary
    cp "$BUILD_DIR/easyTTY" "$DEB_DIR/usr/bin/easytty"
    chmod 755 "$DEB_DIR/usr/bin/easytty"
    
    # Create symlink
    ln -sf easytty "$DEB_DIR/usr/bin/easyTTY"
    
    # Copy documentation
    cp "$PROJECT_DIR/README.md" "$DEB_DIR/usr/share/doc/$PACKAGE_NAME/"
    
    # Create control file
    cat > "$DEB_DIR/DEBIAN/control" << EOF
Package: $PACKAGE_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: $ARCH
Depends: libc6 (>= 2.17), libncurses6 | libncurses5, libudev1
Maintainer: $MAINTAINER
Description: $DESCRIPTION
 EasyTTY is a terminal-based utility for creating persistent USB
 device names using udev rules. It provides a menuconfig-style
 interface for easy navigation and configuration.
 .
 Features:
  - Auto-detect USB serial devices (ttyUSB, ttyACM, etc.)
  - Extract USB device attributes (vendor ID, product ID, serial)
  - Create/delete udev rules for persistent device naming
  - Automatic rule application
EOF

    # Create postinst script
    cat > "$DEB_DIR/DEBIAN/postinst" << 'EOF'
#!/bin/bash
set -e
echo "EasyTTY installed successfully!"
echo "Run 'easytty' or 'sudo easytty' to start."
exit 0
EOF
    chmod 755 "$DEB_DIR/DEBIAN/postinst"
    
    # Create postrm script
    cat > "$DEB_DIR/DEBIAN/postrm" << 'EOF'
#!/bin/bash
set -e
if [ "$1" = "purge" ]; then
    echo "Note: udev rules in /etc/udev/rules.d/99-easytty-*.rules were preserved."
    echo "Remove them manually if desired."
fi
exit 0
EOF
    chmod 755 "$DEB_DIR/DEBIAN/postrm"
    
    echo -e "${GREEN}✓ Package structure created${NC}"
}

# Build the .deb package
build_deb() {
    echo "Building .deb package..."
    cd "$PROJECT_DIR/dist"
    dpkg-deb --build --root-owner-group "${PACKAGE_NAME}_${VERSION}_${ARCH}"
    
    echo -e "${GREEN}✓ Package created: dist/${PACKAGE_NAME}_${VERSION}_${ARCH}.deb${NC}"
}

# Main
main() {
    check_deps
    build_project
    create_package
    build_deb
    
    echo ""
    echo -e "${GREEN}═══════════════════════════════════════════${NC}"
    echo -e "${GREEN}Package built successfully!${NC}"
    echo -e "${GREEN}═══════════════════════════════════════════${NC}"
    echo ""
    echo "Install with: sudo dpkg -i dist/${PACKAGE_NAME}_${VERSION}_${ARCH}.deb"
    echo "Remove with:  sudo apt remove $PACKAGE_NAME"
}

main "$@"
