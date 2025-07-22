package com.pipewire.controller.data

import com.google.gson.annotations.SerializedName

data class AudioInfo(
    val sinks: List<AudioSink> = emptyList(),
    val sources: List<AudioSource> = emptyList(),
    @SerializedName("sink_inputs")
    val sinkInputs: List<SinkInput> = emptyList(),
    @SerializedName("source_outputs")
    val sourceOutputs: List<SourceOutput> = emptyList(),
    val error: String? = null
)

data class AudioSink(
    val id: Int,
    val name: String,
    val description: String,
    val volume: Int,
    val muted: Boolean,
    val channels: Int,
    @SerializedName("channel_volumes")
    val channelVolumes: List<Int>
)

data class AudioSource(
    val id: Int,
    val name: String,
    val description: String,
    val volume: Int,
    val muted: Boolean,
    val channels: Int,
    @SerializedName("channel_volumes")
    val channelVolumes: List<Int>
)

data class SinkInput(
    val id: Int,
    val name: String,
    @SerializedName("sink_id")
    val sinkId: Int,
    val volume: Int,
    val muted: Boolean,
    val channels: Int,
    @SerializedName("channel_volumes")
    val channelVolumes: List<Int>
)

data class SourceOutput(
    val id: Int,
    val name: String,
    @SerializedName("source_id")
    val sourceId: Int,
    val volume: Int,
    val muted: Boolean
)

data class VolumeChangeRequest(
    val type: String, // "sink", "source", "sink_input"
    val id: Int,
    val volume: Int,
    val channel: Int? = null
)

data class MuteToggleRequest(
    val type: String, // "sink", "source", "sink_input"
    val id: Int
)

enum class DeviceType(val value: String) {
    SINK("sink"),
    SOURCE("source"),
    SINK_INPUT("sink_input"),
    SOURCE_OUTPUT("source_output")
}