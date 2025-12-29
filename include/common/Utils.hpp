#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <regex>

namespace easytty {
namespace utils {

/**
 * @brief Trim whitespace from both ends of a string
 */
inline std::string trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
    auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
    return (start < end) ? std::string(start, end) : std::string();
}

/**
 * @brief Split string by delimiter
 */
inline std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * @brief Check if string starts with prefix
 */
inline bool startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}

/**
 * @brief Check if string ends with suffix
 */
inline bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * @brief Convert string to lowercase
 */
inline std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

/**
 * @brief Convert string to uppercase
 */
inline std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

/**
 * @brief Sanitize string for use in udev rules (remove special characters)
 */
inline std::string sanitizeForUdev(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (std::isalnum(c) || c == '_' || c == '-') {
            result += c;
        } else if (c == ' ') {
            result += '_';
        }
    }
    return result;
}

/**
 * @brief Validate symlink name for udev
 */
inline bool isValidSymlinkName(const std::string& name) {
    if (name.empty() || name.length() > 64) {
        return false;
    }
    
    std::regex pattern("^[a-zA-Z][a-zA-Z0-9_-]*$");
    return std::regex_match(name, pattern);
}

/**
 * @brief Format vendor/product ID to 4-digit hex
 */
inline std::string formatHexId(const std::string& id) {
    std::string result = id;
    // Remove 0x prefix if present
    if (result.size() > 2 && result[0] == '0' && (result[1] == 'x' || result[1] == 'X')) {
        result = result.substr(2);
    }
    // Pad to 4 digits
    while (result.length() < 4) {
        result = "0" + result;
    }
    return toLower(result);
}

/**
 * @brief Execute shell command and return output
 */
std::string executeCommand(const std::string& cmd);

/**
 * @brief Check if running as root
 */
bool isRoot();

/**
 * @brief Get current user name
 */
std::string getCurrentUser();

} // namespace utils
} // namespace easytty
