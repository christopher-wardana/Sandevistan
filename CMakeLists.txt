cmake_minimum_required(VERSION 3.20)
project(project_sandy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # lets VS Code IntelliSense use the real build settings

include(FetchContent)

FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(json)

FetchContent_Declare(whisper
    GIT_REPOSITORY https://github.com/ggml-org/whisper.cpp.git
    GIT_TAG v1.9.5
    GIT_SHALLOW ON
)

set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GGML_OPENMP OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(whisper)





find_package(raylib REQUIRED)

# ---------- Main app ----------
add_executable(sandy
    src/main.cpp
    src/core/gui.cpp
    src/core/parseJson.cpp
    src/core/plugin_registry.cpp
    # add new core .cpp files here
)

target_link_libraries(sandy PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(sandy PRIVATE whisper)


# lets you write #include "core/gui.h" and #include "api/plugin.h"
target_include_directories(sandy PRIVATE src)
target_link_libraries(sandy PRIVATE raylib ${CMAKE_DL_LIBS})   # dlopen/dlsym for loading plugins

# plugins get built into build/plugins; the app gets that path as PLUGIN_DIR
set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/plugins)
target_compile_definitions(sandy PRIVATE PLUGIN_DIR="${PLUGIN_OUTPUT_DIR}")
target_compile_definitions(sandy PRIVATE
    PHRASES_PATH="${CMAKE_SOURCE_DIR}/src/plugins/plugin_app_operations/phrases.json")

# ---------- Plugins ----------
# each folder becomes its own shared library (.so) loaded at runtime
set(PLUGINS
    plugin_app_operations
    plugin_mini_web_browser
)

foreach(plugin ${PLUGINS})
    file(GLOB_RECURSE PLUGIN_SOURCES CONFIGURE_DEPENDS src/plugins/${plugin}/*.cpp)

    if(NOT PLUGIN_SOURCES)
        message(STATUS "Skipping ${plugin}: no .cpp files yet")
        continue()
    endif()

    add_library(${plugin} MODULE ${PLUGIN_SOURCES})
    target_include_directories(${plugin} PRIVATE src)
    target_link_libraries(${plugin} PRIVATE nlohmann_json::nlohmann_json)
    set_target_properties(${plugin} PROPERTIES
        PREFIX ""                                  # plugin_x.so instead of libplugin_x.so
        LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    )

    # macOS: allow plugins to call functions that live in the main app
    if(APPLE)
        target_link_options(${plugin} PRIVATE -undefined dynamic_lookup)
    endif()

    add_dependencies(sandy ${plugin})   # building the app also builds the plugins
endforeach()
