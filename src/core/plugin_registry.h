#pragma once
#include <string>
#include <vector>
#include "plugin_entry.h"

struct LoadedPlugin
{
    std::string name;
    void* lib = nullptr;              // handle returned by dlopen
    PluginHandleFn handle = nullptr;  // the plugin's sandy_plugin_handle
};

class PluginRegistry
{
    public:

    PluginRegistry() = default;
    ~PluginRegistry();

    // Owns the dlopen handles, so copying would close them twice
    PluginRegistry(const PluginRegistry&) = delete;
    PluginRegistry& operator=(const PluginRegistry&) = delete;

    // Load every .so in the folder
    void loadAll(const std::string& dir);

    // Pass the command to each plugin until one handles it
    bool dispatch(const std::string& command);

    const std::vector<LoadedPlugin>& plugins() const { return plugins_; }

    private:

    std::vector<LoadedPlugin> plugins_;
};
