#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <sstream>

bool runCommand(const std::string& cmd) {
    std::cout << "[CMD] " << cmd << std::endl;
    return system(cmd.c_str()) == 0;
}

std::string getDownloadsPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        return std::string(path) + "\\Downloads";
    }
    return "C:\\Users\\Default\\Downloads";
}

bool downloadZip(const std::string& url, const std::string& destPath) {
    std::string ps = "powershell -Command \"Invoke-WebRequest -Uri '" + url + "' -OutFile '" + destPath + "'\"";
    return runCommand(ps);
}

bool extractZip(const std::string& zipPath, const std::string& extractTo) {
    std::string ps = "powershell -Command \"Expand-Archive -Path '" + zipPath + "' -DestinationPath '" + extractTo + "' -Force\"";
    return runCommand(ps);
}

bool moveToCDrive(const std::string& extractRoot) {
    std::string from = extractRoot + "\\MercuryLang-main";
    std::string to = "C:\\MercuryLang";
    std::string cmd = "move \"" + from + "\" \"" + to + "\"";
    return runCommand(cmd);
}

bool addToPath(const std::string& path) {
    // Open the system environment variable key for PATH
    HKEY hKey;
    LONG openRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", 0, KEY_WRITE, &hKey);
    if (openRes != ERROR_SUCCESS) {
        std::cerr << "Error opening registry key for PATH!" << std::endl;
        return false;
    }

    // Retrieve the current PATH variable from the registry
    char currentPath[MAX_PATH];
    DWORD size = sizeof(currentPath);
    LONG getRes = RegQueryValueExA(hKey, "Path", NULL, NULL, (LPBYTE)currentPath, &size);
    if (getRes != ERROR_SUCCESS) {
        std::cerr << "Error retrieving current PATH from registry!" << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    // Append the new path to the current PATH
    std::string newPath = std::string(currentPath) + ";" + path;

    // Update the PATH environment variable in the registry
    LONG setRes = RegSetValueExA(hKey, "Path", 0, REG_EXPAND_SZ, (const BYTE*)newPath.c_str(), (newPath.length() + 1) * sizeof(char));
    if (setRes != ERROR_SUCCESS) {
        std::cerr << "Error setting new PATH in registry!" << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    // Close the registry key
    RegCloseKey(hKey);
    return true;
}

int main() {
    std::string repoUrl = "https://github.com/dinhsonhai132/MercuryLang/archive/refs/heads/main.zip";
    std::string downloadsPath = getDownloadsPath();
    std::string zipPath = downloadsPath + "\\MercuryLang-main.zip";
    std::string extractTemp = downloadsPath + "\\MercuryTemp";

    std::cout << "== MercuryLang v2.0.1 ==\n\n";

    system(("mkdir \"" + extractTemp + "\" >nul 2>nul").c_str());

    if (!downloadZip(repoUrl, zipPath)) {
        std::cerr << "Error downloading the ZIP file!\n";
        return 1;
    }

    if (!extractZip(zipPath, extractTemp)) {
        std::cerr << "Error extracting the ZIP file!\n";
        return 1;
    }

    if (!moveToCDrive(extractTemp)) {
        std::cerr << "Error moving the extracted files to C:\\MercuryLang!\n";
        return 1;
    }

    std::cout << "MercuryLang has been successfully downloaded and extracted to C:\\MercuryLang\n";

    // Add C:\MercuryLang\v2.0.1\bin to PATH
    std::string binPath = "C:\\MercuryLang\\v2.0.1\\bin";
    if (!addToPath(binPath)) {
        std::cerr << "Error adding MercuryLang bin directory to PATH!\n";
        return 1;
    }

    std::cout << "MercuryLang bin directory has been added to the PATH!\n";

    return 0;
}
