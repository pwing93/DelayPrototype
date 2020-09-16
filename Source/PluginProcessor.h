
#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

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
    
    void fillDelayBuffer (int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData);
    void getFromDelayBuffer (juce::AudioBuffer<float>& buffer, int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData);
    void feedbackDelay (int channel, const int bufferLength, const int delayBufferLength, float* dryBuffer);
    
    
    
    
    //***** My functions for LoFi elements starts here:
    void distortSample (float* channelData, const int bufferLength);
    
    //LPF element functions
    void updateFilter();
    void updateParameters();
    
    //Autopanner element for pingpong effect
    void pingPong(juce::AudioBuffer<float>& buffer);

private:
    
    //SVT Filter object from JUCE DSP module
    juce::dsp::ProcessorDuplicator<juce::dsp::StateVariableFilter::Filter<float>, juce::dsp::StateVariableFilter::Parameters<float>> stateVariableFilter;
    
    juce::AudioProcessorValueTreeState state;
    std::atomic<float>* feedbackParam, *distorionParam, *timeParam, *lpfParam, *pingpongParam;
    //std::atomic<int>* timeParam, *lpfParam;
    //std::atomic<bool>* pingpongParam;
    
    juce::AudioBuffer<float> mDelayBuffer;
    int mWritePosition {0};
    int mSampleRate {48000};
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};
