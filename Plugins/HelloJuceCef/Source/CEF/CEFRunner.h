#pragma once

#include <JuceHeader.h>

//==============================================================================
class IHostComponentInformationProvider
{
public:
    //==============================================================================
    virtual juce::Rectangle<int> getHostComponentRectangle() const = 0;
    virtual void onRendererCreated() = 0;

private:
    //==============================================================================

    JUCE_LEAK_DETECTOR(IHostComponentInformationProvider)
};

//==============================================================================
class CEFRunner
    : public juce::Thread
{
public:
    //==============================================================================
    explicit CEFRunner(IHostComponentInformationProvider& hostComponent);
    ~CEFRunner() override;

    //==============================================================================
    void* getNativeHandle() const;
    void quitCEFMessageLoop();

private:
    //==============================================================================
    virtual void run() override;

    //==============================================================================
    mutable std::mutex mutex_;

    std::atomic<sdl2::SDL_Window*> sdlWindow_;
    std::atomic<sdl2::SDL_Renderer*> sdlRenderer_;
    std::atomic<bool> entereRunLoop_;

    IHostComponentInformationProvider& hostComponentRef_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CEFRunner)
};
