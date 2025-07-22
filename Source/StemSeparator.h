#pragma once

#include <JuceHeader.h>
#include <torch/torch.h>
#include <torch/script.h>

class StemSeparator
{
public:
    StemSeparator();
    ~StemSeparator();
    
    void prepare(double sampleRate);
    bool isReady() const;
    
    void separateStems(const juce::AudioBuffer<float>& input,
                      juce::AudioBuffer<float>& vocalsOut,
                      juce::AudioBuffer<float>& drumsOut,
                      juce::AudioBuffer<float>& bassOut,
                      juce::AudioBuffer<float>& otherOut);

private:
    torch::jit::script::Module model;
    bool modelLoaded = false;
    double currentSampleRate = 44100.0;
    
    // Helper methods
    torch::Tensor audioBufferToTensor(const juce::AudioBuffer<float>& buffer);
    void tensorToAudioBuffers(const torch::Tensor& tensor,
                             juce::AudioBuffer<float>& vocalsOut,
                             juce::AudioBuffer<float>& drumsOut,
                             juce::AudioBuffer<float>& bassOut,
                             juce::AudioBuffer<float>& otherOut);
    
    juce::String getModelPath();
};