#pragma once
#include <string>

// Opens a macOS app by name, e.g. "Google Chrome".
// Returns true if it opened.
bool openApp(const std::string& appName);

bool minimizeApp(const std::string& appName);

bool exitApp(const std::string& appName);
