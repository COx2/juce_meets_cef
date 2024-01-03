#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (1200, 800);
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
}
