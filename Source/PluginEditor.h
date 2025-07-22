#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class StemMixerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    StemMixerAudioProcessorEditor (StemMixerAudioProcessor&);
    ~StemMixerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    StemMixerAudioProcessor& audioProcessor;
    
    // UI Components
    struct StemControl
    {
        juce::Slider gainSlider;
        juce::TextButton muteButton;
        juce::TextButton soloButton;
        juce::Label nameLabel;
        
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloAttachment;
    };
    
    StemControl vocalsControl;
    StemControl drumsControl;
    StemControl bassControl;
    StemControl otherControl;
    
    juce::Label titleLabel;
    
    // Helper methods
    void setupStemControl(StemControl& control, const juce::String& name, 
                         const juce::String& gainParamId,
                         const juce::String& muteParamId,
                         const juce::String& soloParamId);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StemMixerAudioProcessorEditor)
};