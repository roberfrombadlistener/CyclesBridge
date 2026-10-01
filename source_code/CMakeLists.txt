cmake_minimum_required(VERSION 3.22)

project(CyclesBridge VERSION 0.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.3
)

FetchContent_MakeAvailable(JUCE)

add_library(CyclesBridgeCommon INTERFACE)

target_include_directories(
    CyclesBridgeCommon
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Source/Common
)

target_link_libraries(
    CyclesBridgeCommon
    INTERFACE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_events
)

# ============================================================
# Standalone bridge application
# ============================================================

juce_add_gui_app(
    CyclesBridgeApp
    PRODUCT_NAME "CyclesBridge"
    COMPANY_NAME "CyclesBridge"
    BUNDLE_ID "com.cyclesbridge.bridge"
)

juce_generate_juce_header(CyclesBridgeApp)

target_sources(
    CyclesBridgeApp
    PRIVATE
        Source/Bridge/Main.cpp
        Source/Bridge/MainComponent.cpp
        Source/Bridge/MainComponent.h
        Source/Bridge/UsbAudioSender.cpp
        Source/Bridge/UsbAudioSender.h
)

target_link_libraries(
    CyclesBridgeApp
    PRIVATE
        CyclesBridgeCommon
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(
    CyclesBridgeApp
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_ASIO=1
)

# ============================================================
# VST3 plugin
# ============================================================

juce_add_plugin(
    CyclesBridgePlugin
    COMPANY_NAME "CyclesBridge"
    PRODUCT_NAME "CyclesBridge"
    BUNDLE_ID "com.cyclesbridge.plugin"
    PLUGIN_MANUFACTURER_CODE CyBr
    PLUGIN_CODE CyB6
    FORMATS VST3
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Instrument"
)

juce_generate_juce_header(CyclesBridgePlugin)

target_sources(
    CyclesBridgePlugin
    PRIVATE
        Source/Plugin/PluginProcessor.cpp
        Source/Plugin/PluginProcessor.h
        Source/Plugin/PluginEditor.cpp
        Source/Plugin/PluginEditor.h
        Source/Plugin/NetworkReceiver.cpp
        Source/Plugin/NetworkReceiver.h
        Source/Plugin/AudioRingBuffer.h
)

target_link_libraries(
    CyclesBridgePlugin
    PRIVATE
        CyclesBridgeCommon
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(
    CyclesBridgePlugin
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)
