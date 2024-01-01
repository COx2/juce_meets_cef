#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "CEF/CEFRunner.h"

//==============================================================================
class CefHostingComponent
    : public juce::Component
    , public IHostComponentInformationProvider
{
public:
    //==============================================================================
    CefHostingComponent();
    ~CefHostingComponent() override;

private:
    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void parentHierarchyChanged() override;

    //==============================================================================
    virtual juce::Rectangle<int> getHostComponentRectangle() const override;
    virtual void onRendererCreated() override;

    std::unique_ptr<CEFRunner> cefRunner_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CefHostingComponent)
};

//==============================================================================
class AudioPluginAudioProcessorEditor final
    : public juce::AudioProcessorEditor
    //, public IHostComponentInformationProvider
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //virtual juce::Rectangle<int> getHostComponentRectangle() const override;
    //virtual void onRendererCreated() override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    //juce::Rectangle<int> cefHostRectangle_;

#if JUCE_WINDOWS
    std::unique_ptr<juce::HWNDComponent> juceHwndView;
#elif JUCE_MAC
    std::unique_ptr<juce::NSViewComponent> juceNsView;
#elif JUCE_LINUX
    std::unique_ptr<juce::XEmbedComponent> juceXEmbedView;
#endif


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
