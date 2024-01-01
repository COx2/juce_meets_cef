#pragma once

#include <juce_core/juce_core.h>

namespace client
{
    class MainMessageLoop;
}

class JuceCefClient
{
public:
    JuceCefClient();
    ~JuceCefClient();

    int run();
    void quit();

private:
    std::unique_ptr<client::MainMessageLoop> messageLoop_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JuceCefClient)
};
