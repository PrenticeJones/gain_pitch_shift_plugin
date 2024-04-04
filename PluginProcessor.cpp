/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TestTake2AudioProcessor::TestTake2AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), chorusEffect()//, lowPassFilter(juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(44100, 2000.0f)
#endif
{
}

TestTake2AudioProcessor::~TestTake2AudioProcessor()
{
}

//==============================================================================
const juce::String TestTake2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TestTake2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TestTake2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TestTake2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TestTake2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TestTake2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TestTake2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void TestTake2AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TestTake2AudioProcessor::getProgramName (int index)
{
    return {};
}

void TestTake2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TestTake2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();

    chorusEffect.prepare(spec);

    chorusEffect.setCentreDelay(27.0f);
    chorusEffect.setMix(0.8f);
    chorusEffect.setFeedback(0.8f);
    chorusEffect.setDepth(0.8f);
    //lowPassFilter.prepare(spec);
}

void TestTake2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TestTake2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TestTake2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    auto numSamples = buffer.getNumSamples();
    juce::AudioBuffer<float> tempBuffer(totalNumInputChannels, numSamples);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* readPointer = buffer.getReadPointer(channel);
        auto* writePointer = tempBuffer.getWritePointer(channel);
        float readPosition = 0.0f;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            auto readIndex = static_cast<int>(readPosition);
            auto nextIndex = readIndex + 1 < numSamples ? readIndex + 1 : readIndex;
            auto frac = readPosition - readIndex;

            // Simple linear interpolation for fractional positions
            float sampleValue = (1 - frac) * readPointer[readIndex] + frac * readPointer[nextIndex];
            writePointer[sample] = sampleValue;

            // Increment readPosition by pitchShift amount to adjust playback speed and pitch
            readPosition += pitchShift;
            if (readPosition >= numSamples) readPosition -= numSamples; // Wrap around to avoid buffer overrun
        }
    }

    // Apply gain after pitch shifting
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Applying gain to the buffer
            channelData[sample] = tempBuffer.getSample(channel, sample) * rawVolume;
        }
    }
}



//==============================================================================
bool TestTake2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TestTake2AudioProcessor::createEditor()
{
    return new TestTake2AudioProcessorEditor (*this);
}

//==============================================================================
void TestTake2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TestTake2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}
//==============================================================================


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TestTake2AudioProcessor();
}

