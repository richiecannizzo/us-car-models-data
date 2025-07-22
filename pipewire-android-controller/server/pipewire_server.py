#!/usr/bin/env python3
"""
PipeWire Audio Control Server
Provides WebSocket API for controlling PipeWire audio streams remotely
"""

import json
import logging
from flask import Flask, request
from flask_socketio import SocketIO, emit
from flask_cors import CORS
import pulsectl
import threading
import time
from typing import Dict, List, Optional

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = Flask(__name__)
app.config['SECRET_KEY'] = 'pipewire-controller-secret'
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='eventlet')

class PipeWireController:
    def __init__(self):
        self.pulse = None
        self.clients = set()
        self.monitoring = False
        self.connect_to_pulse()
        
    def connect_to_pulse(self):
        """Connect to PulseAudio/PipeWire server"""
        try:
            self.pulse = pulsectl.Pulse('pipewire-controller')
            logger.info("Connected to PulseAudio/PipeWire server")
        except Exception as e:
            logger.error(f"Failed to connect to PulseAudio/PipeWire: {e}")
            self.pulse = None
    
    def get_audio_info(self) -> Dict:
        """Get current audio streams, sinks, and sources information"""
        if not self.pulse:
            return {"error": "Not connected to audio server"}
        
        try:
            # Get all sinks (output devices)
            sinks = []
            for sink in self.pulse.sink_list():
                sinks.append({
                    "id": sink.index,
                    "name": sink.name,
                    "description": sink.description,
                    "volume": self._volume_to_percent(sink.volume),
                    "muted": bool(sink.mute),
                    "channels": len(sink.volume.values),
                    "channel_volumes": [self._volume_to_percent([vol]) for vol in sink.volume.values]
                })
            
            # Get all sources (input devices)
            sources = []
            for source in self.pulse.source_list():
                if not source.name.endswith('.monitor'):  # Skip monitor sources
                    sources.append({
                        "id": source.index,
                        "name": source.name,
                        "description": source.description,
                        "volume": self._volume_to_percent(source.volume),
                        "muted": bool(source.mute),
                        "channels": len(source.volume.values),
                        "channel_volumes": [self._volume_to_percent([vol]) for vol in source.volume.values]
                    })
            
            # Get all sink inputs (applications playing audio)
            sink_inputs = []
            for si in self.pulse.sink_input_list():
                app_name = si.proplist.get('application.name', 'Unknown')
                sink_inputs.append({
                    "id": si.index,
                    "name": app_name,
                    "sink_id": si.sink,
                    "volume": self._volume_to_percent(si.volume),
                    "muted": bool(si.mute),
                    "channels": len(si.volume.values),
                    "channel_volumes": [self._volume_to_percent([vol]) for vol in si.volume.values]
                })
            
            # Get all source outputs (applications recording audio)
            source_outputs = []
            for so in self.pulse.source_output_list():
                app_name = so.proplist.get('application.name', 'Unknown')
                source_outputs.append({
                    "id": so.index,
                    "name": app_name,
                    "source_id": so.source,
                    "volume": self._volume_to_percent(so.volume) if hasattr(so, 'volume') else 100,
                    "muted": bool(so.mute) if hasattr(so, 'mute') else False
                })
            
            return {
                "sinks": sinks,
                "sources": sources,
                "sink_inputs": sink_inputs,
                "source_outputs": source_outputs
            }
            
        except Exception as e:
            logger.error(f"Error getting audio info: {e}")
            return {"error": str(e)}
    
    def _volume_to_percent(self, volume) -> int:
        """Convert PulseAudio volume to percentage"""
        if hasattr(volume, 'values'):
            # Average of all channels
            return int((sum(volume.values) / len(volume.values)) * 100 / 0x10000)
        elif isinstance(volume, list) and len(volume) > 0:
            return int(volume[0] * 100 / 0x10000)
        return 0
    
    def _percent_to_volume(self, percent: int):
        """Convert percentage to PulseAudio volume"""
        return int(percent * 0x10000 / 100)
    
    def set_sink_volume(self, sink_id: int, volume: int, channel: Optional[int] = None) -> bool:
        """Set volume for a sink (output device)"""
        if not self.pulse:
            return False
        
        try:
            sink = self.pulse.get_sink_by_index(sink_id)
            if channel is not None:
                # Set specific channel volume
                new_volume = sink.volume
                new_volume.values[channel] = self._percent_to_volume(volume)
            else:
                # Set all channels to same volume
                new_volume = pulsectl.PulseVolumeInfo([self._percent_to_volume(volume)] * len(sink.volume.values))
            
            self.pulse.volume_set(sink, new_volume)
            return True
        except Exception as e:
            logger.error(f"Error setting sink volume: {e}")
            return False
    
    def set_source_volume(self, source_id: int, volume: int, channel: Optional[int] = None) -> bool:
        """Set volume for a source (input device)"""
        if not self.pulse:
            return False
        
        try:
            source = self.pulse.get_source_by_index(source_id)
            if channel is not None:
                # Set specific channel volume
                new_volume = source.volume
                new_volume.values[channel] = self._percent_to_volume(volume)
            else:
                # Set all channels to same volume
                new_volume = pulsectl.PulseVolumeInfo([self._percent_to_volume(volume)] * len(source.volume.values))
            
            self.pulse.volume_set(source, new_volume)
            return True
        except Exception as e:
            logger.error(f"Error setting source volume: {e}")
            return False
    
    def set_sink_input_volume(self, sink_input_id: int, volume: int, channel: Optional[int] = None) -> bool:
        """Set volume for a sink input (application)"""
        if not self.pulse:
            return False
        
        try:
            sink_input = self.pulse.get_sink_input_by_index(sink_input_id)
            if channel is not None:
                # Set specific channel volume
                new_volume = sink_input.volume
                new_volume.values[channel] = self._percent_to_volume(volume)
            else:
                # Set all channels to same volume
                new_volume = pulsectl.PulseVolumeInfo([self._percent_to_volume(volume)] * len(sink_input.volume.values))
            
            self.pulse.volume_set(sink_input, new_volume)
            return True
        except Exception as e:
            logger.error(f"Error setting sink input volume: {e}")
            return False
    
    def toggle_mute(self, device_type: str, device_id: int) -> bool:
        """Toggle mute for a device or application"""
        if not self.pulse:
            return False
        
        try:
            if device_type == "sink":
                device = self.pulse.get_sink_by_index(device_id)
            elif device_type == "source":
                device = self.pulse.get_source_by_index(device_id)
            elif device_type == "sink_input":
                device = self.pulse.get_sink_input_by_index(device_id)
            else:
                return False
            
            self.pulse.mute(device, not device.mute)
            return True
        except Exception as e:
            logger.error(f"Error toggling mute: {e}")
            return False

