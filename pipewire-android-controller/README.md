# PipeWire Android Controller

A modern Android app that allows you to remotely control PipeWire audio streams, including volume levels, mute states, and individual channel balances for all audio devices and applications.

## Features

- **Real-time Audio Control**: Control volume and mute for all PipeWire audio streams
- **Individual Channel Control**: Adjust left/right balance and multi-channel audio setups
- **Device Management**: Control output devices, input devices, applications, and recording streams
- **Modern UI**: Beautiful Material 3 design with dark/light theme support
- **Real-time Updates**: Instant synchronization when audio changes occur
- **Network Discovery**: Easy server connection setup

## Architecture

### Server Component (Python)
- **Flask + SocketIO**: WebSocket server for real-time communication
- **PulseCtl Integration**: Interfaces with PipeWire through PulseAudio compatibility layer
- **Real-time Monitoring**: Detects and broadcasts audio changes instantly
- **Multi-client Support**: Multiple Android devices can connect simultaneously

### Android App (Kotlin + Jetpack Compose)
- **Modern UI**: Material 3 design with responsive layouts
- **Real-time Communication**: WebSocket client for instant updates
- **Tabbed Interface**: Organized by device types (Output, Input, Apps, Recording)
- **Individual Controls**: Master volume and per-channel faders
- **Connection Management**: Easy server configuration and status indicators

## Installation

### Server Setup (Linux with PipeWire)

1. **Prerequisites**:
   ```bash
   sudo apt update
   sudo apt install python3 python3-pip python3-venv pipewire pipewire-pulse
   ```

2. **Install Server**:
   ```bash
   cd server/
   chmod +x setup.sh
   ./setup.sh
   ```

3. **Start Server**:
   ```bash
   # As a service (recommended)
   sudo systemctl enable pipewire-controller@$USER
   sudo systemctl start pipewire-controller@$USER
   
   # Or manually
   cd /opt/pipewire-controller
   source venv/bin/activate
   python pipewire_server.py
   ```

4. **Check Status**:
   ```bash
   sudo systemctl status pipewire-controller@$USER
   ```

### Android App Installation

1. **Build from Source**:
   ```bash
   cd android-app/
   ./gradlew assembleDebug
   ```

2. **Install APK**:
   ```bash
   adb install app/build/outputs/apk/debug/app-debug.apk
   ```

## Usage

### Initial Setup

1. **Start the Server**: Ensure the PipeWire server is running on your Linux machine
2. **Find Server IP**: Note your Linux machine's IP address (e.g., `192.168.1.100`)
3. **Open Android App**: Launch "PipeWire Controller" on your Android device
4. **Connect**: Tap the WiFi icon → Enter server URL: `ws://192.168.1.100:5000` → Connect

### Using the App

#### Output Devices Tab
- Control speakers, headphones, and other output devices
- Adjust master volume and individual channel levels
- Mute/unmute devices

#### Input Devices Tab  
- Control microphones and other input devices
- Adjust recording levels and channel balance

#### Applications Tab
- Control volume for individual applications
- Mute specific apps without affecting others
- See which apps are currently playing audio

#### Recording Tab
- Monitor applications that are recording audio
- View recording levels and mute status

### Controls

- **Master Volume Slider**: Controls overall device/app volume
- **Channel Sliders**: Individual left/right or multi-channel control
- **Mute Button**: Toggle mute state (red = muted, blue = unmuted)
- **Refresh Button**: Manually refresh audio information
- **Connection Button**: Manage server connection

## API Reference

### WebSocket Events

#### Client → Server
- `get_audio_info`: Request current audio state
- `set_volume`: Change volume level
  ```json
  {
    "type": "sink|source|sink_input",
    "id": 0,
    "volume": 75,
    "channel": 0  // optional, for specific channel
  }
  ```
- `toggle_mute`: Toggle mute state
  ```json
  {
    "type": "sink|source|sink_input", 
    "id": 0
  }
  ```

#### Server → Client
- `audio_info`: Complete audio state information
- `volume_set_result`: Volume change confirmation
- `mute_toggle_result`: Mute toggle confirmation

### REST Endpoints
- `GET /health`: Server health check

## Configuration

### Server Configuration
Edit `pipewire_server.py` to customize:
- Port number (default: 5000)
- Monitoring interval (default: 1 second)
- CORS settings

### Android App Configuration
The app stores server URLs in Android preferences and supports:
- Custom server URLs
- Connection retry logic
- Automatic reconnection

## Troubleshooting

### Server Issues

**Server won't start**:
```bash
# Check PipeWire status
systemctl --user status pipewire

# Check dependencies
python3 -c "import pulsectl; print('PulseCtl OK')"

# Check permissions
groups $USER  # should include 'audio'
```

**Connection refused**:
```bash
# Check if server is listening
netstat -tulpn | grep :5000

# Check firewall
sudo ufw status
sudo ufw allow 5000/tcp  # if needed
```

### Android App Issues

**Can't connect to server**:
- Verify server IP address and port
- Ensure both devices are on same network
- Check firewall settings on Linux machine
- Try manual server start for debugging

**No audio devices shown**:
- Verify PipeWire is running on Linux
- Check server logs for errors
- Restart PipeWire: `systemctl --user restart pipewire`

**Audio changes not updating**:
- Check WebSocket connection status
- Verify server monitoring is working
- Try refreshing manually

### Debugging

**Server logs**:
```bash
# Service logs
journalctl -u pipewire-controller@$USER -f

# Manual run for detailed output
cd /opt/pipewire-controller
source venv/bin/activate
python pipewire_server.py
```

**Android logs**:
```bash
adb logcat | grep PipeWire
```

## Development

### Building from Source

**Server Development**:
```bash
cd server/
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python pipewire_server.py
```

**Android Development**:
```bash
cd android-app/
./gradlew assembleDebug
# Import into Android Studio for development
```

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## System Requirements

### Server (Linux)
- **OS**: Linux distribution with PipeWire support
- **Audio**: PipeWire 0.3+ with PulseAudio compatibility
- **Python**: Python 3.8+
- **Network**: WiFi/Ethernet connection

### Android App
- **OS**: Android 7.0+ (API level 24+)
- **RAM**: 2GB+ recommended
- **Network**: WiFi connection
- **Storage**: 50MB for app installation

## Security Notes

- Server runs on local network only by default
- No authentication implemented (suitable for home networks)
- For security, use on trusted networks only
- Consider VPN for remote access

## Known Limitations

- Requires PulseAudio compatibility layer for PipeWire
- Network latency may affect real-time responsiveness
- Some advanced PipeWire features not exposed
- Audio format changes may require app restart