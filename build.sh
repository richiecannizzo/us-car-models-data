#!/bin/bash

set -e

echo "Building StemMixer VST3 Plugin..."

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the plugin
echo "Building plugin..."
make -j$(nproc)

echo ""
echo "Build complete!"
echo ""

# Check if the VST3 was built successfully
if [ -f "StemMixer_artefacts/Release/VST3/StemMixer.vst3" ]; then
    echo "✅ VST3 plugin built successfully!"
    echo "📁 Plugin location: build/StemMixer_artefacts/Release/VST3/StemMixer.vst3"
    
    # Copy to system VST3 directory
    VST3_DIR="$HOME/.vst3"
    mkdir -p "$VST3_DIR"
    
    echo "📋 Copying plugin to $VST3_DIR..."
    cp -r "StemMixer_artefacts/Release/VST3/StemMixer.vst3" "$VST3_DIR/"
    
    echo "✅ Plugin installed to $VST3_DIR/StemMixer.vst3"
    echo ""
    echo "🎵 You can now use StemMixer in your DAW!"
    echo ""
    echo "⚠️  Remember to place your htdemucs_traced.pt model file in the same directory as the plugin binary"
    echo "   or in the build directory for the plugin to work properly."
else
    echo "❌ Build failed - VST3 plugin not found"
    exit 1
fi