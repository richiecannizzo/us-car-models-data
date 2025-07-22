#pragma once

#include <JuceHeader.h>
#include "StemSeparator.h"

//==============================================================================
/**
*/
class StemMixerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    StemMixerAudioProcessor();
    ~StemMixerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter management
    juce::AudioProcessorValueTreeState parameters;
    
    // Stem gains
    std::atomic<float>* vocalsGain;
    std::atomic<float>* drumsGain;
    std::atomic<float>* bassGain;
    std::atomic<float>* otherGain;
    
    // Mute buttons
    std::atomic<float>* vocalsMute;
    std::atomic<float>* drumsMute;
    std::atomic<float>* bassMute;
    std::atomic<float>* otherMute;
    
    // Solo buttons
    std::atomic<float>* vocalsSolo;
    std::atomic<float>* drumsSolo;
    std::atomic<float>* bassSolo;
    std::atomic<float>* otherSolo;

private:
    //==============================================================================
    std::unique_ptr<StemSeparator> stemSeparator;
    
    // Audio buffer for accumulating input chunks
    juce::AudioBuffer<float> inputBuffer;
    int inputBufferPos = 0;
    static constexpr int chunkSize = 44100; // 1 second at 44.1kHz
    
    // Buffers for separated stems
    juce::AudioBuffer<float> vocalsBuffer;
    juce::AudioBuffer<float> drumsBuffer;
    juce::AudioBuffer<float> bassBuffer;
    juce::AudioBuffer<float> otherBuffer;
    
    // Output buffer for processed chunks
    juce::AudioBuffer<float> outputBuffer;
    int outputBufferPos = 0;
    
    double currentSampleRate = 44100.0;
    
    // Helper methods
    void processStemSeparation();
    bool anySoloActive() const;
    bool shouldPlayStem(int stemIndex) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StemMixerAudioProcessor)
};