package com.pipewire.controller.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.pipewire.controller.data.DeviceType
import com.pipewire.controller.network.PipeWireSocketService
import com.pipewire.controller.ui.components.AudioControlCard
import com.pipewire.controller.viewmodel.AudioControlViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AudioControlScreen(
    viewModel: AudioControlViewModel = viewModel()
) {
    val connectionState by viewModel.connectionState.collectAsState()
    val audioInfo by viewModel.audioInfo.collectAsState()
    val errorMessage by viewModel.errorMessage.collectAsState()
    val selectedTab by viewModel.selectedTab.collectAsState()
    val serverUrl by viewModel.serverUrl.collectAsState()
    val isConnecting by viewModel.isConnecting.collectAsState()
    
    val sinks by viewModel.sinks.collectAsState(initial = emptyList())
    val sources by viewModel.sources.collectAsState(initial = emptyList())
    val applications by viewModel.applications.collectAsState(initial = emptyList())
    val recordings by viewModel.recordings.collectAsState(initial = emptyList())
    
    var showConnectionDialog by remember { mutableStateOf(false) }
    var urlInput by remember { mutableStateOf(serverUrl) }
    
    // Show error message as snackbar
    errorMessage?.let { error ->
        LaunchedEffect(error) {
            // Auto-clear error after 5 seconds
            kotlinx.coroutines.delay(5000)
            viewModel.clearError()
        }
    }
    
    Column(
        modifier = Modifier.fillMaxSize()
    ) {
        // Top App Bar
        TopAppBar(
            title = { 
                Text("PipeWire Controller")
            },
            actions = {
                IconButton(
                    onClick = { 
                        if (connectionState == PipeWireSocketService.ConnectionState.CONNECTED) {
                            viewModel.refreshAudioInfo()
                        }
                    },
                    enabled = connectionState == PipeWireSocketService.ConnectionState.CONNECTED
                ) {
                    Icon(
                        imageVector = Icons.Default.Refresh,
                        contentDescription = "Refresh"
                    )
                }
                
                IconButton(
                    onClick = { showConnectionDialog = true }
                ) {
                    Icon(
                        imageVector = when (connectionState) {
                            PipeWireSocketService.ConnectionState.CONNECTED -> Icons.Default.Wifi
                            PipeWireSocketService.ConnectionState.CONNECTING -> Icons.Default.WifiOff
                            PipeWireSocketService.ConnectionState.DISCONNECTED -> Icons.Default.WifiOff
                        },
                        contentDescription = "Connection",
                        tint = when (connectionState) {
                            PipeWireSocketService.ConnectionState.CONNECTED -> MaterialTheme.colorScheme.primary
                            PipeWireSocketService.ConnectionState.CONNECTING -> MaterialTheme.colorScheme.onSurfaceVariant
                            PipeWireSocketService.ConnectionState.DISCONNECTED -> MaterialTheme.colorScheme.error
                        }
                    )
                }
            }
        )
        
        // Connection status
        if (connectionState != PipeWireSocketService.ConnectionState.CONNECTED) {
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 8.dp),
                colors = CardDefaults.cardColors(
                    containerColor = when (connectionState) {
                        PipeWireSocketService.ConnectionState.CONNECTING -> MaterialTheme.colorScheme.primaryContainer
                        PipeWireSocketService.ConnectionState.DISCONNECTED -> MaterialTheme.colorScheme.errorContainer
                        else -> MaterialTheme.colorScheme.surfaceVariant
                    }
                )
            ) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    if (connectionState == PipeWireSocketService.ConnectionState.CONNECTING || isConnecting) {
                        CircularProgressIndicator(
                            modifier = Modifier.size(20.dp),
                            strokeWidth = 2.dp
                        )
                    } else {
                        Icon(
                            imageVector = Icons.Default.WifiOff,
                            contentDescription = null,
                            tint = MaterialTheme.colorScheme.onErrorContainer
                        )
                    }
                    
                    Spacer(modifier = Modifier.width(12.dp))
                    
                    Text(
                        text = when {
                            isConnecting || connectionState == PipeWireSocketService.ConnectionState.CONNECTING -> "Connecting to server..."
                            connectionState == PipeWireSocketService.ConnectionState.DISCONNECTED -> "Disconnected - Tap to connect"
                            else -> "Unknown state"
                        },
                        style = MaterialTheme.typography.bodyMedium
                    )
                }
            }
        }
        
        // Error message
        errorMessage?.let { error ->
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 4.dp),
                colors = CardDefaults.cardColors(
                    containerColor = MaterialTheme.colorScheme.errorContainer
                )
            ) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Icon(
                        imageVector = Icons.Default.Error,
                        contentDescription = null,
                        tint = MaterialTheme.colorScheme.onErrorContainer
                    )
                    Spacer(modifier = Modifier.width(12.dp))
                    Text(
                        text = error,
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onErrorContainer,
                        modifier = Modifier.weight(1f)
                    )
                    IconButton(
                        onClick = { viewModel.clearError() }
                    ) {
                        Icon(
                            imageVector = Icons.Default.Close,
                            contentDescription = "Dismiss",
                            tint = MaterialTheme.colorScheme.onErrorContainer
                        )
                    }
                }
            }
        }
        
        if (connectionState == PipeWireSocketService.ConnectionState.CONNECTED) {
            // Tab row
            TabRow(
                selectedTabIndex = selectedTab
            ) {
                Tab(
                    selected = selectedTab == 0,
                    onClick = { viewModel.setSelectedTab(0) },
                    text = { Text("Output (${sinks.size})") },
                    icon = { Icon(Icons.Default.VolumeUp, contentDescription = null) }
                )
                Tab(
                    selected = selectedTab == 1,
                    onClick = { viewModel.setSelectedTab(1) },
                    text = { Text("Input (${sources.size})") },
                    icon = { Icon(Icons.Default.Mic, contentDescription = null) }
                )
                Tab(
                    selected = selectedTab == 2,
                    onClick = { viewModel.setSelectedTab(2) },
                    text = { Text("Apps (${applications.size})") },
                    icon = { Icon(Icons.Default.Apps, contentDescription = null) }
                )
                Tab(
                    selected = selectedTab == 3,
                    onClick = { viewModel.setSelectedTab(3) },
                    text = { Text("Recording (${recordings.size})") },
                    icon = { Icon(Icons.Default.RecordVoiceOver, contentDescription = null) }
                )
            }
            
            // Content
            LazyColumn(
                modifier = Modifier.fillMaxSize(),
                contentPadding = PaddingValues(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                when (selectedTab) {
                    0 -> {
                        if (sinks.isEmpty()) {
                            item {
                                EmptyStateCard("No output devices found")
                            }
                        } else {
                            items(sinks) { sink ->
                                AudioControlCard(
                                    title = sink.description,
                                    subtitle = sink.name,
                                    volume = sink.volume,
                                    muted = sink.muted,
                                    channels = sink.channels,
                                    channelVolumes = sink.channelVolumes,
                                    deviceType = DeviceType.SINK,
                                    deviceId = sink.id,
                                    onVolumeChange = viewModel::setVolume,
                                    onMuteToggle = viewModel::toggleMute
                                )
                            }
                        }
                    }
                    1 -> {
                        if (sources.isEmpty()) {
                            item {
                                EmptyStateCard("No input devices found")
                            }
                        } else {
                            items(sources) { source ->
                                AudioControlCard(
                                    title = source.description,
                                    subtitle = source.name,
                                    volume = source.volume,
                                    muted = source.muted,
                                    channels = source.channels,
                                    channelVolumes = source.channelVolumes,
                                    deviceType = DeviceType.SOURCE,
                                    deviceId = source.id,
                                    onVolumeChange = viewModel::setVolume,
                                    onMuteToggle = viewModel::toggleMute
                                )
                            }
                        }
                    }
                    2 -> {
                        if (applications.isEmpty()) {
                            item {
                                EmptyStateCard("No applications playing audio")
                            }
                        } else {
                            items(applications) { app ->
                                AudioControlCard(
                                    title = app.name,
                                    subtitle = "Application",
                                    volume = app.volume,
                                    muted = app.muted,
                                    channels = app.channels,
                                    channelVolumes = app.channelVolumes,
                                    deviceType = DeviceType.SINK_INPUT,
                                    deviceId = app.id,
                                    onVolumeChange = viewModel::setVolume,
                                    onMuteToggle = viewModel::toggleMute
                                )
                            }
                        }
                    }
                    3 -> {
                        if (recordings.isEmpty()) {
                            item {
                                EmptyStateCard("No applications recording audio")
                            }
                        } else {
                            items(recordings) { recording ->
                                Card(
                                    modifier = Modifier.fillMaxWidth(),
                                    elevation = CardDefaults.cardElevation(defaultElevation = 4.dp),
                                    shape = RoundedCornerShape(12.dp)
                                ) {
                                    Column(
                                        modifier = Modifier
                                            .fillMaxWidth()
                                            .padding(16.dp)
                                    ) {
                                        Text(
                                            text = recording.name,
                                            style = MaterialTheme.typography.titleMedium,
                                            fontWeight = FontWeight.SemiBold
                                        )
                                        Text(
                                            text = "Recording Application",
                                            style = MaterialTheme.typography.bodySmall,
                                            color = MaterialTheme.colorScheme.onSurfaceVariant
                                        )
                                        Spacer(modifier = Modifier.height(8.dp))
                                        Row(
                                            verticalAlignment = Alignment.CenterVertically
                                        ) {
                                            Icon(
                                                imageVector = Icons.Default.RecordVoiceOver,
                                                contentDescription = null,
                                                tint = MaterialTheme.colorScheme.primary
                                            )
                                            Spacer(modifier = Modifier.width(8.dp))
                                            Text(
                                                text = "Volume: ${recording.volume}%",
                                                style = MaterialTheme.typography.bodyMedium
                                            )
                                            if (recording.muted) {
                                                Spacer(modifier = Modifier.width(8.dp))
                                                Text(
                                                    text = "MUTED",
                                                    style = MaterialTheme.typography.bodySmall,
                                                    color = MaterialTheme.colorScheme.error
                                                )
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Connection dialog
    if (showConnectionDialog) {
        AlertDialog(
            onDismissRequest = { showConnectionDialog = false },
            title = { Text("Server Connection") },
            text = {
                Column {
                    Text("Enter the server URL:")
                    Spacer(modifier = Modifier.height(8.dp))
                    OutlinedTextField(
                        value = urlInput,
                        onValueChange = { urlInput = it },
                        label = { Text("Server URL") },
                        placeholder = { Text("ws://192.168.1.100:5000") },
                        modifier = Modifier.fillMaxWidth()
                    )
                }
            },
            confirmButton = {
                TextButton(
                    onClick = {
                        viewModel.setServerUrl(urlInput)
                        if (connectionState == PipeWireSocketService.ConnectionState.CONNECTED) {
                            viewModel.disconnect()
                        } else {
                            viewModel.connect()
                        }
                        showConnectionDialog = false
                    }
                ) {
                    Text(
                        if (connectionState == PipeWireSocketService.ConnectionState.CONNECTED) "Reconnect" else "Connect"
                    )
                }
            },
            dismissButton = {
                TextButton(
                    onClick = { showConnectionDialog = false }
                ) {
                    Text("Cancel")
                }
            }
        )
    }
}

@Composable
private fun EmptyStateCard(message: String) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp),
        shape = RoundedCornerShape(12.dp)
    ) {
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .padding(32.dp),
            contentAlignment = Alignment.Center
        ) {
            Column(
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Icon(
                    imageVector = Icons.Default.Info,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier.size(48.dp)
                )
                Spacer(modifier = Modifier.height(16.dp))
                Text(
                    text = message,
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
    }
}