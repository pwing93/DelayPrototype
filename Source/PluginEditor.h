
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class OtherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OtherLookAndFeel()
    {
        setColour (juce::Slider::thumbColourId, juce::Colours::red);
    };
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
    {
        auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        //fill
        g.setColour (juce::Colours::linen);
        g.fillEllipse (rx, ry, rw, rw);

        //outline
        g.setColour (juce::Colours::coral);
        g.drawEllipse (rx, ry, rw, rw, 3.0f);

        juce::Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 10.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));

        //pointer
        g.setColour (juce::Colours::coral);
        g.fillPath (p);

    }
};

class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~NewProjectAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& params;
    juce::Slider timeSlider, feedbackSlider, distortionSlider, lpfSlider, pingpongSlider;
    juce::Label timeLabel, feedbackLabel, distortionLabel, lpfLabel, pingpongLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeAttachment, feedbackAttachment, distortionAttachment, lpfAttachment, pingpongAttachment;
    
    OtherLookAndFeel otherLookAndFeel;
    
    void addSlider(juce::String name, juce::String labelText, juce::Slider& slider, juce::Label& label, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment)
    {
        addAndMakeVisible(slider);
        attachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment (params, name, slider));
        label.setText(labelText, juce::dontSendNotification);
        label.attachToComponent(&slider, false);
        label.setFont(25);
        label.juce::Label::setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
        slider.setPopupDisplayEnabled(true, true, this);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};


