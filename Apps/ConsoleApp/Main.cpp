#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <JuceHeader.h>
#include "MainComponent.h"
#include "CefJuceAudioSink.h"

#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "cefsimple/simple_app.h"

//==============================================================================
class MainWindow final
    : public juce::DocumentWindow
{
public:
    explicit MainWindow(juce::String name)
        : DocumentWindow(name,
            juce::Desktop::getInstance().getDefaultLookAndFeel()
            .findColour(ResizableWindow::backgroundColourId),
            DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new MainComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
        setFullScreen(true);
#else
        setResizable(true, true);
        centreWithSize(getWidth(), getHeight());
#endif

        setVisible(true);
    }

    void closeButtonPressed() override
    {
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

int main (int argc, char* argv[])
{
    // Your code goes here!
    juce::ignoreUnused (argc, argv);

    int exit_code;

#if defined(ARCH_CPU_32_BITS)
    // Run the main thread on 32-bit Windows using a fiber with the preferred 4MiB
    // stack size. This function must be called at the top of the executable entry
    // point function (`main()` or `wWinMain()`). It is used in combination with
    // the initial stack size of 0.5MiB configured via the `/STACK:0x80000` linker
    // flag on executable targets. This saves significant memory on threads (like
    // those in the Windows thread pool, and others) whose stack size can only be
    // controlled via the linker flag.
    exit_code = CefRunWinMainWithPreferredStackSize(wWinMain, hInstance,
        lpCmdLine, nCmdShow);
    if (exit_code >= 0) {
        // The fiber has completed so return here.
        return exit_code;
    }
#endif

    void* sandbox_info = nullptr;

#if defined(CEF_USE_SANDBOX)
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif

    // Provide CEF with command-line arguments.
    HINSTANCE hInstance = (HINSTANCE)juce::Process::getCurrentModuleInstanceHandle();
    CefMainArgs main_args(hInstance);

    // CEF applications have multiple sub-processes (render, GPU, etc) that share
    // the same executable. This function checks the command-line and, if this is
    // a sub-process, executes the appropriate logic.
    exit_code = CefExecuteProcess(main_args, nullptr, sandbox_info);
    if (exit_code >= 0) {
        // The sub-process has completed so return here.
        return exit_code;
    }

    // Parse command-line arguments for use in this method.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromString(::GetCommandLineW());

    // Specify CEF global settings here.
    CefSettings settings;

    if (command_line->HasSwitch("enable-chrome-runtime")) {
        // Enable experimental Chrome runtime. See issue #2969 for details.
        settings.chrome_runtime = true;
    }

#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    // SimpleApp implements application-level callbacks for the browser process.
    // It will create the first browser instance in OnContextInitialized() after
    // CEF has initialized.
    CefRefPtr<SimpleApp> app(new SimpleApp);

    // Initialize the CEF browser process. May return false if initialization
    // fails or if early exit is desired (for example, due to process singleton
    // relaunch behavior).
    if (!CefInitialize(main_args, settings, app.get(), sandbox_info)) {
        return 1;
    }

    // Create main window of JUCE.
    auto scoped_gui = juce::ScopedJuceInitialiser_GUI();

    std::unique_ptr<MainWindow> mainWindow;
    std::shared_ptr<CefJuceAudioSink> audio_sink;

    juce::MessageManager::getInstance()->callAsync([&]() {
        mainWindow = std::make_unique<MainWindow>("JUCE and CEF");

        audio_sink = std::make_shared<CefJuceAudioSink>();
        app->GetSimpleHandler()->addJuceAudioSink(audio_sink);

        audio_sink->getDeviceSelector()->setBounds(0, 0, 400, 600);
        mainWindow->getContentComponent()->addAndMakeVisible(audio_sink->getDeviceSelector());
    });

    // Run the CEF message loop. This will block until CefQuitMessageLoop() is
    // called.
    CefRunMessageLoop();

    app->GetSimpleHandler()->removeJuceAudioSink(audio_sink);
    mainWindow.reset();
    audio_sink.reset();

    // Shut down CEF.
    CefShutdown();

    return 0;
}
