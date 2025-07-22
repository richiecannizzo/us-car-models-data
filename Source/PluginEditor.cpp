#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StemMixerAudioProcessorEditor::StemMixerAudioProcessorEditor (StemMixerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up title label
    titleLabel.setText("StemMixer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    
    // Set up stem controls
    setupStemControl(vocalsControl, "Vocals", "vocalsGain", "vocalsMute", "vocalsSolo");
    setupStemControl(drumsControl, "Drums", "drumsGain", "drumsMute", "drumsSolo");
    setupStemControl(bassControl, "Bass", "bassGain", "bassMute", "bassSolo");
    setupStemControl(otherControl, "Other", "otherGain", "otherMute", "otherSolo");
    
    // Set colors for different stems
    vocalsControl.nameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff4CAF50)); // Green
    drumsControl.nameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff2196F3));  // Blue
    bassControl.nameLabel.setColour(juce::Label::textColourId, juce::Colour(0xffFF9800));   // Orange
    otherControl.nameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9C27B0));  // Purple
    
    // Set the size of the plugin editor
    setSize (400, 300);
}

StemMixerAudioProcessorEditor::~StemMixerAudioProcessorEditor()
{
}

//==============================================================================
void StemMixerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark background
    g.fillAll (juce::Colour(0xff2a2a2a));
    
    // Background gradient
    juce::ColourGradient gradient(juce::Colour(0xff3a3a3a), 0, 0, 
                                 juce::Colour(0xff1a1a1a), 0, getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Border
    g.setColour(juce::Colour(0xff4a4a4a));
    g.drawRect(getLocalBounds(), 2);
}

void StemMixerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(10, 10);
    
    // Title
    auto titleArea = bounds.removeFromTop(40);
    titleLabel.setBounds(titleArea);
    
    bounds.removeFromTop(10); // Spacing
    
    // Stem controls
    auto stemControlsArea = bounds;
    const int stemWidth = stemControlsArea.getWidth() / 4;
    
    vocalsControl.nameLabel.setBounds(0, stemControlsArea.getY(), stemWidth, 20);
    vocalsControl.gainSlider.setBounds(10, stemControlsArea.getY() + 25, stemWidth - 20, 150);
    vocalsControl.muteButton.setBounds(10, stemControlsArea.getY() + 180, (stemWidth - 30) / 2, 25);
    vocalsControl.soloButton.setBounds(10 + (stemWidth - 30) / 2 + 10, stemControlsArea.getY() + 180, (stemWidth - 30) / 2, 25);
    
    drumsControl.nameLabel.setBounds(stemWidth, stemControlsArea.getY(), stemWidth, 20);
    drumsControl.gainSlider.setBounds(stemWidth + 10, stemControlsArea.getY() + 25, stemWidth - 20, 150);
    drumsControl.muteButton.setBounds(stemWidth + 10, stemControlsArea.getY() + 180, (stemWidth - 30) / 2, 25);
    drumsControl.soloButton.setBounds(stemWidth + 10 + (stemWidth - 30) / 2 + 10, stemControlsArea.getY() + 180, (stemWidth - 30) / 2, 25);
    
    bassControl.nameLabel.setBounds(stemWidth * 2, stemControlsArea.getY(), stemWidth, 20);
    bassControl.gainSlider.setBounds(stemWidth * 2 + 10, stemControlsArea.getY() + 25, stemWidth - 20, 150);
    bassControl.muteButton.setBounds(stemWidth * 2 + 10, stemControlsArea.getY() + 180, (stemWidth - 30) / 2, 25);
    bassControl.soloButton.setBounds(stemWidth * 2 + 10 + (stemWidth - 30) / 2 + 10, stemControlsArea.getY() + 180, (stemWidth - 30) / 2, 25);
    
    otherControl.nameLabel.setBounds(stemWidth * 3, stemControlsArea.getY(), stemWidth, 20);
    otherControl.gainSlider.setBounds(stemWidth * 3 + 10, stemControlsArea.getY() + 25, stemWidth - 20, 150);
    otherControl.muteButton.setBounds(stemWidth * 3 + 10, stemControlsArea.getY() + 180, (stemWidth - 30) / 2, 25);
    otherControl.soloButton.setBounds(stemWidth * 3 + 10 + (stemWidth - 30) / 2 + 10, stemControlsArea.getY() + 180, (stemWidth - 30) / 2, 25);
}

void StemMixerAudioProcessorEditor::setupStemControl(StemControl& control, 
                                                    const juce::String& name,
                                                    const juce::String& gainParamId,
                                                    const juce::String& muteParamId,
                                                    const juce::String& soloParamId)
{
    // Name label
    control.nameLabel.setText(name, juce::dontSendNotification);
    control.nameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    control.nameLabel.setJustificationType(juce::Justification::centred);
    control.nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(control.nameLabel);
    
    // Gain slider
    control.gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    control.gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    control.gainSlider.setRange(0.0, 2.0, 0.01);
    control.gainSlider.setValue(1.0);
    control.gainSlider.setTextValueSuffix("");
    control.gainSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    control.gainSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff404040));
    control.gainSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff202020));
    addAndMakeVisible(control.gainSlider);
    
    // Mute button
    control.muteButton.setButtonText("M");
    control.muteButton.setClickingTogglesState(true);
    control.muteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
    control.muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    control.muteButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    control.muteButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    addAndMakeVisible(control.muteButton);
    
    // Solo button
    control.soloButton.setButtonText("S");
    control.soloButton.setClickingTogglesState(true);
    control.soloButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
    control.soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    control.soloButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    control.soloButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    addAndMakeVisible(control.soloButton);
    
    // Attach to parameters
    control.gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, gainParamId, control.gainSlider);
    control.muteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, muteParamId, control.muteButton);
    control.soloAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, soloParamId, control.soloButton);
}