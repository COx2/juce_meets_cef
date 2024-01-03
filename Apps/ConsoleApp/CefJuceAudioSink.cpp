#include "CefJuceAudioSink.h"

//==============================================================================
CefJuceAudioSink::CefJuceAudioSink()
{
    setAudioChannels(0, 2);

    deviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent>
        (deviceManager,
        0, 256,
        0, 256,
        true, true,
        true, false);
}

CefJuceAudioSink::~CefJuceAudioSink()
{
}

juce::Component* CefJuceAudioSink::getDeviceSelector() const
{
    return deviceSelector.get();
}

//==============================================================================
void CefJuceAudioSink::onAudioStreamStarted()
{
    juce::Logger::outputDebugString(juce::String("onAudioStreamStarted channel_layout: " + juce::String(audioParameters.channel_layout)));
    juce::Logger::outputDebugString(juce::String("onAudioStreamStarted sample_rate: " + juce::String(audioParameters.sample_rate)));
    juce::Logger::outputDebugString(juce::String("onAudioStreamStarted frames_per_buffer: " + juce::String(audioParameters.frames_per_buffer)));
}

void CefJuceAudioSink::onAudioStreamPacket(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() == 2)
    {
        ringBufferChannelLeft.writeToFifo(buffer.getReadPointer(0), buffer.getNumSamples());
        ringBufferChannelRight.writeToFifo(buffer.getReadPointer(1), buffer.getNumSamples());
    }
}

void CefJuceAudioSink::onAudioStreamStopped()
{
}

//==============================================================================
void CefJuceAudioSink::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
}

void CefJuceAudioSink::releaseResources()
{
}

void CefJuceAudioSink::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    const float level = 0.5f;

    auto* buffer = bufferToFill.buffer;
    if (buffer->getNumChannels() == 2 && ringBufferChannelLeft.getNumReady() > 4096)
    {
        ringBufferChannelLeft.readFromFifo(buffer->getWritePointer(0), buffer->getNumSamples());
        ringBufferChannelRight.readFromFifo(buffer->getWritePointer(1), buffer->getNumSamples());
    }
    else
    {
        //juce::Logger::outputDebugString(juce::String("Buffer lacking: " + juce::String(ringBufferChannelLeft.getNumReady())));
    }
}

