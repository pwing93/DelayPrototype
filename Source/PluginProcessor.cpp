
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
,
state(*this, nullptr, juce::Identifier("params"),
{
    std::make_unique<juce::AudioParameterInt>("time", "Time", 0, 1000, 0),
    
    std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f),
    
    std::make_unique<juce::AudioParameterBool>("pingpong", "Ping-Pong", false),
    
    std::make_unique<juce::AudioParameterFloat>("distortion", "Distortion", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f),
    
    std::make_unique<juce::AudioParameterInt>("lpf", "LPF", 500, 20000, 20000)
}
)
#endif
{
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int numInputChannels = getTotalNumInputChannels();
    const int delayBufferSize = 2 * (sampleRate + samplesPerBlock);
    mSampleRate = sampleRate;
    
    mDelayBuffer.setSize(numInputChannels, delayBufferSize);
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = mSampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getMainBusNumOutputChannels();
    
    timeParam = state.getRawParameterValue("time");
    feedbackParam = state.getRawParameterValue("feedback");
    distorionParam = state.getRawParameterValue("distortion");
    lpfParam = state.getRawParameterValue("lpf");
    pingpongParam = state.getRawParameterValue("pingpong");
    
    stateVariableFilter.reset();
    updateFilter();
    stateVariableFilter.prepare(spec);
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int bufferLength = buffer.getNumSamples();
    const int delayBufferLength = mDelayBuffer.getNumSamples();
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        
        const float* bufferData = buffer.getReadPointer(channel);
        const float* delayBufferData = mDelayBuffer.getReadPointer(channel);
        float* dryBuffer = buffer.getWritePointer(channel);
        
        //** My parameters for LOFI elements etc.
        auto* channelData = buffer.getWritePointer(channel);
        
        juce::dsp::AudioBlock<float> block (buffer);
        updateFilter();
        
        distortSample(channelData, bufferLength);
        stateVariableFilter.process(juce::dsp::ProcessContextReplacing<float> (block));
        fillDelayBuffer(channel, bufferLength, delayBufferLength, bufferData, delayBufferData);
        getFromDelayBuffer(buffer, channel, bufferLength, delayBufferLength, bufferData, delayBufferData);
        feedbackDelay(channel, bufferLength, delayBufferLength, dryBuffer);
        pingPong(buffer);
        
    }
    
    mWritePosition += bufferLength;
    mWritePosition %= delayBufferLength;
}

void NewProjectAudioProcessor::distortSample(float* channelData, const int bufferLength)
{
    for (int i = 0; i < bufferLength; i++)
    {
        auto input = channelData[i] * 10;
        auto cleanOut = channelData[i];
        float distortion = *distorionParam;
        
        float mix = distortion;
        float output = (1.0 - (distortion / 2.5));
        
        //Hard Clipping Algorithm
        if (input > 0.01)
        {
            input = 0.01;
        }
            
        else if (input < -0.01)
        {
            input = -0.01;
        }
            
        else
        {
            input = input;
        }
        //Formula for taking "Wet" affected Input signal and mixing back with "Dry" unaffected signal
        channelData[i] = ((1 - mix) * cleanOut) + (mix * input);
        
        //Output Make-up Gain stage
        channelData[i] = channelData[i] * output;
    }
}

void NewProjectAudioProcessor::updateFilter()
{
    stateVariableFilter.state->type = juce::dsp::StateVariableFilter::Parameters<float>::Type::lowPass;
    stateVariableFilter.state->setCutOffFrequency (mSampleRate, *lpfParam, 1.0f);
}

void NewProjectAudioProcessor::fillDelayBuffer (int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData)
{
    const float gain = 0.3;
    
     //copy the data from main buffer to delay buffer
     if (delayBufferLength > bufferLength + mWritePosition)
     {
         mDelayBuffer.copyFromWithRamp(channel, mWritePosition, bufferData, bufferLength, gain, gain);
     }
     
     else
     {
         const int bufferRemaining = delayBufferLength - mWritePosition;
         
         mDelayBuffer.copyFromWithRamp(channel, mWritePosition, bufferData, bufferRemaining, gain, gain);
         mDelayBuffer.copyFromWithRamp(channel, 0, bufferData, bufferLength - bufferRemaining, gain, gain);
     }
}

void NewProjectAudioProcessor::getFromDelayBuffer (juce::AudioBuffer<float>& buffer, int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData)
{
    float delayTime = *timeParam;
    const int readPosition = static_cast<int> (delayBufferLength + mWritePosition - (mSampleRate * delayTime / 1000)) % delayBufferLength;
    
    if (delayBufferLength > bufferLength + readPosition)
    {
        buffer.copyFrom(channel, 0, delayBufferData + readPosition, bufferLength);
    }
    
    else
    {
        const int bufferRemaining = delayBufferLength - readPosition;
        buffer.copyFrom(channel, 0, delayBufferData + readPosition, bufferRemaining);
        buffer.copyFrom(channel, bufferRemaining, delayBufferData, bufferLength - bufferRemaining);
    }
}

void NewProjectAudioProcessor::feedbackDelay (int channel, const int bufferLength, const int delayBufferLength, float* dryBuffer)
{
    float feedBackGain = *feedbackParam;
    if (delayBufferLength > bufferLength + mWritePosition)
    {
        mDelayBuffer.addFromWithRamp(channel, mWritePosition, dryBuffer, bufferLength, feedBackGain, feedBackGain);
    }
    
    else
    {
        const int bufferRemaining = delayBufferLength - mWritePosition;
        
        mDelayBuffer.addFromWithRamp(channel, bufferRemaining, dryBuffer, bufferRemaining, feedBackGain, feedBackGain);
        mDelayBuffer.addFromWithRamp(channel, 0, dryBuffer, bufferLength - bufferRemaining, feedBackGain, feedBackGain);
    }
}

void NewProjectAudioProcessor::pingPong (juce::AudioBuffer<float>& buffer)
{
    if (*pingpongParam  == true)
    {
        float nextRad = 0;
        
        auto* channeldataL = buffer.getWritePointer(0);
        auto* channeldataR = buffer.getWritePointer(1);
        
        float mSeconds = *timeParam / 1000;
        
        int numberSamples = mSampleRate * mSeconds;
        const float radsPerSample = (2 * juce::double_Pi) / numberSamples;
        
        //float gSlider = gain->get();
        
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            auto inputL = channeldataL[i];
            auto inputR = channeldataR[i];
            
            float sinValue = std::sin(nextRad) + 1;
            sinValue = (sinValue * juce::double_Pi) / 4;
            
            inputL = inputL * cos(sinValue);
            inputR = inputR * sin(sinValue);
            
            channeldataL[i] = inputL;
            channeldataR[i] = inputR;
            
            nextRad += radsPerSample;
        }
        
        if (nextRad > numberSamples)
        {
            nextRad = 0;
        }
    }
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this, state);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto stateTree = state.copyState();
    std::unique_ptr<juce::XmlElement> xml (stateTree.createXml());
    copyXmlToBinary(*xml, destData);
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(state.state.getType()))
    {
        state.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
