#include "system-info.h"

/// <summary>
/// Getting the computer name
/// </summary>
/// <returns>My computer name</returns>
std::string getMyComputerName() {
#ifdef WIN_OS
    char buffer[256];
    unsigned long size = 256;

    GetComputerName(buffer, &size);
#endif

#ifdef LINUX_OS
    char buffer[HOST_NAME_MAX];
    gethostname(buffer, HOST_NAME_MAX);
#endif

    return std::string(buffer);
}

std::string getExecutableFilepath() {
    std::vector<char> buffer;
    DWORD r = 0;

    do {
        buffer.resize(buffer.size() + MAX_PATH);
        r = GetModuleFileName(nullptr, &buffer.at(0), buffer.size());

    } while (r >= buffer.size());

    buffer.resize(r);
    return std::string(buffer.begin(), buffer.end());
}

std::string getWorkingDirectory() {
    std::string result = std::filesystem::current_path().string();

#ifdef WIN_OS
    std::string exePath = getExecutableFilepath();

    if ((exePath.size() > 0) && (split(exePath, "\\").size() > 0)) {
        result = exePath.substr(0, exePath.rfind("\\"));
    }
#endif

    return result;
}