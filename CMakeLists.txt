cmake_minimum_required(VERSION 3.22)
project(Violent VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9
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
)

target_sources(Violent
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/SynthEngine.h
)

target_compile_definitions(Violent
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
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
