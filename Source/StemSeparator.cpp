#include "StemSeparator.h"

StemSeparator::StemSeparator()
{
}

StemSeparator::~StemSeparator()
{
}

void StemSeparator::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    
    try
    {
        juce::String modelPath = getModelPath();
        
        if (juce::File(modelPath).existsAsFile())
        {
            model = torch::jit::load(modelPath.toStdString());
            model.eval();
            modelLoaded = true;
            
            DBG("StemSeparator: Model loaded successfully from " + modelPath);
        }
        else
        {
            DBG("StemSeparator: Model file not found at " + modelPath);
            modelLoaded = false;
        }
    }
    catch (const std::exception& e)
    {
        DBG("StemSeparator: Failed to load model: " + juce::String(e.what()));
        modelLoaded = false;
    }
}

bool StemSeparator::isReady() const
{
    return modelLoaded;
}

void StemSeparator::separateStems(const juce::AudioBuffer<float>& input,
                                 juce::AudioBuffer<float>& vocalsOut,
                                 juce::AudioBuffer<float>& drumsOut,
                                 juce::AudioBuffer<float>& bassOut,
                                 juce::AudioBuffer<float>& otherOut)
{
    if (!modelLoaded)
    {
        // If model not loaded, return silence for all stems
        vocalsOut.clear();
        drumsOut.clear();
        bassOut.clear();
        otherOut.clear();
        return;
    }
    
    try
    {
        // Convert audio buffer to tensor
        torch::Tensor inputTensor = audioBufferToTensor(input);
        
        // Perform inference
        std::vector<torch::jit::IValue> inputs;
        inputs.push_back(inputTensor);
        
        torch::Tensor outputTensor = model.forward(inputs).toTensor();
        
        // Convert output tensor back to audio buffers
        tensorToAudioBuffers(outputTensor, vocalsOut, drumsOut, bassOut, otherOut);
    }
    catch (const std::exception& e)
    {
        DBG("StemSeparator: Inference failed: " + juce::String(e.what()));
        
        // On error, return silence for all stems
        vocalsOut.clear();
        drumsOut.clear();
        bassOut.clear();
        otherOut.clear();
    }
}

torch::Tensor StemSeparator::audioBufferToTensor(const juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Create tensor with shape [1, 2, numSamples]
    torch::Tensor tensor = torch::zeros({1, 2, numSamples}, torch::kFloat32);
    
    auto accessor = tensor.accessor<float, 3>();
    
    for (int channel = 0; channel < juce::jmin(2, numChannels); ++channel)
    {
        const float* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            accessor[0][channel][sample] = channelData[sample];
        }
    }
    
    // If input is mono, duplicate to stereo
    if (numChannels == 1)
    {
        const float* channelData = buffer.getReadPointer(0);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            accessor[0][1][sample] = channelData[sample];
        }
    }
    
    return tensor;
}

void StemSeparator::tensorToAudioBuffers(const torch::Tensor& tensor,
                                        juce::AudioBuffer<float>& vocalsOut,
                                        juce::AudioBuffer<float>& drumsOut,
                                        juce::AudioBuffer<float>& bassOut,
                                        juce::AudioBuffer<float>& otherOut)
{
    // Expected tensor shape: [1, 4, 2, numSamples]
    // 4 stems: vocals, drums, bass, other
    
    if (tensor.dim() != 4 || tensor.size(0) != 1 || tensor.size(1) != 4 || tensor.size(2) != 2)
    {
        DBG("StemSeparator: Unexpected tensor shape");
        vocalsOut.clear();
        drumsOut.clear();
        bassOut.clear();
        otherOut.clear();
        return;
    }
    
    const int numSamples = tensor.size(3);
    
    // Ensure output buffers are correct size
    vocalsOut.setSize(2, numSamples);
    drumsOut.setSize(2, numSamples);
    bassOut.setSize(2, numSamples);
    otherOut.setSize(2, numSamples);
    
    auto accessor = tensor.accessor<float, 4>();
    
    // Copy stems to output buffers
    for (int channel = 0; channel < 2; ++channel)
    {
        float* vocalsData = vocalsOut.getWritePointer(channel);
        float* drumsData = drumsOut.getWritePointer(channel);
        float* bassData = bassOut.getWritePointer(channel);
        float* otherData = otherOut.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            vocalsData[sample] = accessor[0][0][channel][sample]; // Vocals
            drumsData[sample] = accessor[0][1][channel][sample];  // Drums
            bassData[sample] = accessor[0][2][channel][sample];   // Bass
            otherData[sample] = accessor[0][3][channel][sample];  // Other
        }
    }
}

juce::String StemSeparator::getModelPath()
{
    // Try different possible locations for the model file
    std::vector<juce::String> possiblePaths = {
        "htdemucs_traced.pt",
        "./htdemucs_traced.pt",
        "../htdemucs_traced.pt",
        "Assets/htdemucs_traced.pt",
        "../Assets/htdemucs_traced.pt"
    };
    
    for (const auto& path : possiblePaths)
    {
        if (juce::File(path).existsAsFile())
        {
            return path;
        }
    }
    
    // Default fallback
    return "htdemucs_traced.pt";
}