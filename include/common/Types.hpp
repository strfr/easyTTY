#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <functional>

namespace easytty {

/**
 * @brief Device information structure containing USB device attributes
 */
struct DeviceInfo {
    std::string devPath;        // e.g., /dev/ttyUSB0
    std::string sysPath;        // e.g., /sys/devices/...
    std::string subsystem;      // e.g., tty, usb
    std::string vendor;         // Vendor name
    std::string vendorId;       // e.g., 0403
    std::string productId;      // e.g., 6001
    std::string serial;         // Serial number
    std::string manufacturer;   // Manufacturer string
    std::string product;        // Product string
    std::string driver;         // Kernel driver
    std::string devNode;        // Device node name (ttyUSB0)
    
    bool isValid() const {
        return !devPath.empty() && !vendorId.empty();
    }
    
    std::string getDisplayName() const {
        if (!product.empty()) {
            return product + " (" + devNode + ")";
        }
        return devNode;
    }
};

/**
 * @brief udev rule structure
 */
struct UdevRule {
    std::string name;           // Custom symlink name
    std::string vendorId;
    std::string productId;
    std::string serial;
    std::string symlink;        // Resulting symlink in /dev/
    std::string filePath;       // Path to the rule file
    int priority;               // Rule priority (e.g., 99)
    bool isActive;              // Whether rule is currently active
    
    std::string generateRule() const;
    std::string getFileName() const;
};

/**
 * @brief Result type for operations
 */
struct OperationResult {
    bool success;
    std::string message;
    
    static OperationResult Success(const std::string& msg = "Operation completed successfully") {
        return {true, msg};
    }
    
    static OperationResult Failure(const std::string& msg) {
        return {false, msg};
    }
};

/**
 * @brief Menu item types for TUI
 */
enum class MenuItemType {
    Action,
    Submenu,
    Toggle,
    Input,
    Separator,
    Back
};

/**
 * @brief Color scheme for TUI
 */
struct ColorScheme {
    static constexpr int NORMAL = 1;
    static constexpr int HIGHLIGHT = 2;
    static constexpr int TITLE = 3;
    static constexpr int STATUS = 4;
    static constexpr int ERROR = 5;
    static constexpr int SUCCESS = 6;
    static constexpr int BORDER = 7;
    static constexpr int DEVICE = 8;
};

} // namespace easytty
