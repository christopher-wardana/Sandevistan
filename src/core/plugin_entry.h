#pragma once

// Every plugin must export these two functions.
// The core finds them by name at runtime (dlsym), so they're extern "C"
// to stop C++ from mangling the names.
extern "C" {
    // Short name for the plugin, e.g. "app_operations"
    const char* sandy_plugin_name();

    // Try to handle a command the user typed.
    // Return true if this plugin handled it, false to let the next plugin try.
    bool sandy_plugin_handle(const char* command);
}

using PluginNameFn   = const char* (*)();
using PluginHandleFn = bool (*)(const char*);
