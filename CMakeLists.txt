cmake_minimum_required (VERSION 3.24.1)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS")
set (CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum version of the target platform")

execute_process (
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE AA_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process (
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE AA_GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process (COMMAND uname -n OUTPUT_VARIABLE HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process (
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/juce
    OUTPUT_VARIABLE AA_JUCE_GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include (PamplejuceVersion)

set (PROJECT_NAME "AmbiCreator")

set (PRODUCT_NAME "AmbiCreator${MAJOR_VERSION}")

set (COMPANY_NAME "Austrian Audio")

set (BUNDLE_ID "audio.austrian.software.plugins.ambicreator")

project (${PROJECT_NAME} VERSION ${CURRENT_VERSION})

include (JUCEDefaults)

include (PamplejuceMacOS)

add_subdirectory (JUCE)

set (JUCE_BUILD_LV2 OFF CACHE BOOL "Disable LV2")
set (JUCE_BUILD_ARA OFF CACHE BOOL "Disable ARA")

if (NOT DEFINED FORMATS)
    set (FORMATS Standalone VST3)
endif ()

if (APPLE)
    message (STATUS "Building on MacOS")

    set (FORMATS ${FORMATS} AU AUv3 AAX)

    set_directory_properties (
        PROPERTIES JUCE_AAX_COPY_DIR "$ENV{HOME}/Library/Application Support/Avid/Audio/Plug-Ins"
    )

elseif (UNIX)
    message (STATUS "Building on Linux")
elseif (WIN32)
    message (STATUS "Building on Windows")
    set (FORMATS ${FORMATS} AAX)
else ()
    message (FATAL_ERROR "Unsupported platform")
endif ()

juce_add_plugin (
    "${PROJECT_NAME}"
    ICON_BIG
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icon.png"
    COMPANY_NAME
    "${COMPANY_NAME}"
    BUNDLE_ID
    "${BUNDLE_ID}"
    COPY_PLUGIN_AFTER_BUILD
    ON
    PLUGIN_MANUFACTURER_CODE
    OIDA
    PLUGIN_CODE
    AAAC
    FORMATS
    "${FORMATS}"
    PRODUCT_NAME
    "${PRODUCT_NAME}"
)

add_library (SharedCode INTERFACE)

target_compile_features (SharedCode INTERFACE cxx_std_20)

if (APPLE)
    target_link_options (SharedCode INTERFACE -Wl)
endif ()

set (SourceFiles source/PluginEditor.h source/PluginProcessor.h source/PluginEditor.cpp
                 source/PluginProcessor.cpp
)
file (GLOB_RECURSE SourceLookAndFeelFiles
      CONFIGURE_DEPENDS" ${CMAKE_CURRENT_SOURCE_DIR}/resources/lookAndFeel/*.h"
      "${CMAKE_CURRENT_SOURCE_DIR}/resources/lookAndFeel/*.cpp"
)

target_sources (SharedCode INTERFACE ${SourceFiles} ${SourceLookAndFeelFiles})

target_include_directories (${PROJECT_NAME} INTERFACE ${juce_generated_headers_directory})

include (Assets)

include (XcodePrettify)

target_compile_definitions (
    SharedCode
    INTERFACE JUCE_WEB_BROWSER=0
              JUCE_USE_CURL=0
              JUCE_VST3_CAN_REPLACE_VST2=0
              JUCE_MODAL_LOOPS_PERMITTED=1
              # JUCE_DISPLAY_SPLASH_SCREEN=0
              CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
              VERSION="${CURRENT_VERSION}"
              AA_BUILD_COMMIT_HASH="${AA_GIT_COMMIT_HASH}"
              AA_BUILD_TAG="${AA_GIT_TAG}"
              AA_BUILD_JUCE_VERSION="${AA_JUCE_GIT_TAG}"
              PRODUCT_NAME_WITHOUT_VERSION="AmbiCreator"
)

target_link_libraries (
    SharedCode
    INTERFACE Assets
              juce_audio_basics
              juce_audio_devices
              juce_audio_formats
              juce_audio_processors
              juce_audio_utils
              juce_core
              juce_dsp
              juce_events
              juce_graphics
              juce_gui_basics
              juce_gui_extra
              juce_opengl
              juce::juce_recommended_config_flags
              juce::juce_recommended_lto_flags
              juce::juce_recommended_warning_flags
)

target_link_libraries ("${PROJECT_NAME}" PRIVATE SharedCode)

include (Tests)

include (GitHubENV)
