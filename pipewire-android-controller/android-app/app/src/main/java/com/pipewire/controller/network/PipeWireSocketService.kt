package com.pipewire.controller.network

import android.util.Log
import com.google.gson.Gson
import com.pipewire.controller.data.*
import io.socket.client.IO
import io.socket.client.Socket
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import org.json.JSONObject
import java.net.URISyntaxException

class PipeWireSocketService {
    private var socket: Socket? = null
    private val gson = Gson()
    
    private val _connectionState = MutableStateFlow(ConnectionState.DISCONNECTED)
    val connectionState: StateFlow<ConnectionState> = _connectionState.asStateFlow()
    
    private val _audioInfo = MutableStateFlow(AudioInfo())
    val audioInfo: StateFlow<AudioInfo> = _audioInfo.asStateFlow()
    
    private val _errorMessage = MutableStateFlow<String?>(null)
    val errorMessage: StateFlow<String?> = _errorMessage.asStateFlow()
    
    fun connect(serverUrl: String) {
        try {
            _connectionState.value = ConnectionState.CONNECTING
            
            val opts = IO.Options()
            opts.reconnection = true
            opts.reconnectionAttempts = 5
            opts.reconnectionDelay = 1000
            
            socket = IO.socket(serverUrl, opts)
            
            socket?.let { socket ->
                socket.on(Socket.EVENT_CONNECT) {
                    Log.d(TAG, "Connected to PipeWire server")
                    _connectionState.value = ConnectionState.CONNECTED
                    _errorMessage.value = null
                }
                
                socket.on(Socket.EVENT_DISCONNECT) {
                    Log.d(TAG, "Disconnected from PipeWire server")
                    _connectionState.value = ConnectionState.DISCONNECTED
                }
                
                socket.on(Socket.EVENT_CONNECT_ERROR) { args ->
                    Log.e(TAG, "Connection error: ${args.contentToString()}")
                    _connectionState.value = ConnectionState.DISCONNECTED
                    _errorMessage.value = "Connection failed: ${args.firstOrNull()}"
                }
                
                socket.on("audio_info") { args ->
                    try {
                        val data = args[0] as JSONObject
                        val audioInfo = gson.fromJson(data.toString(), AudioInfo::class.java)
                        _audioInfo.value = audioInfo
                        Log.d(TAG, "Received audio info: ${audioInfo.sinks.size} sinks, ${audioInfo.sources.size} sources")
                    } catch (e: Exception) {
                        Log.e(TAG, "Error parsing audio info", e)
                        _errorMessage.value = "Error parsing audio data: ${e.message}"
                    }
                }
                
                socket.on("volume_set_result") { args ->
                    try {
                        val data = args[0] as JSONObject
                        val success = data.getBoolean("success")
                        if (!success) {
                            _errorMessage.value = "Failed to set volume"
                        }
                    } catch (e: Exception) {
                        Log.e(TAG, "Error parsing volume set result", e)
                    }
                }
                
                socket.on("mute_toggle_result") { args ->
                    try {
                        val data = args[0] as JSONObject
                        val success = data.getBoolean("success")
                        if (!success) {
                            _errorMessage.value = "Failed to toggle mute"
                        }
                    } catch (e: Exception) {
                        Log.e(TAG, "Error parsing mute toggle result", e)
                    }
                }
                
                socket.connect()
            }
            
        } catch (e: URISyntaxException) {
            Log.e(TAG, "Invalid server URL", e)
            _connectionState.value = ConnectionState.DISCONNECTED
            _errorMessage.value = "Invalid server URL: ${e.message}"
        } catch (e: Exception) {
            Log.e(TAG, "Error connecting to server", e)
            _connectionState.value = ConnectionState.DISCONNECTED
            _errorMessage.value = "Connection error: ${e.message}"
        }
    }
    
    fun disconnect() {
        socket?.disconnect()
        socket = null
        _connectionState.value = ConnectionState.DISCONNECTED
    }
    
    fun requestAudioInfo() {
        socket?.emit("get_audio_info")
    }
    
    fun setVolume(type: DeviceType, id: Int, volume: Int, channel: Int? = null) {
        val request = VolumeChangeRequest(
            type = type.value,
            id = id,
            volume = volume,
            channel = channel
        )
        
        val jsonObject = JSONObject(gson.toJson(request))
        socket?.emit("set_volume", jsonObject)
    }
    
    fun toggleMute(type: DeviceType, id: Int) {
        val request = MuteToggleRequest(
            type = type.value,
            id = id
        )
        
        val jsonObject = JSONObject(gson.toJson(request))
        socket?.emit("toggle_mute", jsonObject)
    }
    
    fun clearError() {
        _errorMessage.value = null
    }
    
    enum class ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    }
    
    companion object {
        private const val TAG = "PipeWireSocket"
    }
}