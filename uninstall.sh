#!/bin/bash
#
# EasyTTY Uninstall Script
#

set -e

INSTALL_DIR="/usr/local/bin"

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: Please run as root (sudo)"
    exit 1
fi

echo "Uninstalling EasyTTY..."

# Remove binary and symlinks
rm -f "$INSTALL_DIR/easytty"
rm -f "$INSTALL_DIR/easyTTY"

echo "EasyTTY has been uninstalled."
echo ""
echo "Note: udev rules created by EasyTTY are preserved in /etc/udev/rules.d/"
echo "To remove them: sudo rm /etc/udev/rules.d/99-easytty-*.rules"
