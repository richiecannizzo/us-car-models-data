package com.pipewire.controller.ui.components

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.pipewire.controller.data.DeviceType

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AudioControlCard(
    title: String,
    subtitle: String? = null,
    volume: Int,
    muted: Boolean,
    channels: Int,
    channelVolumes: List<Int>,
    deviceType: DeviceType,
    deviceId: Int,
    onVolumeChange: (DeviceType, Int, Int, Int?) -> Unit,
    onMuteToggle: (DeviceType, Int) -> Unit,
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier.fillMaxWidth(),
        elevation = CardDefaults.cardElevation(defaultElevation = 4.dp),
        shape = RoundedCornerShape(12.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp)
        ) {
            // Header
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Column(
                    modifier = Modifier.weight(1f)
                ) {
                    Text(
                        text = title,
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.SemiBold,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis
                    )
                    if (subtitle != null) {
                        Text(
                            text = subtitle,
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                    }
                }
                
                IconButton(
                    onClick = { onMuteToggle(deviceType, deviceId) }
                ) {
                    Icon(
                        imageVector = if (muted) Icons.Default.VolumeOff else Icons.Default.VolumeUp,
                        contentDescription = if (muted) "Unmute" else "Mute",
                        tint = if (muted) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.primary
                    )
                }
            }
            
            Spacer(modifier = Modifier.height(12.dp))
            
            // Master volume control
            VolumeSlider(
                label = "Master",
                volume = volume,
                muted = muted,
                onVolumeChange = { newVolume ->
                    onVolumeChange(deviceType, deviceId, newVolume, null)
                }
            )
            
            // Individual channel controls (if more than 1 channel)
            if (channels > 1 && channelVolumes.isNotEmpty()) {
                Spacer(modifier = Modifier.height(8.dp))
                
                channelVolumes.forEachIndexed { index, channelVolume ->
                    VolumeSlider(
                        label = getChannelName(index, channels),
                        volume = channelVolume,
                        muted = muted,
                        onVolumeChange = { newVolume ->
                            onVolumeChange(deviceType, deviceId, newVolume, index)
                        },
                        modifier = Modifier.padding(start = 16.dp)
                    )
                }
            }
        }
    }
}

@Composable
private fun VolumeSlider(
    label: String,
    volume: Int,
    muted: Boolean,
    onVolumeChange: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    Column(modifier = modifier) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = label,
                style = MaterialTheme.typography.bodyMedium,
                color = if (muted) MaterialTheme.colorScheme.onSurfaceVariant else MaterialTheme.colorScheme.onSurface
            )
            Text(
                text = "${volume}%",
                style = MaterialTheme.typography.bodyMedium,
                color = if (muted) MaterialTheme.colorScheme.onSurfaceVariant else MaterialTheme.colorScheme.onSurface
            )
        }
        
        Slider(
            value = volume.toFloat(),
            onValueChange = { newValue ->
                onVolumeChange(newValue.toInt())
            },
            valueRange = 0f..100f,
            enabled = !muted,
            colors = SliderDefaults.colors(
                thumbColor = if (muted) MaterialTheme.colorScheme.onSurfaceVariant else MaterialTheme.colorScheme.primary,
                activeTrackColor = if (muted) MaterialTheme.colorScheme.onSurfaceVariant else MaterialTheme.colorScheme.primary,
                inactiveTrackColor = MaterialTheme.colorScheme.surfaceVariant
            )
        )
    }
}

private fun getChannelName(index: Int, totalChannels: Int): String {
    return when (totalChannels) {
        2 -> if (index == 0) "Left" else "Right"
        6 -> when (index) {
            0 -> "Front Left"
            1 -> "Front Right"
            2 -> "Front Center"
            3 -> "LFE"
            4 -> "Rear Left"
            5 -> "Rear Right"
            else -> "Channel ${index + 1}"
        }
        8 -> when (index) {
            0 -> "Front Left"
            1 -> "Front Right"
            2 -> "Front Center"
            3 -> "LFE"
            4 -> "Rear Left"
            5 -> "Rear Right"
            6 -> "Side Left"
            7 -> "Side Right"
            else -> "Channel ${index + 1}"
        }
        else -> "Channel ${index + 1}"
    }
}