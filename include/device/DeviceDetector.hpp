#pragma once

#include "common/Types.hpp"
#include <vector>
#include <memory>
#include <libudev.h>

namespace easytty {

/**
 * @brief Device detector using libudev
 * 
 * Scans the system for serial devices (ttyUSB, ttyACM, etc.)
 * and retrieves their USB attributes for udev rule generation.
 */
class DeviceDetector {
public:
    DeviceDetector();
    ~DeviceDetector();
    
    // Prevent copying
    DeviceDetector(const DeviceDetector&) = delete;
    DeviceDetector& operator=(const DeviceDetector&) = delete;
    
    /**
     * @brief Scan for all serial devices
     * @return Vector of detected devices
     */
    std::vector<DeviceInfo> scanDevices();
    
    /**
     * @brief Scan for a specific device type
     * @param pattern Device pattern (e.g., "ttyUSB", "ttyACM")
     * @return Vector of matching devices
     */
    std::vector<DeviceInfo> scanDevices(const std::string& pattern);
    
    /**
     * @brief Get device info by path
     * @param devPath Device path (e.g., /dev/ttyUSB0)
     * @return Device info if found
     */
    std::optional<DeviceInfo> getDeviceInfo(const std::string& devPath);
    
    /**
     * @brief Refresh device list
     */
    void refresh();
    
    /**
     * @brief Get all currently detected devices
     */
    const std::vector<DeviceInfo>& getDevices() const { return devices_; }

private:
    struct udev* udev_;
    std::vector<DeviceInfo> devices_;
    
    /**
     * @brief Extract device information from udev device
     */
    DeviceInfo extractDeviceInfo(struct udev_device* dev);
    
    /**
     * @brief Find parent USB device
     */
    struct udev_device* findUsbParent(struct udev_device* dev);
    
    /**
     * @brief Get udev attribute safely
     */
    std::string getAttr(struct udev_device* dev, const char* attr);
    
    /**
     * @brief Get sysattr safely
     */
    std::string getSysAttr(struct udev_device* dev, const char* attr);
};

} // namespace easytty
