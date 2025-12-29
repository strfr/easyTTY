#pragma once

#include "device/DeviceDetector.hpp"
#include "udev/UdevManager.hpp"
#include "tui/Screen.hpp"
#include "tui/Menu.hpp"
#include <memory>

namespace easytty {

/**
 * @brief Main application class
 * 
 * Coordinates all components and provides the main application logic
 */
class Application {
public:
    Application();
    ~Application();
    
    /**
     * @brief Run the application
     * @return Exit code
     */
    int run();

private:
    std::unique_ptr<DeviceDetector> deviceDetector_;
    std::unique_ptr<UdevManager> udevManager_;
    bool running_;
    
    // Menu handlers
    void showMainMenu();
    void showDeviceList();
    void showExistingRules();
    void showDeviceDetails(const DeviceInfo& device);
    void createRuleForDevice(const DeviceInfo& device);
    void deleteRuleMenu(const UdevRule& rule);
    void showHelp();
    void showAbout();
    
    // Utility
    void refreshAll();
    std::string formatDeviceForList(const DeviceInfo& device) const;
    std::string formatRuleForList(const UdevRule& rule) const;
};

} // namespace easytty
