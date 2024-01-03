#include "CefJuceAudioSink.h"
#include "DSPModulePluginDemo.h"

//==============================================================================
CefJuceAudioSink::CefJuceAudioSink()
    : deviceSampleRate(48000)
    , browserSampleRate(48000)
{
    ringBufferAudioSource = std::make_unique<RingBufferAudioSource>();

    resamplingAudioSource = std::make_unique<juce::ResamplingAudioSource>(ringBufferAudioSource.get(), false, ringBufferAudioSource->getNumChannels());

    dspModuleProcessr = std::make_unique<DspModulePluginDemoAudioProcessor>();
    dspModuleProcessr->setPlayConfigDetails(2, 2, 48000, 512);

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

juce::Component* CefJuceAudioSink::getDSPModuleEditor() const
{
    return dspModuleProcessr->createEditorIfNeeded();
}

//==============================================================================
void CefJuceAudioSink::onAudioStreamStarted()
{
    juce::Logger::outputDebugString(juce::String("onAudioStreamStarted channel_layout: " + juce::String(audioParameters.channel_layout)));
    juce::Logger::outputDebugString(juce::String("onAudioStreamStarted sample_rate: " + juce::String(audioParameters.sample_rate)));
    juce::Logger::outputDebugString(juce::String("onAudioStreamStarted frames_per_buffer: " + juce::String(audioParameters.frames_per_buffer)));

    browserSampleRate = audioParameters.sample_rate;

    ringBufferAudioSource->prepareToPlay(audioParameters.frames_per_buffer, audioParameters.sample_rate);

    // TODO: update resampling setting.
#if 0
    resamplingAudioSource->setResamplingRatio(deviceSampleRate / browserSampleRate);
#else
    resamplingAudioSource->setResamplingRatio(browserSampleRate / deviceSampleRate);
#endif
}

void CefJuceAudioSink::onAudioStreamPacket(juce::AudioBuffer<float>& buffer)
{
    ringBufferAudioSource->pushAudioBlock(buffer);
}

void CefJuceAudioSink::onAudioStreamStopped()
{
}

//==============================================================================
void CefJuceAudioSink::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    resamplingAudioSource->prepareToPlay(samplesPerBlockExpected, sampleRate);

    if (samplesPerBlockExpected != 0 && sampleRate != 0)
    {
        dspModuleProcessr->prepareToPlay(sampleRate, samplesPerBlockExpected);
    }

    deviceSampleRate = sampleRate;

    // TODO: update resampling setting.
#if 0
    resamplingAudioSource->setResamplingRatio(deviceSampleRate / browserSampleRate);
#else
    resamplingAudioSource->setResamplingRatio(browserSampleRate / deviceSampleRate);
#endif
}

void CefJuceAudioSink::releaseResources()
{
    resamplingAudioSource->releaseResources();

    dspModuleProcessr->releaseResources();
}

void CefJuceAudioSink::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    const float level = 0.5f;

    auto* buffer = bufferToFill.buffer;
    buffer->clear();

    if (buffer->getNumChannels() == 2)
    {
#if 0
        resamplingAudioSource->getNextAudioBlock(bufferToFill);
#else
        ringBufferAudioSource->getNextAudioBlock(bufferToFill);
#endif
    }

    juce::MidiBuffer midi_buffer;
    dspModuleProcessr->processBlock(*buffer, midi_buffer);
}

