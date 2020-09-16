
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p, juce::AudioProcessorValueTreeState& state)
    : AudioProcessorEditor (&p), audioProcessor (p), params(state)
{
    setSize (600, 400);
    
    //addSlider("thresh", "Threshold", threshSlider, threshLabel, threshAttachment);
    setLookAndFeel(&otherLookAndFeel);
    
    addSlider("time", "Time", timeSlider,timeLabel, timeAttachment);
    addSlider("feedback", "Feedback", feedbackSlider, feedbackLabel, feedbackAttachment);
    addSlider("distortion", "Distortion", distortionSlider, distortionLabel, distortionAttachment);
    addSlider("lpf", "LPF", lpfSlider, lpfLabel, lpfAttachment);
    addSlider("pingpong", "Ping-pong", pingpongSlider, pingpongLabel, pingpongAttachment);
    
    timeSlider.setSliderStyle(juce::Slider::Rotary);
    feedbackSlider.setSliderStyle(juce::Slider::Rotary);
    distortionSlider.setSliderStyle(juce::Slider::Rotary);
    lpfSlider.setSliderStyle(juce::Slider::Rotary);
    pingpongSlider.setSliderStyle(juce::Slider::Rotary);
    
//    timeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
//    feedbackSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
//    distortionSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
//    lpfSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
//    pingpongSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkseagreen);
}

void NewProjectAudioProcessorEditor::resized()
{
    //threshSlider.setBounds(100, 0, 200, 50);
    timeSlider.setBounds(25, 25, 150, 150);
    feedbackSlider.setBounds(225, 25, 150, 150);
    distortionSlider.setBounds(425, 25, 150, 150);
    lpfSlider.setBounds(125, 225, 150, 150);
    pingpongSlider.setBounds(325, 225, 150, 150);
    
}
