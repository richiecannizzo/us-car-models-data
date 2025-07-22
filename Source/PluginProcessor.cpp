#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StemMixerAudioProcessor::StemMixerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters(*this, nullptr, juce::Identifier("StemMixer"),
    {
        std::make_unique<juce::AudioParameterFloat>("vocalsGain", "Vocals Gain", 0.0f, 2.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("drumsGain", "Drums Gain", 0.0f, 2.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("bassGain", "Bass Gain", 0.0f, 2.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("otherGain", "Other Gain", 0.0f, 2.0f, 1.0f),
        
        std::make_unique<juce::AudioParameterBool>("vocalsMute", "Vocals Mute", false),
        std::make_unique<juce::AudioParameterBool>("drumsMute", "Drums Mute", false),
        std::make_unique<juce::AudioParameterBool>("bassMute", "Bass Mute", false),
        std::make_unique<juce::AudioParameterBool>("otherMute", "Other Mute", false),
        
        std::make_unique<juce::AudioParameterBool>("vocalsSolo", "Vocals Solo", false),
        std::make_unique<juce::AudioParameterBool>("drumsSolo", "Drums Solo", false),
        std::make_unique<juce::AudioParameterBool>("bassSolo", "Bass Solo", false),
        std::make_unique<juce::AudioParameterBool>("otherSolo", "Other Solo", false)
    })
{
    // Get parameter pointers
    vocalsGain = parameters.getRawParameterValue("vocalsGain");
    drumsGain = parameters.getRawParameterValue("drumsGain");
    bassGain = parameters.getRawParameterValue("bassGain");
    otherGain = parameters.getRawParameterValue("otherGain");
    
    vocalsMute = parameters.getRawParameterValue("vocalsMute");
    drumsMute = parameters.getRawParameterValue("drumsMute");
    bassMute = parameters.getRawParameterValue("bassMute");
    otherMute = parameters.getRawParameterValue("otherMute");
    
    vocalsSolo = parameters.getRawParameterValue("vocalsSolo");
    drumsSolo = parameters.getRawParameterValue("drumsSolo");
    bassSolo = parameters.getRawParameterValue("bassSolo");
    otherSolo = parameters.getRawParameterValue("otherSolo");
    
    // Initialize stem separator
    stemSeparator = std::make_unique<StemSeparator>();
}

StemMixerAudioProcessor::~StemMixerAudioProcessor()
{
}

//==============================================================================
const juce::String StemMixerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StemMixerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StemMixerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StemMixerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StemMixerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StemMixerAudioProcessor::getNumPrograms()
{
    return 1;
}

int StemMixerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StemMixerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StemMixerAudioProcessor::getProgramName (int index)
{
    return {};
}

void StemMixerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StemMixerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    // Initialize buffers
    inputBuffer.setSize(2, chunkSize);
    vocalsBuffer.setSize(2, chunkSize);
    drumsBuffer.setSize(2, chunkSize);
    bassBuffer.setSize(2, chunkSize);
    otherBuffer.setSize(2, chunkSize);
    outputBuffer.setSize(2, chunkSize);
    
    inputBuffer.clear();
    vocalsBuffer.clear();
    drumsBuffer.clear();
    bassBuffer.clear();
    otherBuffer.clear();
    outputBuffer.clear();
    
    inputBufferPos = 0;
    outputBufferPos = 0;
    
    // Initialize stem separator
    if (stemSeparator)
    {
        stemSeparator->prepare(sampleRate);
    }
}

void StemMixerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StemMixerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void StemMixerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int numSamples = buffer.getNumSamples();
    
    // Process in chunks
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        // Copy input to input buffer
        for (int channel = 0; channel < juce::jmin(2, totalNumInputChannels); ++channel)
        {
            inputBuffer.setSample(channel, inputBufferPos, buffer.getSample(channel, sampleIndex));
        }
        
        inputBufferPos++;
        
        // When we have a full chunk, process it
        if (inputBufferPos >= chunkSize)
        {
            processStemSeparation();
            inputBufferPos = 0;
            outputBufferPos = 0;
        }
        
        // Output processed audio
        if (outputBufferPos < chunkSize)
        {
            for (int channel = 0; channel < juce::jmin(2, totalNumOutputChannels); ++channel)
            {
                buffer.setSample(channel, sampleIndex, outputBuffer.getSample(channel, outputBufferPos));
            }
            outputBufferPos++;
        }
        else
        {
            // If no processed audio available, output zeros
            for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            {
                buffer.setSample(channel, sampleIndex, 0.0f);
            }
        }
    }
}

void StemMixerAudioProcessor::processStemSeparation()
{
    if (!stemSeparator || !stemSeparator->isReady())
    {
        // If separator not ready, copy input to output
        outputBuffer.makeCopyOf(inputBuffer);
        return;
    }
    
    // Separate stems
    stemSeparator->separateStems(inputBuffer, vocalsBuffer, drumsBuffer, bassBuffer, otherBuffer);
    
    // Clear output buffer
    outputBuffer.clear();
    
    // Mix stems based on gain, mute, and solo settings
    bool soloActive = anySoloActive();
    
    for (int channel = 0; channel < 2; ++channel)
    {
        for (int sample = 0; sample < chunkSize; ++sample)
        {
            float outputSample = 0.0f;
            
            // Vocals
            if (shouldPlayStem(0))
            {
                outputSample += vocalsBuffer.getSample(channel, sample) * vocalsGain->load();
            }
            
            // Drums
            if (shouldPlayStem(1))
            {
                outputSample += drumsBuffer.getSample(channel, sample) * drumsGain->load();
            }
            
            // Bass
            if (shouldPlayStem(2))
            {
                outputSample += bassBuffer.getSample(channel, sample) * bassGain->load();
            }
            
            // Other
            if (shouldPlayStem(3))
            {
                outputSample += otherBuffer.getSample(channel, sample) * otherGain->load();
            }
            
            outputBuffer.setSample(channel, sample, outputSample);
        }
    }
}

bool StemMixerAudioProcessor::anySoloActive() const
{
    return vocalsSolo->load() > 0.5f || 
           drumsSolo->load() > 0.5f || 
           bassSolo->load() > 0.5f || 
           otherSolo->load() > 0.5f;
}

bool StemMixerAudioProcessor::shouldPlayStem(int stemIndex) const
{
    bool soloActive = anySoloActive();
    
    switch (stemIndex)
    {
        case 0: // Vocals
            if (vocalsMute->load() > 0.5f) return false;
            if (soloActive) return vocalsSolo->load() > 0.5f;
            return true;
            
        case 1: // Drums
            if (drumsMute->load() > 0.5f) return false;
            if (soloActive) return drumsSolo->load() > 0.5f;
            return true;
            
        case 2: // Bass
            if (bassMute->load() > 0.5f) return false;
            if (soloActive) return bassSolo->load() > 0.5f;
            return true;
            
        case 3: // Other
            if (otherMute->load() > 0.5f) return false;
            if (soloActive) return otherSolo->load() > 0.5f;
            return true;
            
        default:
            return false;
    }
}

//==============================================================================
bool StemMixerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* StemMixerAudioProcessor::createEditor()
{
    return new StemMixerAudioProcessorEditor (*this);
}

//==============================================================================
void StemMixerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void StemMixerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StemMixerAudioProcessor();
}