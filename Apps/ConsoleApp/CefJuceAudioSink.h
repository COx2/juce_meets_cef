#pragma once

#include <JuceHeader.h>
#include "cefsimple/simple_handler.h"

constexpr int kRingBufferSize = 1 << 16;

class RingBufferSingleChannel
{
public:
    RingBufferSingleChannel()
        : myBuffer(std::array<float, kRingBufferSize>())
        , abstractFifo(juce::AbstractFifo{ kRingBufferSize })
    {
        abstractFifo.reset();
        myBuffer.fill(0.0f);
    };
    
    ~RingBufferSingleChannel()
    {
    };

    void writeToFifo(const float* sampleData, int numSamples)
    {
        const auto scope = abstractFifo.write(numSamples);

        if (scope.blockSize1 > 0)
        {
            juce::FloatVectorOperations::copy(myBuffer.data() + scope.startIndex1, sampleData, scope.blockSize1);
        }

        if (scope.blockSize2 > 0)
        {
            juce::FloatVectorOperations::copy(myBuffer.data() + scope.startIndex2, sampleData + scope.blockSize1, scope.blockSize2);
        }
    }

    void readFromFifo(float* sampleData, int numSamples)
    {
        const auto scope = abstractFifo.read(numSamples);

        if (scope.blockSize1 > 0)
        {
            juce::FloatVectorOperations::copy(sampleData, myBuffer.data() + scope.startIndex1, scope.blockSize1);
        }

        if (scope.blockSize2 > 0)
        {
            juce::FloatVectorOperations::copy(sampleData + scope.blockSize1, myBuffer.data() + scope.startIndex2, scope.blockSize2);
        }
    }

    int getNumReady()
    {
        return abstractFifo.getNumReady();
    }

private:
    juce::AbstractFifo abstractFifo{ kRingBufferSize };
    std::array<float, kRingBufferSize> myBuffer{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingBufferSingleChannel)
};

//==============================================================================
class RingBufferAudioSource
    : public juce::AudioSource
{
public:
    RingBufferAudioSource()
        : sampleRate_(48000)
    {}

    ~RingBufferAudioSource()
    {}

    int getNumChannels() const { return 2; }
    double getSampleRate() const { return sampleRate_; }

    virtual void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        sampleRate_ = sampleRate;
    }

    virtual void releaseResources() override
    {

    }

    virtual void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
    {
        //if (isCharging_)
        //{
        //    return;
        //}

        auto* buffer = bufferToFill.buffer;
        buffer->clear();

        if (buffer->getNumChannels() == 2)
        {
            ringBufferChannelLeft.readFromFifo(buffer->getWritePointer(0), buffer->getNumSamples());
            ringBufferChannelRight.readFromFifo(buffer->getWritePointer(1), buffer->getNumSamples());
        }
    }

    void pushAudioBlock(const juce::AudioBuffer<float>& buffer)
    {
        //if (ringBufferChannelLeft.getNumReady() == 0)
        //{
        //    isCharging_ = true;
        //}

        if (buffer.getNumChannels() == 2)
        {
            ringBufferChannelLeft.writeToFifo(buffer.getReadPointer(0), buffer.getNumSamples());
            ringBufferChannelRight.writeToFifo(buffer.getReadPointer(1), buffer.getNumSamples());
        }

        //if (ringBufferChannelLeft.getNumReady() == 65536 - 1)
        //{
        //    isCharging_ = false;
        //}
    }

private:
    RingBufferSingleChannel ringBufferChannelLeft;
    RingBufferSingleChannel ringBufferChannelRight;
    double sampleRate_;
    std::atomic<bool> isCharging_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingBufferAudioSource)
};

//==============================================================================
class CefJuceAudioSink
    : public JuceAudioSink
    , public juce::AudioAppComponent
{
public:
    //==============================================================================
    CefJuceAudioSink();
    virtual ~CefJuceAudioSink() override;

    juce::Component* getDeviceSelector() const;
    juce::Component* getDSPModuleEditor() const;

private:
    //==============================================================================
    virtual void onAudioStreamStarted() override;
    virtual void onAudioStreamPacket(juce::AudioBuffer<float>& buffer) override;
    virtual void onAudioStreamStopped() override;

    //==============================================================================
    virtual void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    virtual void releaseResources() override;
    virtual void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;


    std::unique_ptr<juce::AudioDeviceSelectorComponent> deviceSelector;
    double deviceSampleRate;
    double browserSampleRate;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingAudioSource;
    std::unique_ptr<RingBufferAudioSource> ringBufferAudioSource;

    std::unique_ptr<juce::AudioProcessor> dspModuleProcessr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CefJuceAudioSink)
};
