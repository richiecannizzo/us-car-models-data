package com.pipewire.controller.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.pipewire.controller.data.*
import com.pipewire.controller.network.PipeWireSocketService
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch

class AudioControlViewModel : ViewModel() {
    private val socketService = PipeWireSocketService()
    
    // Server connection
    private val _serverUrl = MutableStateFlow("ws://192.168.1.100:5000")
    val serverUrl: StateFlow<String> = _serverUrl.asStateFlow()
    
    // Connection state
    val connectionState = socketService.connectionState
    val audioInfo = socketService.audioInfo
    val errorMessage = socketService.errorMessage
    
    // UI state
    private val _selectedTab = MutableStateFlow(0)
    val selectedTab: StateFlow<Int> = _selectedTab.asStateFlow()
    
    private val _isConnecting = MutableStateFlow(false)
    val isConnecting: StateFlow<Boolean> = _isConnecting.asStateFlow()
    
    // Derived states for each category
    val sinks = audioInfo.map { it.sinks }
    val sources = audioInfo.map { it.sources }
    val applications = audioInfo.map { it.sinkInputs }
    val recordings = audioInfo.map { it.sourceOutputs }
    
    fun setServerUrl(url: String) {
        _serverUrl.value = url
    }
    
    fun connect() {
        viewModelScope.launch {
            _isConnecting.value = true
            socketService.connect(_serverUrl.value)
            _isConnecting.value = false
        }
    }
    
    fun disconnect() {
        socketService.disconnect()
    }
    
    fun refreshAudioInfo() {
        socketService.requestAudioInfo()
    }
    
    fun setVolume(type: DeviceType, id: Int, volume: Int, channel: Int? = null) {
        socketService.setVolume(type, id, volume, channel)
    }
    
    fun toggleMute(type: DeviceType, id: Int) {
        socketService.toggleMute(type, id)
    }
    
    fun setSelectedTab(index: Int) {
        _selectedTab.value = index
    }
    
    fun clearError() {
        socketService.clearError()
    }
    
    override fun onCleared() {
        super.onCleared()
        socketService.disconnect()
    }
}