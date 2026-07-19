cmake_minimum_required(VERSION 3.22)

# Must be set before project() to affect toolchain and suppress macOS 15 SDK unavailability errors in JUCE
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS version")

project(Violent VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.0
)
FetchContent_MakeAvailable(JUCE)

# Suppress deprecation warnings from JUCE's own sources (e.g. CVDisplayLink on macOS 14+)
set_property(DIRECTORY "${juce_SOURCE_DIR}" APPEND PROPERTY
    COMPILE_OPTIONS "-Wno-deprecated-declarations")

juce_add_plugin(Violent
    COMPANY_NAME            "Violent Audio"
    BUNDLE_ID               "com.violentaudio.violent"
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE VlAu
    PLUGIN_CODE             Vlns
    FORMATS                 AU VST3 Standalone
    PRODUCT_NAME            "Violent"
    AU_MAIN_TYPE            kAudioUnitType_MusicDevice
    VST3_CATEGORIES         "Instrument|Synth"
    ICON_BIG                "${CMAKE_CURRENT_SOURCE_DIR}/Assets/icon.png"
)

juce_generate_juce_header(Violent)

target_sources(Violent
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/StandaloneApp.cpp
        Source/SynthEngine.h
        Source/FxChain.h
        Source/StreamChain.h
)

target_compile_definitions(Violent
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
)

target_link_libraries(Violent
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
