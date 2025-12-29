#include "app/Application.hpp"
#include "common/Utils.hpp"
#include <sstream>
#include <iomanip>

namespace easytty {

Application::Application()
    : deviceDetector_(std::make_unique<DeviceDetector>())
    , udevManager_(std::make_unique<UdevManager>())
    , running_(true) {}

Application::~Application() {
    if (tui::gScreen) {
        tui::gScreen->cleanup();
        tui::gScreen.reset();
    }
}

int Application::run() {
    // Initialize screen
    tui::gScreen = std::make_unique<tui::Screen>();
    tui::gScreen->init();
    
    // Initial device scan
    deviceDetector_->scanDevices();
    udevManager_->refresh();
    
    // Show main menu
    showMainMenu();
    
    return 0;
}

void Application::showMainMenu() {
    while (running_) {
        // Refresh data before showing menu
        refreshAll();
        
        tui::Menu menu("USB Device Manager", "Manage persistent USB device names with udev rules");
        
        std::vector<tui::MenuItem> items;
        
        // Device count info (freshly scanned)
        size_t deviceCount = deviceDetector_->getDevices().size();
        size_t ruleCount = udevManager_->getExistingRules().size();
        
        items.push_back(tui::MenuItem(
            "List Connected Devices (" + std::to_string(deviceCount) + " found)",
            "Scan and list USB serial devices",
            MenuItemType::Submenu,
            [this]() { showDeviceList(); }
        ));
        
        items.push_back(tui::MenuItem(
            "Manage Existing Rules (" + std::to_string(ruleCount) + " rules)",
            "View, edit, or delete existing udev rules",
            MenuItemType::Submenu,
            [this]() { showExistingRules(); }
        ));
        
        items.push_back(tui::MenuItem::Separator());
        
        items.push_back(tui::MenuItem(
            "Reload & Apply udev Rules",
            "Reload udev rules and trigger device re-enumeration",
            MenuItemType::Action,
            [this]() {
                auto result = udevManager_->applyRules();
                if (result.success) {
                    tui::gScreen->showMessageDialog("Success", result.message, false);
                } else {
                    tui::gScreen->showMessageDialog("Error", result.message, true);
                }
            }
        ));
        
        items.push_back(tui::MenuItem::Separator());
        
        items.push_back(tui::MenuItem(
            "Help",
            "Show usage instructions",
            MenuItemType::Action,
            [this]() { showHelp(); }
        ));
        
        items.push_back(tui::MenuItem(
            "About",
            "About EasyTTY",
            MenuItemType::Action,
            [this]() { showAbout(); }
        ));
        
        items.push_back(tui::MenuItem::Separator());
        
        items.push_back(tui::MenuItem(
            "Exit",
            "Exit the application",
            MenuItemType::Back  // Use Back type to exit menu loop
        ));
        
        menu.setItems(items);
        
        if (!utils::isRoot()) {
            menu.setStatus("Note: Running without root - some operations may require sudo password", false);
        }
        
        int result = menu.run();
        // Any exit from main menu (ESC, Q, or Exit selection) should quit
        running_ = false;
    }
}

void Application::showDeviceList() {
    while (true) {
        // Refresh devices and rules before showing menu
        deviceDetector_->scanDevices();
        udevManager_->refresh();
        
        tui::Menu menu("Connected USB Serial Devices", "Select a device to create a persistent name");
        
        std::vector<tui::MenuItem> items;
        
        const auto& devices = deviceDetector_->getDevices();
        
        if (devices.empty()) {
            items.push_back(tui::MenuItem(
                "No USB serial devices found",
                "",
                MenuItemType::Action,
                nullptr,
                false
            ));
        } else {
            for (const auto& device : devices) {
                // Check if rule already exists for this device
                bool hasRule = udevManager_->ruleExists(device);
                std::string label = formatDeviceForList(device);
                if (hasRule) {
                    label += " [RULE EXISTS]";
                }
                
                items.push_back(tui::MenuItem(
                    label,
                    device.devPath,
                    MenuItemType::Submenu,
                    [this, device]() { showDeviceDetails(device); }
                ));
            }
        }
        
        items.push_back(tui::MenuItem::Separator());
        
        // Use Back type for Refresh so it exits menu loop and rebuilds
        items.push_back(tui::MenuItem(
            "Refresh",
            "Rescan for devices",
            MenuItemType::Back
        ));
        
        items.push_back(tui::MenuItem(
            "< Back to Main Menu",
            "Return to main menu",
            MenuItemType::Back
        ));
        
        menu.setItems(items);
        menu.setHelp("↑/↓: Navigate  Enter: Select device  ESC: Back");
        
        int result = menu.run();
        
        // Check which item was selected
        if (result == -1) {
            // ESC or Q pressed
            return;
        }
        
        // Check if "< Back to Main Menu" was selected (last item)
        if (result >= 0 && static_cast<size_t>(result) == items.size() - 1) {
            return;
        }
        
        // Otherwise it was Refresh or came back from device details, loop continues
    }
}

void Application::showExistingRules() {
    while (true) {
        // Refresh rules before showing menu
        udevManager_->refresh();
        
        tui::Menu menu("Existing udev Rules", "Manage EasyTTY created udev rules");
        
        std::vector<tui::MenuItem> items;
        
        const auto& rules = udevManager_->getExistingRules();
        
        if (rules.empty()) {
            items.push_back(tui::MenuItem(
                "No EasyTTY rules found",
                "",
                MenuItemType::Action,
                nullptr,
                false
            ));
        } else {
            for (const auto& rule : rules) {
                std::string label = formatRuleForList(rule);
                bool symlinkExists = udevManager_->verifySymlink(rule.symlink);
                
                if (symlinkExists) {
                    label += " [ACTIVE]";
                } else {
                    label += " [INACTIVE]";
                }
                
                items.push_back(tui::MenuItem(
                    label,
                    "/dev/" + rule.symlink,
                    MenuItemType::Submenu,
                    [this, rule]() { deleteRuleMenu(rule); }
                ));
            }
        }
        
        items.push_back(tui::MenuItem::Separator());
        
        // Use Back type for Refresh so it exits menu loop and rebuilds
        items.push_back(tui::MenuItem(
            "Refresh",
            "Reload rules from disk",
            MenuItemType::Back
        ));
        
        items.push_back(tui::MenuItem(
            "< Back to Main Menu",
            "Return to main menu",
            MenuItemType::Back
        ));
        
        menu.setItems(items);
        menu.setHelp("↑/↓: Navigate  Enter: Select rule  ESC: Back");
        
        int result = menu.run();
        
        // Check which item was selected
        if (result == -1) {
            // ESC or Q pressed
            return;
        }
        
        // Check if "< Back to Main Menu" was selected (last item)
        if (result >= 0 && static_cast<size_t>(result) == items.size() - 1) {
            return;
        }
        
        // Otherwise it was Refresh or came back from rule details, loop continues
    }
}

void Application::showDeviceDetails(const DeviceInfo& device) {
    // Refresh rules to get latest status
    udevManager_->refresh();
    
    bool stayInMenu = true;
    
    while (stayInMenu) {
        std::stringstream subtitle;
        subtitle << device.devPath << " - " << device.getDisplayName();
        
        tui::Menu menu("Device Details", subtitle.str());
        
        std::vector<tui::MenuItem> items;
        
        // Device info display
        items.push_back(tui::MenuItem("Device Path: " + device.devPath, "", MenuItemType::Action, nullptr, false));
        items.push_back(tui::MenuItem::Separator());
        items.push_back(tui::MenuItem("Vendor ID:    " + device.vendorId, "", MenuItemType::Action, nullptr, false));
        items.push_back(tui::MenuItem("Product ID:   " + device.productId, "", MenuItemType::Action, nullptr, false));
        if (!device.manufacturer.empty()) {
            items.push_back(tui::MenuItem("Manufacturer: " + device.manufacturer, "", MenuItemType::Action, nullptr, false));
        }
        if (!device.product.empty()) {
            items.push_back(tui::MenuItem("Product:      " + device.product, "", MenuItemType::Action, nullptr, false));
        }
        if (!device.serial.empty()) {
            items.push_back(tui::MenuItem("Serial:       " + device.serial, "", MenuItemType::Action, nullptr, false));
        } else {
            items.push_back(tui::MenuItem("Serial:       (none - device has no serial)", "", MenuItemType::Action, nullptr, false));
        }
        if (!device.driver.empty()) {
            items.push_back(tui::MenuItem("Driver:       " + device.driver, "", MenuItemType::Action, nullptr, false));
        }
        if (!device.busNum.empty() && !device.devNum.empty()) {
            items.push_back(tui::MenuItem("USB Location: Bus " + device.busNum + " Dev " + device.devNum, "", MenuItemType::Action, nullptr, false));
        }
        
        items.push_back(tui::MenuItem::Separator());
        
        // Refresh and check if rule exists
        bool hasRule = udevManager_->ruleExists(device);
        
        if (hasRule) {
            items.push_back(tui::MenuItem(
                "Rule already exists for this device",
                "",
                MenuItemType::Action,
                nullptr,
                false
            ));
        } else {
            items.push_back(tui::MenuItem(
                "Create Persistent Name Rule",
                "Create udev rule for this device",
                MenuItemType::Action,
                [this, device, &stayInMenu]() { 
                    createRuleForDevice(device); 
                }
            ));
        }
        
        items.push_back(tui::MenuItem::Separator());
        items.push_back(tui::MenuItem::Back());
        
        menu.setItems(items);
        
        int result = menu.run();
        if (result == -1) {
            stayInMenu = false;
        }
    }
}

void Application::createRuleForDevice(const DeviceInfo& device) {
    // Suggest a default name based on product or vendor
    std::string defaultName;
    if (!device.product.empty()) {
        defaultName = utils::sanitizeForUdev(device.product);
    } else {
        defaultName = device.devNode;
    }
    
    // Show input dialog
    std::string symlinkName = tui::gScreen->showInputDialog(
        "Create Device Rule",
        "Enter symlink name (will appear as /dev/<name>):",
        defaultName
    );
    
    // Trim and validate
    symlinkName = utils::trim(symlinkName);
    
    if (symlinkName.empty()) {
        tui::gScreen->showMessageDialog("Cancelled", "No name entered, rule not created.", false);
        return;
    }
    
    // Validate name
    if (!utils::isValidSymlinkName(symlinkName)) {
        tui::gScreen->showMessageDialog(
            "Invalid Name",
            "Name must start with letter, contain only letters, numbers, _ or -",
            true
        );
        return;
    }
    
    // Confirm creation
    std::stringstream confirmMsg;
    confirmMsg << "Create /dev/" << symlinkName << " for " << device.devPath << "?";
    
    if (!tui::gScreen->showConfirmDialog("Confirm Rule Creation", confirmMsg.str())) {
        return;
    }
    
    // Create the rule
    auto result = udevManager_->createRule(device, symlinkName);
    
    if (result.success) {
        // Apply rules
        auto applyResult = udevManager_->applyRules();
        
        std::stringstream successMsg;
        successMsg << "Rule created: /dev/" << symlinkName;
        if (applyResult.success) {
            successMsg << "\nRules applied successfully!";
        }
        
        tui::gScreen->showMessageDialog("Success", successMsg.str(), false);
    } else {
        tui::gScreen->showMessageDialog("Error", result.message, true);
    }
}

void Application::deleteRuleMenu(const UdevRule& rule) {
    std::stringstream subtitle;
    subtitle << "/dev/" << rule.symlink << " -> " << rule.vendorId << ":" << rule.productId;
    
    tui::Menu menu("Rule Details", subtitle.str());
    
    std::vector<tui::MenuItem> items;
    
    items.push_back(tui::MenuItem("Symlink: /dev/" + rule.symlink, "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("Vendor ID: " + rule.vendorId, "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("Product ID: " + rule.productId, "", MenuItemType::Action, nullptr, false));
    if (!rule.serial.empty()) {
        items.push_back(tui::MenuItem("Serial: " + rule.serial, "", MenuItemType::Action, nullptr, false));
    }
    items.push_back(tui::MenuItem("File: " + rule.filePath, "", MenuItemType::Action, nullptr, false));
    
    items.push_back(tui::MenuItem::Separator());
    
    items.push_back(tui::MenuItem(
        "Delete This Rule",
        "Remove the udev rule",
        MenuItemType::Action,
        [this, rule]() {
            std::string msg = "Delete rule for /dev/" + rule.symlink + "?";
            if (tui::gScreen->showConfirmDialog("Confirm Deletion", msg)) {
                auto result = udevManager_->deleteRuleFile(rule.filePath);
                if (result.success) {
                    udevManager_->applyRules();
                    tui::gScreen->showMessageDialog("Success", "Rule deleted and udev reloaded", false);
                } else {
                    tui::gScreen->showMessageDialog("Error", result.message, true);
                }
            }
        }
    ));
    
    items.push_back(tui::MenuItem::Separator());
    items.push_back(tui::MenuItem::Back());
    
    menu.setItems(items);
    menu.run();
}

void Application::showHelp() {
    tui::Menu helpMenu("Help", "EasyTTY Usage Guide");
    
    std::vector<tui::MenuItem> items;
    
    items.push_back(tui::MenuItem("=== WHAT IS EASYTTY ===", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("EasyTTY creates persistent names for USB serial devices.", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("Instead of /dev/ttyUSB0, your device can be /dev/RS485_1", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem::Separator());
    
    items.push_back(tui::MenuItem("=== HOW TO USE ===", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("1. Connect your USB device", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("2. Go to 'List Connected Devices'", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("3. Select your device", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("4. Choose 'Create Persistent Name Rule'", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("5. Enter your desired name (e.g., RS485_1)", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("6. The symlink /dev/RS485_1 will be created", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem::Separator());
    
    items.push_back(tui::MenuItem("=== NAVIGATION ===", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("↑/↓ or j/k: Navigate menu items", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("Enter: Select/Execute item", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("ESC: Go back / Cancel", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("Q: Quit application", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem::Separator());
    
    items.push_back(tui::MenuItem::Back());
    
    helpMenu.setItems(items);
    helpMenu.run();
}

void Application::showAbout() {
    tui::Menu aboutMenu("About EasyTTY", "USB Device Naming Utility");
    
    std::vector<tui::MenuItem> items;
    
    items.push_back(tui::MenuItem("EasyTTY v1.0.0", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem::Separator());
    items.push_back(tui::MenuItem("A KConfig-style TUI application for managing", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("persistent USB serial device names using udev.", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem::Separator());
    items.push_back(tui::MenuItem("Features:", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("  - Automatic device detection", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("  - USB attribute extraction", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("  - udev rule generation", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("  - Rule management (add/delete)", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem("  - Automatic rule application", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem::Separator());
    items.push_back(tui::MenuItem("Built with ncurses and libudev", "", MenuItemType::Action, nullptr, false));
    items.push_back(tui::MenuItem::Separator());
    
    items.push_back(tui::MenuItem::Back());
    
    aboutMenu.setItems(items);
    aboutMenu.run();
}

void Application::refreshAll() {
    deviceDetector_->scanDevices();
    udevManager_->refresh();
}

std::string Application::formatDeviceForList(const DeviceInfo& device) const {
    std::stringstream ss;
    ss << device.devNode;
    
    if (!device.product.empty()) {
        ss << " - " << device.product;
    } else if (!device.manufacturer.empty()) {
        ss << " - " << device.manufacturer;
    }
    
    ss << " [" << device.vendorId << ":" << device.productId;
    
    // Add serial if available (helps distinguish identical devices)
    if (!device.serial.empty()) {
        // Truncate long serials for display
        std::string shortSerial = device.serial;
        if (shortSerial.length() > 8) {
            shortSerial = shortSerial.substr(0, 8) + "..";
        }
        ss << " S:" << shortSerial;
    }
    
    ss << "]";
    
    return ss.str();
}

std::string Application::formatRuleForList(const UdevRule& rule) const {
    std::stringstream ss;
    ss << rule.symlink;
    ss << " [" << rule.vendorId << ":" << rule.productId;
    
    if (!rule.serial.empty()) {
        std::string shortSerial = rule.serial;
        if (shortSerial.length() > 8) {
            shortSerial = shortSerial.substr(0, 8) + "..";
        }
        ss << " S:" << shortSerial;
    }
    
    ss << "]";
    
    return ss.str();
}

} // namespace easytty
