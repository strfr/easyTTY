#!/bin/bash
#
# EasyTTY Installation Script
# One-line install: curl -sSL https://raw.githubusercontent.com/USER/easyTTY/main/install.sh | sudo bash
#

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

VERSION="1.0.0"
INSTALL_DIR="/usr/local/bin"
BUILD_DIR="/tmp/easytty-build-$$"
REPO_URL="https://github.com/USER/easyTTY.git"

echo -e "${BLUE}"
echo "╔═══════════════════════════════════════════╗"
echo "║     EasyTTY Installer v${VERSION}            ║"
echo "║     USB Device Naming Utility             ║"
echo "╚═══════════════════════════════════════════╝"
echo -e "${NC}"

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: Please run as root (sudo)${NC}"
    echo "Usage: curl -sSL <url>/install.sh | sudo bash"
    exit 1
fi

# Detect package manager and install dependencies
install_dependencies() {
    echo -e "${YELLOW}[1/4] Installing dependencies...${NC}"
    
    if command -v apt-get &> /dev/null; then
        apt-get update -qq
        apt-get install -y -qq git cmake build-essential libncurses-dev libudev-dev > /dev/null
    elif command -v dnf &> /dev/null; then
        dnf install -y -q git cmake gcc-c++ ncurses-devel systemd-devel
    elif command -v yum &> /dev/null; then
        yum install -y -q git cmake gcc-c++ ncurses-devel systemd-devel
    elif command -v pacman &> /dev/null; then
        pacman -Sy --noconfirm --quiet git cmake base-devel ncurses
    elif command -v zypper &> /dev/null; then
        zypper install -y git cmake gcc-c++ ncurses-devel libudev-devel
    else
        echo -e "${RED}Error: Unsupported package manager${NC}"
        echo "Please install manually: git cmake g++ libncurses-dev libudev-dev"
        exit 1
    fi
    echo -e "${GREEN}✓ Dependencies installed${NC}"
}

# Clone and build
build_easytty() {
    echo -e "${YELLOW}[2/4] Downloading source...${NC}"
    
    # Clean up any previous build
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Check if we're running from a local directory with source
    if [ -f "/tmp/easytty-local/CMakeLists.txt" ]; then
        cp -r /tmp/easytty-local/* .
    else
        git clone --depth 1 --branch v${VERSION} "$REPO_URL" . 2>/dev/null || \
        git clone --depth 1 "$REPO_URL" . 2>/dev/null
    fi
    echo -e "${GREEN}✓ Source downloaded${NC}"
    
    echo -e "${YELLOW}[3/4] Building...${NC}"
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release > /dev/null
    make -j$(nproc) > /dev/null 2>&1
    echo -e "${GREEN}✓ Build complete${NC}"
}

# Install binary
install_binary() {
    echo -e "${YELLOW}[4/4] Installing...${NC}"
    
    # Install binary
    install -m 755 "$BUILD_DIR/build/easyTTY" "$INSTALL_DIR/easytty"
    
    # Create symlink for alternative name
    ln -sf "$INSTALL_DIR/easytty" "$INSTALL_DIR/easyTTY" 2>/dev/null || true
    
    # Clean up
    rm -rf "$BUILD_DIR"
    
    echo -e "${GREEN}✓ Installed to $INSTALL_DIR/easytty${NC}"
}

# Main installation
main() {
    install_dependencies
    build_easytty
    install_binary
    
    echo ""
    echo -e "${GREEN}╔═══════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║     Installation Complete!                ║${NC}"
    echo -e "${GREEN}╚═══════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "Run ${BLUE}easytty${NC} or ${BLUE}sudo easytty${NC} to start"
    echo -e "Run ${BLUE}easytty --help${NC} for usage information"
    echo ""
}

main "$@"
