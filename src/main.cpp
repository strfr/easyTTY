#include "app/Application.hpp"
#include "common/Utils.hpp"
#include <iostream>
#include <cstring>

void printUsage(const char* programName) {
    std::cout << "EasyTTY - USB Device Naming Utility\n\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -v, --version  Show version information\n";
    std::cout << "  -l, --list     List connected USB serial devices (non-interactive)\n";
    std::cout << "  -r, --rules    List existing EasyTTY udev rules (non-interactive)\n";
    std::cout << "\n";
    std::cout << "Running without options starts the interactive TUI.\n";
    std::cout << "\n";
    std::cout << "Note: Some operations require root privileges.\n";
    std::cout << "      Run with sudo if you encounter permission errors.\n";
}

void printVersion() {
    std::cout << "EasyTTY version 1.0.0\n";
    std::cout << "USB Device Naming Utility using udev\n";
}

void listDevices() {
    try {
        easytty::DeviceDetector detector;
        auto devices = detector.scanDevices();
        
        if (devices.empty()) {
            std::cout << "No USB serial devices found.\n";
            return;
        }
        
        std::cout << "Found " << devices.size() << " USB serial device(s):\n\n";
        
        for (const auto& dev : devices) {
            std::cout << "Device: " << dev.devPath << "\n";
            std::cout << "  Vendor ID:    " << dev.vendorId << "\n";
            std::cout << "  Product ID:   " << dev.productId << "\n";
            if (!dev.manufacturer.empty()) {
                std::cout << "  Manufacturer: " << dev.manufacturer << "\n";
            }
            if (!dev.product.empty()) {
                std::cout << "  Product:      " << dev.product << "\n";
            }
            if (!dev.serial.empty()) {
                std::cout << "  Serial:       " << dev.serial << "\n";
            }
            if (!dev.driver.empty()) {
                std::cout << "  Driver:       " << dev.driver << "\n";
            }
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void listRules() {
    try {
        easytty::UdevManager manager;
        auto rules = manager.getRules();
        
        if (rules.empty()) {
            std::cout << "No EasyTTY udev rules found.\n";
            return;
        }
        
        std::cout << "Found " << rules.size() << " EasyTTY udev rule(s):\n\n";
        
        for (const auto& rule : rules) {
            std::cout << "Symlink: /dev/" << rule.symlink << "\n";
            std::cout << "  Vendor ID:  " << rule.vendorId << "\n";
            std::cout << "  Product ID: " << rule.productId << "\n";
            if (!rule.serial.empty()) {
                std::cout << "  Serial:     " << rule.serial << "\n";
            }
            std::cout << "  File:       " << rule.filePath << "\n";
            std::cout << "  Active:     " << (manager.verifySymlink(rule.symlink) ? "Yes" : "No") << "\n";
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printVersion();
            return 0;
        }
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            listDevices();
            return 0;
        }
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--rules") == 0) {
            listRules();
            return 0;
        }
    }
    
    // Run interactive TUI
    try {
        easytty::Application app;
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
