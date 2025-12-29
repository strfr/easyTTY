#include "device/DeviceDetector.hpp"
#include "common/Utils.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace easytty {

DeviceDetector::DeviceDetector() {
    udev_ = udev_new();
    if (!udev_) {
        throw std::runtime_error("Failed to initialize udev");
    }
}

DeviceDetector::~DeviceDetector() {
    if (udev_) {
        udev_unref(udev_);
    }
}

std::vector<DeviceInfo> DeviceDetector::scanDevices() {
    devices_.clear();
    
    struct udev_enumerate* enumerate = udev_enumerate_new(udev_);
    if (!enumerate) {
        return devices_;
    }
    
    // Add matching subsystems
    udev_enumerate_add_match_subsystem(enumerate, "tty");
    udev_enumerate_scan_devices(enumerate);
    
    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* entry;
    
    udev_list_entry_foreach(entry, devices) {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* dev = udev_device_new_from_syspath(udev_, path);
        
        if (dev) {
            const char* devNode = udev_device_get_devnode(dev);
            if (devNode) {
                std::string devPath(devNode);
                // Filter for serial devices
                if (devPath.find("ttyUSB") != std::string::npos ||
                    devPath.find("ttyACM") != std::string::npos ||
                    devPath.find("ttyAMA") != std::string::npos ||
                    devPath.find("ttySC") != std::string::npos) {
                    
                    DeviceInfo info = extractDeviceInfo(dev);
                    if (info.isValid()) {
                        devices_.push_back(info);
                    }
                }
            }
            udev_device_unref(dev);
        }
    }
    
    udev_enumerate_unref(enumerate);
    
    // Sort by device node
    std::sort(devices_.begin(), devices_.end(), 
              [](const DeviceInfo& a, const DeviceInfo& b) {
                  return a.devPath < b.devPath;
              });
    
    return devices_;
}

std::vector<DeviceInfo> DeviceDetector::scanDevices(const std::string& pattern) {
    scanDevices();
    
    std::vector<DeviceInfo> filtered;
    std::copy_if(devices_.begin(), devices_.end(), std::back_inserter(filtered),
                 [&pattern](const DeviceInfo& dev) {
                     return dev.devPath.find(pattern) != std::string::npos;
                 });
    
    return filtered;
}

std::optional<DeviceInfo> DeviceDetector::getDeviceInfo(const std::string& devPath) {
    struct udev_device* dev = udev_device_new_from_devnum(
        udev_, 'c', 
        std::filesystem::status(devPath).type() == std::filesystem::file_type::character 
            ? 0 : 0);
    
    // Try finding by syspath instead
    std::string sysPath = "/sys/class/tty/" + 
                          devPath.substr(devPath.rfind('/') + 1);
    
    dev = udev_device_new_from_syspath(udev_, sysPath.c_str());
    
    if (!dev) {
        // Scan all and find matching
        scanDevices();
        for (const auto& device : devices_) {
            if (device.devPath == devPath) {
                return device;
            }
        }
        return std::nullopt;
    }
    
    DeviceInfo info = extractDeviceInfo(dev);
    udev_device_unref(dev);
    
    return info.isValid() ? std::optional<DeviceInfo>(info) : std::nullopt;
}

void DeviceDetector::refresh() {
    scanDevices();
}

DeviceInfo DeviceDetector::extractDeviceInfo(struct udev_device* dev) {
    DeviceInfo info;
    
    const char* devNode = udev_device_get_devnode(dev);
    if (devNode) {
        info.devPath = devNode;
        info.devNode = std::string(devNode).substr(std::string(devNode).rfind('/') + 1);
    }
    
    const char* sysPath = udev_device_get_syspath(dev);
    if (sysPath) {
        info.sysPath = sysPath;
    }
    
    const char* subsystem = udev_device_get_subsystem(dev);
    if (subsystem) {
        info.subsystem = subsystem;
    }
    
    // Get USB parent device for attributes
    struct udev_device* usb_dev = findUsbParent(dev);
    if (usb_dev) {
        info.vendorId = utils::formatHexId(getSysAttr(usb_dev, "idVendor"));
        info.productId = utils::formatHexId(getSysAttr(usb_dev, "idProduct"));
        info.serial = getSysAttr(usb_dev, "serial");
        info.manufacturer = getSysAttr(usb_dev, "manufacturer");
        info.product = getSysAttr(usb_dev, "product");
        info.busNum = getSysAttr(usb_dev, "busnum");
        info.devNum = getSysAttr(usb_dev, "devnum");
        
        // Get kernel path (USB port path like "1-2.3") for physical location
        const char* sysName = udev_device_get_sysname(usb_dev);
        if (sysName) {
            info.kernelPath = sysName;
        }
        
        const char* driver = udev_device_get_driver(usb_dev);
        if (driver) {
            info.driver = driver;
        }
    }
    
    // Get interface driver and interface number
    struct udev_device* intf_dev = udev_device_get_parent_with_subsystem_devtype(
        dev, "usb", "usb_interface");
    if (intf_dev) {
        const char* driver = udev_device_get_driver(intf_dev);
        if (driver) {
            info.driver = driver;
        }
        info.interfaceNum = getSysAttr(intf_dev, "bInterfaceNumber");
    }
    
    return info;
}

struct udev_device* DeviceDetector::findUsbParent(struct udev_device* dev) {
    return udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
}

std::string DeviceDetector::getAttr(struct udev_device* dev, const char* attr) {
    const char* value = udev_device_get_property_value(dev, attr);
    return value ? std::string(value) : "";
}

std::string DeviceDetector::getSysAttr(struct udev_device* dev, const char* attr) {
    const char* value = udev_device_get_sysattr_value(dev, attr);
    return value ? utils::trim(std::string(value)) : "";
}

} // namespace easytty
