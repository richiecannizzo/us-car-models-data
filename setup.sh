#!/bin/bash

set -e

echo "Setting up StemMixer VST3 Plugin Build Environment..."

# Create directories
mkdir -p build

# Install system dependencies
echo "Installing system dependencies..."
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libasound2-dev \
    libjack-jackd2-dev \
    ladspa-sdk \
    libcurl4-openssl-dev \
    libfreetype6-dev \
    libx11-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxcursor-dev \
    libxext-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxrender-dev \
    libwebkit2gtk-4.0-dev \
    libglu1-mesa-dev \
    mesa-common-dev

# Download JUCE if not already present
if [ ! -d "JUCE" ]; then
    echo "Downloading JUCE..."
    git clone https://github.com/juce-framework/JUCE.git
    cd JUCE
    git checkout 7.0.8
    cd ..
else
    echo "JUCE already exists, skipping download..."
fi

# Download and setup LibTorch
echo "Setting up LibTorch..."
if [ ! -d "/usr/local/libtorch" ]; then
    echo "Downloading LibTorch CPU version..."
    wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.1.0%2Bcpu.zip -O libtorch.zip
    unzip -q libtorch.zip
    sudo mv libtorch /usr/local/
    rm libtorch.zip
    
    # Add to LD_LIBRARY_PATH
    echo 'export LD_LIBRARY_PATH=/usr/local/libtorch/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
    export LD_LIBRARY_PATH=/usr/local/libtorch/lib:$LD_LIBRARY_PATH
else
    echo "LibTorch already installed, skipping..."
fi

# Create placeholder model file if it doesn't exist
if [ ! -f "Assets/htdemucs_traced.pt" ]; then
    echo "Creating placeholder model file..."
    echo "WARNING: You need to place the actual htdemucs_traced.pt file in the Assets/ directory"
    echo "For now, creating an empty placeholder file."
    touch Assets/htdemucs_traced.pt
fi

echo ""
echo "Setup complete!"
echo ""
echo "Next steps:"
echo "1. Place your htdemucs_traced.pt model file in the Assets/ directory"
echo "2. Run: cd build && cmake .. && make"
echo "3. The VST3 plugin will be built and copied to your system VST3 directory"
echo ""
echo "Note: You may need to restart your terminal or run 'source ~/.bashrc' to update the LD_LIBRARY_PATH"