# Initialize controller
controller = PipeWireController()

@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    logger.info(f"Client connected: {request.sid}")
    controller.clients.add(request.sid)
    # Send initial audio info
    emit('audio_info', controller.get_audio_info())

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    logger.info(f"Client disconnected: {request.sid}")
    controller.clients.discard(request.sid)

@socketio.on('get_audio_info')
def handle_get_audio_info():
    """Send current audio information to client"""
    emit('audio_info', controller.get_audio_info())

@socketio.on('set_volume')
def handle_set_volume(data):
    """Handle volume change request"""
    device_type = data.get('type')
    device_id = data.get('id')
    volume = data.get('volume')
    channel = data.get('channel')
    
    success = False
    if device_type == 'sink':
        success = controller.set_sink_volume(device_id, volume, channel)
    elif device_type == 'source':
        success = controller.set_source_volume(device_id, volume, channel)
    elif device_type == 'sink_input':
        success = controller.set_sink_input_volume(device_id, volume, channel)
    
    if success:
        # Broadcast updated info to all clients
        socketio.emit('audio_info', controller.get_audio_info())
    
    emit('volume_set_result', {'success': success})

@socketio.on('toggle_mute')
def handle_toggle_mute(data):
    """Handle mute toggle request"""
    device_type = data.get('type')
    device_id = data.get('id')
    
    success = controller.toggle_mute(device_type, device_id)
    
    if success:
        # Broadcast updated info to all clients
        socketio.emit('audio_info', controller.get_audio_info())
    
    emit('mute_toggle_result', {'success': success})

def monitor_audio_changes():
    """Monitor for audio changes and broadcast updates"""
    last_info = {}
    while controller.monitoring:
        try:
            current_info = controller.get_audio_info()
            if current_info != last_info:
                socketio.emit('audio_info', current_info)
                last_info = current_info
        except Exception as e:
            logger.error(f"Error in monitoring: {e}")
        time.sleep(1)  # Check every second

@app.route('/health')
def health_check():
    """Health check endpoint"""
    return {'status': 'healthy', 'connected_to_audio': controller.pulse is not None}

if __name__ == '__main__':
    # Start monitoring in background
    controller.monitoring = True
    monitor_thread = threading.Thread(target=monitor_audio_changes, daemon=True)
    monitor_thread.start()
    
    logger.info("Starting PipeWire Control Server on port 5000")
    socketio.run(app, host='0.0.0.0', port=5000, debug=False)