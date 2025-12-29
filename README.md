# EasyTTY - USB Device Naming Utility

A KConfig-style TUI application for creating persistent USB serial device names using udev rules on Linux.

## Features

- 🔍 **Auto-detect USB Serial Devices** - Automatically discovers ttyUSB, ttyACM, and other USB serial devices
- 📝 **Extract Device Attributes** - Retrieves vendor ID, product ID, serial number, and other USB attributes
- ✨ **Create Persistent Names** - Generate udev rules to create symlinks like `/dev/RS485_1` instead of `/dev/ttyUSB0`
- 🗂️ **Manage Existing Rules** - View, verify, and delete existing EasyTTY-created rules
- 🔄 **Auto-apply Rules** - Automatically reloads udev rules after changes
- 🖥️ **KConfig-style TUI** - Familiar menuconfig-like interface with keyboard navigation

## Screenshots

```
┌─────────────────────────────────────────────────────────────┐
│              EasyTTY - USB Device Manager                   │
├─────────────────────────────────────────────────────────────┤
│  > List Connected Devices (2 found)                         │
│  > Manage Existing Rules (1 rules)                          │
│  ──────────────────────────────────────────────────────     │
│    Refresh Devices                                          │
│    Reload & Apply udev Rules                                │
│  ──────────────────────────────────────────────────────     │
│    Help                                                     │
│    About                                                    │
│  ──────────────────────────────────────────────────────     │
│    Exit                                                     │
├─────────────────────────────────────────────────────────────┤
│ ↑/↓: Navigate  Enter: Select  Q: Quit  ESC: Back           │
└─────────────────────────────────────────────────────────────┘
```

## Requirements

- Linux with udev
- CMake 3.16+
- GCC/Clang with C++17 support
- ncurses development library
- libudev development library

### Installing Dependencies

**Debian/Ubuntu:**
```bash
sudo apt install build-essential cmake libncurses-dev libudev-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc-c++ cmake ncurses-devel systemd-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake ncurses systemd
```

## Building

```bash
# Clone or navigate to the project directory
cd easyTTY

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
make -j$(nproc)

# Optional: Install system-wide
sudo make install
```

## Usage

### Interactive TUI Mode

Simply run the application:

```bash
./easyTTY
```

Or with sudo for full functionality:

```bash
sudo ./easyTTY
```

### Command Line Options

```bash
# Show help
./easyTTY --help

# List connected devices (non-interactive)
./easyTTY --list

# List existing rules (non-interactive)
./easyTTY --rules
```

### Navigation

| Key | Action |
|-----|--------|
| ↑/↓ or j/k | Navigate menu items |
| Enter | Select/Execute |
| ESC | Go back / Cancel |
| Q | Quit application |

## How It Works

1. **Device Detection**: Uses libudev to enumerate USB serial devices and extract their attributes
2. **Rule Generation**: Creates udev rules in `/etc/udev/rules.d/` with the pattern `99-easytty-<name>.rules`
3. **Rule Application**: Reloads udev rules and triggers device re-enumeration

### Example Generated Rule

```
# EasyTTY auto-generated rule
# Device: USB-Serial Controller (ttyUSB0)
# Vendor: FTDI (0403)
# Product: FT232R USB UART (6001)
# Serial: A50285BI

SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A50285BI", SYMLINK+="RS485_1", MODE="0666"
```

## Project Structure

```
easyTTY/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── include/
│   ├── app/
│   │   └── Application.hpp     # Main application class
│   ├── common/
│   │   ├── Types.hpp           # Common types and structures
│   │   └── Utils.hpp           # Utility functions
│   ├── device/
│   │   └── DeviceDetector.hpp  # USB device detection
│   ├── tui/
│   │   ├── Menu.hpp            # Menu component
│   │   └── Screen.hpp          # ncurses screen wrapper
│   └── udev/
│       └── UdevManager.hpp     # udev rule management
├── src/
│   ├── app/
│   │   └── Application.cpp
│   ├── common/
│   │   └── Utils.cpp
│   ├── device/
│   │   └── DeviceDetector.cpp
│   ├── tui/
│   │   ├── Menu.cpp
│   │   └── Screen.cpp
│   ├── udev/
│   │   └── UdevManager.cpp
│   └── main.cpp                # Entry point
└── scripts/
    └── easytty-reload          # Helper script for rule reloading
```

## Troubleshooting

### "Permission denied" errors
Run with sudo: `sudo ./easyTTY`

### Device not appearing
1. Ensure the device is connected
2. Check `dmesg` for connection messages
3. Use "Refresh Devices" in the menu

### Symlink not created after rule creation
1. Try "Reload & Apply udev Rules" from the main menu
2. Unplug and replug the device
3. Check rule file exists in `/etc/udev/rules.d/`

### Multiple devices with same Vendor/Product ID
If you have multiple identical devices, the serial number is used to distinguish them. If devices don't have unique serial numbers, they will all get the same symlink (udev will append numbers automatically).

## License

MIT License - See LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.
