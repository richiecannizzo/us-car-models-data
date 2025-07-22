#!/bin/bash

echo "🔍 StemMixer VST3 Plugin Setup Verification"
echo "============================================"

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
PASSED=0
FAILED=0

check_command() {
    if command -v "$1" &> /dev/null; then
        echo -e "✅ ${GREEN}$1${NC} - Found"
        ((PASSED++))
    else
        echo -e "❌ ${RED}$1${NC} - Not found"
        ((FAILED++))
    fi
}

check_file() {
    if [ -f "$1" ]; then
        echo -e "✅ ${GREEN}$1${NC} - Found"
        ((PASSED++))
    else
        echo -e "❌ ${RED}$1${NC} - Not found"
        ((FAILED++))
    fi
}

check_directory() {
    if [ -d "$1" ]; then
        echo -e "✅ ${GREEN}$1${NC} - Found"
        ((PASSED++))
    else
        echo -e "❌ ${RED}$1${NC} - Not found"
        ((FAILED++))
    fi
}

echo ""
echo "📋 Checking System Dependencies..."
echo "--------------------------------"

# Check for required system tools
check_command "cmake"
check_command "make"
check_command "gcc"
check_command "g++"
check_command "pkg-config"
check_command "git"

echo ""
echo "📦 Checking Development Libraries..."
echo "-----------------------------------"

# Check for key development packages (by checking for header files)
if pkg-config --exists alsa; then
    echo -e "✅ ${GREEN}ALSA development libraries${NC} - Found"
    ((PASSED++))
else
    echo -e "❌ ${RED}ALSA development libraries${NC} - Not found"
    ((FAILED++))
fi

if pkg-config --exists jack; then
    echo -e "✅ ${GREEN}JACK development libraries${NC} - Found"  
    ((PASSED++))
else
    echo -e "❌ ${RED}JACK development libraries${NC} - Not found"
    ((FAILED++))
fi

if pkg-config --exists freetype2; then
    echo -e "✅ ${GREEN}FreeType development libraries${NC} - Found"
    ((PASSED++))
else
    echo -e "❌ ${RED}FreeType development libraries${NC} - Not found" 
    ((FAILED++))
fi

echo ""
echo "🎵 Checking JUCE Framework..."
echo "----------------------------"

check_directory "JUCE"
check_file "JUCE/CMakeLists.txt"

echo ""
echo "🧠 Checking LibTorch..."
echo "---------------------"

check_directory "/usr/local/libtorch"
check_file "/usr/local/libtorch/lib/libtorch.so"

# Check LD_LIBRARY_PATH
if echo "$LD_LIBRARY_PATH" | grep -q "/usr/local/libtorch/lib"; then
    echo -e "✅ ${GREEN}LD_LIBRARY_PATH includes LibTorch${NC}"
    ((PASSED++))
else
    echo -e "⚠️  ${YELLOW}LD_LIBRARY_PATH may not include LibTorch${NC}"
    echo "   Add this to your ~/.bashrc: export LD_LIBRARY_PATH=/usr/local/libtorch/lib:\$LD_LIBRARY_PATH"
fi

echo ""
echo "📁 Checking Project Structure..."
echo "-------------------------------"

check_directory "Source"
check_file "Source/PluginProcessor.h"
check_file "Source/PluginProcessor.cpp"
check_file "Source/PluginEditor.h"
check_file "Source/PluginEditor.cpp"
check_file "Source/StemSeparator.h"
check_file "Source/StemSeparator.cpp"

check_directory "Assets"
check_file "Assets/htdemucs_traced.pt"

check_file "CMakeLists.txt"
check_file "setup.sh"
check_file "build.sh"

echo ""
echo "🔧 Checking Build Environment..."
echo "-------------------------------"

# Check if we can create a simple CMake project
if command -v cmake &> /dev/null; then
    mkdir -p /tmp/cmake_test
    cat > /tmp/cmake_test/CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.15)
project(test)
set(CMAKE_CXX_STANDARD 17)
EOF
    
    if cd /tmp/cmake_test && cmake . &> /dev/null; then
        echo -e "✅ ${GREEN}CMake can generate build files${NC}"
        ((PASSED++))
    else
        echo -e "❌ ${RED}CMake build test failed${NC}"
        ((FAILED++))
    fi
    
    rm -rf /tmp/cmake_test
fi

echo ""
echo "📈 Summary..."
echo "------------"

TOTAL=$((PASSED + FAILED))
echo "Total checks: $TOTAL"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo ""
    echo -e "🎉 ${GREEN}All checks passed!${NC}"
    echo "You're ready to build the StemMixer VST3 plugin."
    echo ""
    echo "Next steps:"
    echo "1. Ensure you have a proper htdemucs_traced.pt model file in Assets/"
    echo "2. Run: ./build.sh"
    echo "3. Test the plugin in your DAW"
    exit 0
else
    echo ""
    echo -e "⚠️  ${YELLOW}Some checks failed.${NC}"
    echo ""
    echo "To fix issues:"
    echo "1. Run ./setup.sh to install missing dependencies"
    echo "2. Make sure you have a valid htdemucs_traced.pt model file"
    echo "3. Check that LibTorch is properly installed and in LD_LIBRARY_PATH"
    echo ""
    echo "If you continue having issues, check the README.md for troubleshooting."
    exit 1
fi