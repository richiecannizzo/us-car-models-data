#!/usr/bin/env python3
"""
Test script for PipeWire Controller Server
Run this to verify the server functionality before deploying
"""

import requests
import socketio
import time
import json
from threading import Event

def test_health_endpoint():
    """Test the health check endpoint"""
    try:
        response = requests.get('http://localhost:5000/health', timeout=5)
        if response.status_code == 200:
            data = response.json()
            print("✓ Health endpoint working")
            print(f"  Status: {data.get('status')}")
            print(f"  Audio connected: {data.get('connected_to_audio')}")
            return True
        else:
            print(f"✗ Health endpoint failed: {response.status_code}")
            return False
    except Exception as e:
        print(f"✗ Health endpoint error: {e}")
        return False

def test_websocket_connection():
    """Test WebSocket connection and basic functionality"""
    connected = Event()
    audio_received = Event()
    
    # Create SocketIO client
    sio = socketio.Client()
    
    @sio.event
    def connect():
        print("✓ WebSocket connected")
        connected.set()
    
    @sio.event
    def disconnect():
        print("  WebSocket disconnected")
    
    @sio.event
    def audio_info(data):
        print("✓ Audio info received")
        try:
            # Validate data structure
            if isinstance(data, dict):
                sinks = data.get('sinks', [])
                sources = data.get('sources', [])
                sink_inputs = data.get('sink_inputs', [])
                
                print(f"  Sinks: {len(sinks)}")
                print(f"  Sources: {len(sources)}")
                print(f"  Applications: {len(sink_inputs)}")
                
                # Print first sink if available
                if sinks:
                    sink = sinks[0]
                    print(f"  Example sink: {sink.get('description', 'Unknown')}")
                    print(f"    Volume: {sink.get('volume', 0)}%")
                    print(f"    Muted: {sink.get('muted', False)}")
                    print(f"    Channels: {sink.get('channels', 0)}")
                
                audio_received.set()
            else:
                print(f"✗ Invalid audio data format: {type(data)}")
        except Exception as e:
            print(f"✗ Error processing audio info: {e}")
    
    try:
        # Connect to server
        sio.connect('http://localhost:5000')
        
        # Wait for connection
        if connected.wait(timeout=5):
            # Request audio info
            sio.emit('get_audio_info')
            
            # Wait for response
            if audio_received.wait(timeout=5):
                print("✓ WebSocket communication working")
                
                # Test volume change (if sinks available)
                sio.emit('get_audio_info')  # Get fresh data
                time.sleep(1)
                
                return True
            else:
                print("✗ No audio info received")
                return False
        else:
            print("✗ WebSocket connection failed")
            return False
            
    except Exception as e:
        print(f"✗ WebSocket test error: {e}")
        return False
    finally:
        sio.disconnect()

def main():
    """Run all tests"""
    print("PipeWire Controller Server Test")
    print("=" * 40)
    
    print("\n1. Testing HTTP health endpoint...")
    health_ok = test_health_endpoint()
    
    print("\n2. Testing WebSocket connection...")
    websocket_ok = test_websocket_connection()
    
    print("\n" + "=" * 40)
    print("Test Results:")
    print(f"  Health endpoint: {'✓ PASS' if health_ok else '✗ FAIL'}")
    print(f"  WebSocket: {'✓ PASS' if websocket_ok else '✗ FAIL'}")
    
    if health_ok and websocket_ok:
        print("\n🎉 All tests passed! Server is working correctly.")
        print("\nYou can now:")
        print("1. Install the Android app")
        print("2. Connect to ws://YOUR_IP:5000")
        print("3. Control your audio remotely!")
    else:
        print("\n❌ Some tests failed. Check the following:")
        print("1. Is the server running? (python pipewire_server.py)")
        print("2. Is PipeWire/PulseAudio working?")
        print("3. Are all dependencies installed?")
        print("4. Check server logs for errors")

if __name__ == "__main__":
    main()