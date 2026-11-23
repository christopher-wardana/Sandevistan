#include "operate_app_plugin.h"
#include "core/plugin_entry.h"
#include <spawn.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cctype>


#define APPLICATIONS_DIR "/Applications"
namespace fs = std::filesystem;

using namespace std;

extern char** environ;

// ---------- helpers (private to this file) ----------

// Runs a program with its arguments and waits for it.
// argv[0] is the program; the list must end with nullptr.
// Returns true if it exited normally with code 0.
static bool runCommand(const char* const argv[])
{
    pid_t pid;
    if (posix_spawnp(&pid, argv[0], nullptr, nullptr, const_cast<char* const*>(argv), environ) != 0)
        return false;

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) return false;

    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

// Makes text safe to put inside AppleScript "double quotes":
// puts a backslash before any " or \ so the name can't end the string early.
static string escapeForAppleScript(const string& text)
{
    string out;
    for (char c : text)
    {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out;
}

// Runs one line of AppleScript via osascript.
// `script` is a named parameter, so it stays alive for the whole call
// and the c_str() pointer can't dangle.
static bool runAppleScript(const string& script)
{
    const char* argv[] = { "/usr/bin/osascript", "-e", script.c_str(), nullptr };
    return runCommand(argv);
}

// If cmd starts with prefix, put the rest in `arg` and return true (if non-empty).
static bool takeArg(const string& cmd, const string& prefix, string& arg)
{
    if (cmd.rfind(prefix, 0) != 0) return false;
    arg = cmd.substr(prefix.size());
    return !arg.empty();
}

static string nameCheck(const string& appName){
    string appStr;
    string lowerCasedName = appName;

    // Lowercase on input
    transform(lowerCasedName.begin(), lowerCasedName.end(), lowerCasedName.begin(),
            [](unsigned char c) { return tolower(c); });

    unordered_map<string, string> aliasMap = {
        {"chrome", "Google Chrome"},
        {"vs code", "Visual Studio Code"},
        {"calc", "Calculator"},
    };

    string path = APPLICATIONS_DIR;
    for (const auto & entry : fs::directory_iterator(path))
    {
        appStr = entry.path();

        appStr.erase (0, 14);
        appStr.erase(appStr.length() - 4);

        // Lowercase on applications listed name
        transform(appStr.begin(), appStr.end(), appStr.begin(),
                [](unsigned char c) { return tolower(c); });

        if(lowerCasedName == appStr) return appName;
    }

    if(aliasMap.find(lowerCasedName) != aliasMap.end()) return aliasMap.find(lowerCasedName) -> second;

    return "Calculator";
    return "Unrecognized App";
}



// App Operations 

bool openApp(const string& appName)
{
    // Name is its own argument here, no script involved, so no escaping needed
    const char* argv[] = { "open", "-a", appName.c_str(), nullptr };
    return runCommand(argv);
}

bool exitApp(const string& appName)
{
    return runAppleScript("quit app \"" + escapeForAppleScript(appName) + "\"");
}

// ---------- plugin entry points ----------

extern "C" const char* sandy_plugin_name()
{
    return "app_operations";
}

// Handles commands like "open Google Chrome"
extern "C" bool sandy_plugin_handle(const char* command)
{
    const string cmd = command;
    const string prefix = "open ";

    const string app = cmd.substr(prefix.size());

    if (app.empty()) return false;

    string checkedName = nameCheck(app);

    if (cmd.rfind("open ", 0) == 0){
        openApp(checkedName);

    }else if (cmd.rfind("exit ", 0) == 0){
        exitApp(checkedName);

    }else{
        return false;
    }
    
    return true;
}
