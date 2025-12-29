#include "common/Utils.hpp"
#include <array>
#include <memory>
#include <cstdio>
#include <unistd.h>
#include <pwd.h>

namespace easytty {
namespace utils {

std::string executeCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        return "";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return trim(result);
}

bool isRoot() {
    return geteuid() == 0;
}

std::string getCurrentUser() {
    struct passwd* pw = getpwuid(getuid());
    if (pw) {
        return std::string(pw->pw_name);
    }
    return "";
}

} // namespace utils
} // namespace easytty
