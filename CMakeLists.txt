cmake_minimum_required(VERSION 3.15)

project(StemMixer VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(PkgConfig REQUIRED)

# Add JUCE
add_subdirectory(JUCE)

# Find LibTorch
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local/libtorch")
find_package(Torch REQUIRED)

# Create the plugin target
juce_add_plugin(StemMixer
    COMPANY_NAME "StemMixer"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Stem
    PLUGIN_CODE StMx
    FORMATS VST3
    PRODUCT_NAME "StemMixer"
    VST3_CATEGORIES "Fx"
)

# Generate JuceHeader.h
juce_generate_juce_header(StemMixer)

# Add source files
target_sources(StemMixer
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/StemSeparator.cpp
)

# Add compile definitions
target_compile_definitions(StemMixer
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link libraries
target_link_libraries(StemMixer
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
        ${TORCH_LIBRARIES}
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Set torch compile flags
set_property(TARGET StemMixer PROPERTY CXX_STANDARD 17)

# Copy model file to build directory
configure_file(${CMAKE_SOURCE_DIR}/Assets/htdemucs_traced.pt 
               ${CMAKE_BINARY_DIR}/htdemucs_traced.pt COPYONLY)