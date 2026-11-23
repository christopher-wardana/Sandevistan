#include "plugin_registry.h"
#include <dlfcn.h>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

PluginRegistry::~PluginRegistry()
{
    for (auto& p : plugins_) dlclose(p.lib);
}

void PluginRegistry::loadAll(const std::string& dir)
{
    if (!fs::exists(dir))
    {
        std::cerr << "[plugins] folder not found: " << dir << "\n";
        return;
    }

    for (const auto& entry : fs::directory_iterator(dir))
    {
        if (entry.path().extension() != ".so") continue;

        void* lib = dlopen(entry.path().c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!lib)
        {
            std::cerr << "[plugins] failed to load " << entry.path() << ": " << dlerror() << "\n";
            continue;
        }

        auto nameFn   = reinterpret_cast<PluginNameFn>(dlsym(lib, "sandy_plugin_name"));
        auto handleFn = reinterpret_cast<PluginHandleFn>(dlsym(lib, "sandy_plugin_handle"));

        if (!nameFn || !handleFn)
        {
            std::cerr << "[plugins] " << entry.path() << " is missing sandy_plugin_name/sandy_plugin_handle\n";
            dlclose(lib);
            continue;
        }

        plugins_.push_back({ nameFn(), lib, handleFn });
        std::cout << "[plugins] loaded " << plugins_.back().name << "\n";
    }
}

bool PluginRegistry::dispatch(const std::string& command)
{
    for (auto& p : plugins_)
    {
        if (p.handle(command.c_str())) return true;
    }
    return false;
}
