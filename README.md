# About Abhilash Reddy: Fullstack Software Development Expertise

I'm Abhilash Reddy, a dedicated fullstack software developer with a deep focus on Rails and React. My commitment is to provide adaptive and innovative solutions, tailored to the unique demands of startups and fast-growing teams. If you're seeking a collaborative partner who blends technical skill with a flexible approach, feel free to connect with me at [hello@grabians.com](mailto:hello@grabians.com).

---

# US Car Models Data (1992 - 2026)

Embark on a journey through the history of American automotive engineering with our extensive dataset. Featuring over 15,000 car models from 1992 to 2026, this resource is invaluable for researchers, industry professionals, and enthusiasts eager to explore automotive trends and milestones.

Download the dataset today to start your exploration.

## Data Integration Guides

- [MySQL Integration](http://www.mysqltutorial.org/import-csv-file-mysql-table/)
- [PostgreSQL Integration](http://www.postgresqltutorial.com/import-csv-file-into-posgresql-table/)
- [MongoDB Integration](https://docs.mongodb.com/manual/reference/program/mongoimport/)

# StemMixer VST3 Plugin

**Real-time audio stem separation plugin for Linux using Demucs AI model**

StemMixer is a VST3 audio effect plugin that performs real-time stem separation on stereo audio input, splitting it into 4 separate stems (vocals, drums, bass, and other instruments) using a pre-trained Demucs TorchScript model. Each stem can be individually controlled with gain, mute, and solo controls.

![StemMixer Interface](docs/interface-preview.png)

## Features

- 🎵 **Real-time stem separation** using Demucs AI model
- 🎛️ **Individual stem controls** with gain sliders (0-200%)
- 🔇 **Mute/Solo buttons** for each stem
- 🎨 **Color-coded UI** for easy identification of stems
- ⚡ **Optimized for performance** with chunk-based processing
- 🎧 **VST3 format** compatible with most Linux DAWs
- 🧠 **PyTorch integration** via TorchScript

## Stems

- **🟢 Vocals** - Lead and backing vocals
- **🔵 Drums** - Drums and percussion
- **🟠 Bass** - Bass guitar and low-frequency instruments  
- **🟣 Other** - All other instruments (guitars, synths, etc.)

## Requirements

### System Requirements

- **OS**: Ubuntu Linux 20.04+ (or compatible)
- **CPU**: Multi-core processor recommended
- **RAM**: 4GB+ recommended
- **Storage**: ~2GB for dependencies + model file

### Dependencies

- CMake 3.15+
- GCC/G++ with C++17 support
- JUCE Framework 7.0.8
- LibTorch (PyTorch C++ API)
- Various Linux audio development libraries

## Quick Start

### 1. Setup Environment

```bash
# Make setup script executable
chmod +x setup.sh

# Run setup (installs dependencies, downloads JUCE and LibTorch)
./setup.sh
```

### 2. Get the Demucs Model

You need to obtain the `htdemucs_traced.pt` TorchScript model file. This is a pre-trained Demucs model that has been traced for TorchScript compatibility.

```bash
# Place your model file in the Assets directory
cp /path/to/your/htdemucs_traced.pt Assets/
```

**Note**: The model file is not included in this repository due to size constraints. You can:
- Train your own model using the [Demucs](https://github.com/facebookresearch/demucs) framework
- Convert an existing Demucs model to TorchScript format
- Find pre-converted models from the community

### 3. Build the Plugin

```bash
# Make build script executable
chmod +x build.sh

# Build and install the plugin
./build.sh
```

### 4. Use in Your DAW

The plugin will be installed to `~/.vst3/StemMixer.vst3`. Open your DAW (Bitwig Studio, Ardour, etc.) and scan for new plugins.

## Manual Build

If you prefer to build manually:

```bash
# Create and enter build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Install
cp -r StemMixer_artefacts/Release/VST3/StemMixer.vst3 ~/.vst3/
```

## Usage

1. **Load the plugin** on an audio track in your DAW
2. **Play audio** through the track
3. **Adjust stem levels** using the gain sliders
4. **Mute/Solo stems** using the M/S buttons
5. **Real-time processing** - changes are applied immediately

### Controls

- **Gain Sliders**: Adjust the volume of each stem (0-200%)
- **Mute (M) Button**: Completely silence a stem (red when active)
- **Solo (S) Button**: Isolate one or more stems (yellow when active)

### Tips

- **Processing Latency**: The plugin processes audio in 1-second chunks (44.1kHz), so expect ~1 second of latency
- **CPU Usage**: AI processing is CPU-intensive; close other applications if needed
- **Model Quality**: The separation quality depends entirely on the Demucs model used

## Technical Details

### Architecture

- **Audio Processing**: Chunk-based processing (44,100 samples at 44.1kHz)
- **AI Integration**: TorchScript model inference via LibTorch
- **UI Framework**: JUCE with custom styling
- **Threading**: Real-time audio thread with AI processing

### Model Requirements

- **Input**: Stereo audio tensor `[1, 2, 44100]`
- **Output**: 4-stem tensor `[1, 4, 2, 44100]`
- **Format**: TorchScript (.pt file)
- **Sample Rate**: 44.1kHz (fixed)

### Performance

- **Chunk Size**: 1 second (44,100 samples)
- **Latency**: ~1 second due to chunk processing
- **CPU Usage**: Varies by model complexity
- **Memory**: ~500MB+ depending on model size

## Tested DAWs

- ✅ **Bitwig Studio** 4.0+
- ✅ **Ardour** 6.0+
- ⚠️ **Reaper** (Linux version, limited testing)

## Troubleshooting

### Build Issues

```bash
# Missing dependencies
sudo apt-get install build-essential cmake pkg-config

# JUCE not found
rm -rf JUCE && git clone https://github.com/juce-framework/JUCE.git

# LibTorch issues
export LD_LIBRARY_PATH=/usr/local/libtorch/lib:$LD_LIBRARY_PATH
```

### Runtime Issues

```bash
# Plugin not loading
ls ~/.vst3/StemMixer.vst3  # Check if installed
ldd ~/.vst3/StemMixer.vst3/Contents/x86_64-linux/StemMixer.so  # Check dependencies

# Model not found
# Ensure htdemucs_traced.pt is in the correct location
```

### Performance Issues

- **High CPU**: Reduce buffer size in DAW, close other applications
- **Audio dropouts**: Increase audio buffer size in DAW settings
- **Long startup**: Model loading takes time on first use

## Development

### Project Structure

```
StemMixer/
├── CMakeLists.txt          # Build configuration
├── Source/                 # Plugin source code
│   ├── PluginProcessor.h/cpp    # Main audio processor
│   ├── PluginEditor.h/cpp       # User interface
│   └── StemSeparator.h/cpp      # AI model integration
├── Assets/                 # Model files
│   └── htdemucs_traced.pt       # Demucs model (not included)
├── setup.sh               # Environment setup
├── build.sh               # Build script
└── README.md              # This file
```

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is open source. See LICENSE file for details.

**Note**: This project does not include the Demucs model. The model is subject to its own license terms from Facebook Research.

## Acknowledgments

- **Demucs**: Facebook Research for the stem separation model
- **JUCE**: Raw Material Software for the audio framework
- **PyTorch**: Meta for the machine learning framework
- **Community**: All contributors and testers

## Support

For issues, questions, or contributions:

- 🐛 **Bug Reports**: Open an issue on GitHub
- 💡 **Feature Requests**: Open an issue with the enhancement label
- 🤝 **Contributions**: Fork and submit a pull request
- 📧 **Contact**: [Your contact information]

---

**Happy music making! 🎵**
