#include "udev/UdevManager.hpp"
#include "common/Utils.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <unistd.h>

namespace fs = std::filesystem;

namespace easytty {

// Implement UdevRule::generateRule
std::string UdevRule::generateRule() const {
    std::stringstream ss;
    ss << "# EasyTTY auto-generated rule for " << name << "\n";
    ss << "# Created by easyTTY - USB device persistent naming\n";
    ss << "SUBSYSTEM==\"tty\", ";
    ss << "ATTRS{idVendor}==\"" << vendorId << "\", ";
    ss << "ATTRS{idProduct}==\"" << productId << "\"";
    
    if (!serial.empty()) {
        ss << ", ATTRS{serial}==\"" << serial << "\"";
    }
    
    ss << ", SYMLINK+=\"" << symlink << "\", MODE=\"0666\"";
    
    return ss.str();
}

std::string UdevRule::getFileName() const {
    return std::to_string(priority) + "-easytty-" + symlink + ".rules";
}

// UdevManager implementation
UdevManager::UdevManager() {
    loadExistingRules();
}

OperationResult UdevManager::createRule(const DeviceInfo& device, const std::string& symlinkName) {
    // Validate symlink name
    if (!utils::isValidSymlinkName(symlinkName)) {
        return OperationResult::Failure("Invalid symlink name. Use only letters, numbers, underscores, and hyphens. Must start with a letter.");
    }
    
    // Check if device is valid
    if (!device.isValid()) {
        return OperationResult::Failure("Invalid device information");
    }
    
    // Check if symlink already exists
    if (symlinkExists(symlinkName)) {
        return OperationResult::Failure("Symlink name '" + symlinkName + "' is already in use");
    }
    
    // Check if rule for this device already exists
    for (const auto& rule : rules_) {
        if (rule.vendorId == device.vendorId && 
            rule.productId == device.productId &&
            (device.serial.empty() || rule.serial == device.serial)) {
            return OperationResult::Failure("A rule for this device already exists as '" + rule.symlink + "'");
        }
    }
    
    // Generate rule content
    std::string content = generateRuleContent(device, symlinkName);
    std::string fileName = generateRuleFileName(symlinkName);
    std::string filePath = std::string(RULES_DIR) + "/" + fileName;
    
    // Write rule file
    auto result = writeRuleFile(filePath, content);
    if (!result.success) {
        return result;
    }
    
    // Reload rules
    loadExistingRules();
    
    return OperationResult::Success("Rule created successfully: /dev/" + symlinkName);
}

OperationResult UdevManager::deleteRule(const std::string& ruleName) {
    // Find rule with matching symlink or name
    auto it = std::find_if(rules_.begin(), rules_.end(),
                          [&ruleName](const UdevRule& rule) {
                              return rule.symlink == ruleName || rule.name == ruleName;
                          });
    
    if (it == rules_.end()) {
        return OperationResult::Failure("Rule not found: " + ruleName);
    }
    
    return deleteRuleFile(it->filePath);
}

OperationResult UdevManager::deleteRuleFile(const std::string& filePath) {
    auto result = removeRuleFile(filePath);
    if (result.success) {
        loadExistingRules();
    }
    return result;
}

bool UdevManager::ruleExists(const DeviceInfo& device) const {
    return std::any_of(rules_.begin(), rules_.end(),
                      [&device](const UdevRule& rule) {
                          return rule.vendorId == device.vendorId &&
                                 rule.productId == device.productId &&
                                 (device.serial.empty() || rule.serial == device.serial);
                      });
}

bool UdevManager::symlinkExists(const std::string& symlinkName) const {
    return std::any_of(rules_.begin(), rules_.end(),
                      [&symlinkName](const UdevRule& rule) {
                          return rule.symlink == symlinkName;
                      });
}

std::vector<UdevRule> UdevManager::getRules() const {
    return rules_;
}

OperationResult UdevManager::reloadRules() {
    std::string output = utils::executeCommand("sudo udevadm control --reload-rules 2>&1");
    
    // Check for errors
    if (output.find("error") != std::string::npos || 
        output.find("failed") != std::string::npos) {
        return OperationResult::Failure("Failed to reload rules: " + output);
    }
    
    return OperationResult::Success("Rules reloaded successfully");
}

OperationResult UdevManager::triggerRules() {
    std::string output = utils::executeCommand("sudo udevadm trigger 2>&1");
    
    if (output.find("error") != std::string::npos || 
        output.find("failed") != std::string::npos) {
        return OperationResult::Failure("Failed to trigger rules: " + output);
    }
    
    return OperationResult::Success("Rules triggered successfully");
}

OperationResult UdevManager::applyRules() {
    auto reloadResult = reloadRules();
    if (!reloadResult.success) {
        return reloadResult;
    }
    
    auto triggerResult = triggerRules();
    if (!triggerResult.success) {
        return triggerResult;
    }
    
    return OperationResult::Success("Rules reloaded and applied successfully");
}

void UdevManager::refresh() {
    loadExistingRules();
}

bool UdevManager::verifySymlink(const std::string& symlinkName) const {
    return fs::exists("/dev/" + symlinkName);
}

std::string UdevManager::generateRuleContent(const DeviceInfo& device, const std::string& symlinkName) const {
    std::stringstream ss;
    
    ss << "# EasyTTY auto-generated rule\n";
    ss << "# Device: " << device.getDisplayName() << "\n";
    ss << "# Vendor: " << device.manufacturer << " (" << device.vendorId << ")\n";
    ss << "# Product: " << device.product << " (" << device.productId << ")\n";
    if (!device.serial.empty()) {
        ss << "# Serial: " << device.serial << "\n";
    }
    ss << "# Original: " << device.devPath << "\n";
    ss << "# Created: " << utils::executeCommand("date") << "\n";
    ss << "\n";
    
    ss << "SUBSYSTEM==\"tty\", ";
    ss << "ATTRS{idVendor}==\"" << device.vendorId << "\", ";
    ss << "ATTRS{idProduct}==\"" << device.productId << "\"";
    
    if (!device.serial.empty()) {
        ss << ", ATTRS{serial}==\"" << device.serial << "\"";
    }
    
    ss << ", SYMLINK+=\"" << symlinkName << "\", MODE=\"0666\"\n";
    
    return ss.str();
}

std::string UdevManager::generateRuleFileName(const std::string& symlinkName) const {
    return std::to_string(DEFAULT_PRIORITY) + "-easytty-" + symlinkName + ".rules";
}

std::optional<UdevRule> UdevManager::parseRuleFile(const std::string& filePath) const {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    UdevRule rule;
    rule.filePath = filePath;
    rule.isActive = true;
    
    // Extract priority from filename
    std::string filename = fs::path(filePath).filename().string();
    try {
        rule.priority = std::stoi(filename.substr(0, 2));
    } catch (...) {
        rule.priority = DEFAULT_PRIORITY;
    }
    
    std::string line;
    std::regex vendorRegex("ATTRS\\{idVendor\\}==\"([0-9a-fA-F]+)\"");
    std::regex productRegex("ATTRS\\{idProduct\\}==\"([0-9a-fA-F]+)\"");
    std::regex serialRegex("ATTRS\\{serial\\}==\"([^\"]+)\"");
    std::regex symlinkRegex("SYMLINK\\+=\"([^\"]+)\"");
    
    while (std::getline(file, line)) {
        // Skip comments for rule parsing, but extract device name from comments
        if (line.find("# Device:") != std::string::npos) {
            rule.name = utils::trim(line.substr(line.find(":") + 1));
        }
        
        if (line.empty() || line[0] == '#') continue;
        
        std::smatch match;
        
        if (std::regex_search(line, match, vendorRegex)) {
            rule.vendorId = match[1].str();
        }
        
        if (std::regex_search(line, match, productRegex)) {
            rule.productId = match[1].str();
        }
        
        if (std::regex_search(line, match, serialRegex)) {
            rule.serial = match[1].str();
        }
        
        if (std::regex_search(line, match, symlinkRegex)) {
            rule.symlink = match[1].str();
        }
    }
    
    // Validate parsed rule
    if (rule.vendorId.empty() || rule.symlink.empty()) {
        return std::nullopt;
    }
    
    if (rule.name.empty()) {
        rule.name = rule.symlink;
    }
    
    return rule;
}

void UdevManager::loadExistingRules() {
    rules_.clear();
    
    if (!fs::exists(RULES_DIR)) {
        return;
    }
    
    for (const auto& entry : fs::directory_iterator(RULES_DIR)) {
        if (!entry.is_regular_file()) continue;
        
        std::string filename = entry.path().filename().string();
        
        // Only process easyTTY rules
        if (filename.find("easytty") == std::string::npos) continue;
        if (!utils::endsWith(filename, ".rules")) continue;
        
        auto rule = parseRuleFile(entry.path().string());
        if (rule) {
            rules_.push_back(*rule);
        }
    }
    
    // Sort by symlink name
    std::sort(rules_.begin(), rules_.end(),
              [](const UdevRule& a, const UdevRule& b) {
                  return a.symlink < b.symlink;
              });
}

bool UdevManager::hasWriteAccess() const {
    return fs::exists(RULES_DIR) && 
           (utils::isRoot() || access(RULES_DIR, W_OK) == 0);
}

OperationResult UdevManager::writeRuleFile(const std::string& filePath, const std::string& content) {
    if (utils::isRoot()) {
        // Direct write if root
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return OperationResult::Failure("Failed to create rule file: " + filePath);
        }
        file << content;
        file.close();
        return OperationResult::Success();
    }
    
    // Use sudo tee for non-root
    std::string cmd = "echo '" + content + "' | sudo tee " + filePath + " > /dev/null 2>&1";
    int ret = system(cmd.c_str());
    
    if (ret != 0) {
        return OperationResult::Failure("Failed to create rule file (sudo required)");
    }
    
    return OperationResult::Success();
}

OperationResult UdevManager::removeRuleFile(const std::string& filePath) {
    if (!fs::exists(filePath)) {
        return OperationResult::Failure("Rule file does not exist: " + filePath);
    }
    
    if (utils::isRoot()) {
        try {
            fs::remove(filePath);
            return OperationResult::Success("Rule deleted successfully");
        } catch (const std::exception& e) {
            return OperationResult::Failure(std::string("Failed to delete rule: ") + e.what());
        }
    }
    
    // Use sudo rm for non-root
    std::string cmd = "sudo rm -f " + filePath + " 2>&1";
    std::string output = utils::executeCommand(cmd);
    
    if (fs::exists(filePath)) {
        return OperationResult::Failure("Failed to delete rule file (sudo required)");
    }
    
    return OperationResult::Success("Rule deleted successfully");
}

} // namespace easytty
