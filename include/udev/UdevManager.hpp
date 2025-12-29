#pragma once

#include "common/Types.hpp"
#include <vector>
#include <string>
#include <map>

namespace easytty {

/**
 * @brief Manages udev rules for persistent device naming
 * 
 * Handles creation, deletion, and management of udev rules
 * in /etc/udev/rules.d/
 */
class UdevManager {
public:
    UdevManager();
    ~UdevManager() = default;
    
    // Rule priority (lower = earlier processing)
    static constexpr int DEFAULT_PRIORITY = 99;
    static constexpr const char* RULES_DIR = "/etc/udev/rules.d";
    static constexpr const char* RULE_PREFIX = "99-easytty-";
    
    /**
     * @brief Create a new udev rule for a device
     * @param device Device to create rule for
     * @param symlinkName Name for the symlink (without /dev/)
     * @return Operation result
     */
    OperationResult createRule(const DeviceInfo& device, const std::string& symlinkName);
    
    /**
     * @brief Delete an existing udev rule
     * @param ruleName Name of the rule to delete
     * @return Operation result
     */
    OperationResult deleteRule(const std::string& ruleName);
    
    /**
     * @brief Delete rule by file path
     * @param filePath Full path to rule file
     * @return Operation result
     */
    OperationResult deleteRuleFile(const std::string& filePath);
    
    /**
     * @brief Check if a rule already exists for a device
     * @param device Device to check
     * @return True if rule exists
     */
    bool ruleExists(const DeviceInfo& device) const;
    
    /**
     * @brief Get rule match type for a device
     * @param device Device to check
     * @return 0 = no rule, 1 = shared rule (no serial), 2 = unique rule (has serial)
     */
    int getRuleMatchType(const DeviceInfo& device) const;
    
    /**
     * @brief Check if symlink name is already in use
     * @param symlinkName Symlink name to check
     * @return True if name is already used
     */
    bool symlinkExists(const std::string& symlinkName) const;
    
    /**
     * @brief Get all easyTTY managed rules
     * @return Vector of existing rules
     */
    std::vector<UdevRule> getRules() const;
    
    /**
     * @brief Reload udev rules
     * @return Operation result
     */
    OperationResult reloadRules();
    
    /**
     * @brief Trigger udev to re-apply rules
     * @return Operation result
     */
    OperationResult triggerRules();
    
    /**
     * @brief Reload and trigger rules
     * @return Operation result
     */
    OperationResult applyRules();
    
    /**
     * @brief Refresh the list of existing rules
     */
    void refresh();
    
    /**
     * @brief Get existing rules (cached)
     */
    const std::vector<UdevRule>& getExistingRules() const { return rules_; }
    
    /**
     * @brief Verify symlink was created
     * @param symlinkName Name of symlink to check
     * @return True if symlink exists in /dev/
     */
    bool verifySymlink(const std::string& symlinkName) const;

private:
    std::vector<UdevRule> rules_;
    
    /**
     * @brief Generate rule file content
     */
    std::string generateRuleContent(const DeviceInfo& device, const std::string& symlinkName) const;
    
    /**
     * @brief Generate rule file name
     */
    std::string generateRuleFileName(const std::string& symlinkName) const;
    
    /**
     * @brief Parse existing rule file
     */
    std::optional<UdevRule> parseRuleFile(const std::string& filePath) const;
    
    /**
     * @brief Load all existing easyTTY rules
     */
    void loadExistingRules();
    
    /**
     * @brief Check if we have write access to rules directory
     */
    bool hasWriteAccess() const;
    
    /**
     * @brief Write rule to file (may need sudo)
     */
    OperationResult writeRuleFile(const std::string& filePath, const std::string& content);
    
    /**
     * @brief Remove rule file (may need sudo)
     */
    OperationResult removeRuleFile(const std::string& filePath);
};

} // namespace easytty
