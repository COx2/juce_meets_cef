#pragma once

#include <JuceHeader.h>
#include "cefsimple/simple_handler.h"

class RingBufferSingleChannel
{
public:
    RingBufferSingleChannel()
    {
        abstractFifo.reset();
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
    juce::AbstractFifo abstractFifo{ 65536 };
    std::array<float, 65536> myBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingBufferSingleChannel)
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

private:
    //==============================================================================
    virtual void onAudioStreamStarted() override;
    virtual void onAudioStreamPacket(juce::AudioBuffer<float>& buffer) override;
    virtual void onAudioStreamStopped() override;

    //==============================================================================
    virtual void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    virtual void releaseResources() override;
    virtual void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    RingBufferSingleChannel ringBufferChannelLeft;
    RingBufferSingleChannel ringBufferChannelRight;

    std::unique_ptr<juce::AudioDeviceSelectorComponent> deviceSelector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CefJuceAudioSink)
};
