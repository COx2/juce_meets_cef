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

#if 0
    // Test creating a command line using set and append methods.
    TEST(CommandLineTest, Manual) {
        CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
        EXPECT_TRUE(command_line.get() != NULL);

        command_line->SetProgram("test.exe");
        command_line->AppendSwitch("switch1");
        command_line->AppendSwitchWithValue("switch2", "val2");
        command_line->AppendSwitchWithValue("switch3", "val3");
        command_line->AppendSwitchWithValue("switch4", "val 4");
        command_line->AppendArgument("arg1");
        command_line->AppendArgument("arg 2");

        VerifyCommandLine(command_line);
    }
#endif
    // Parse command-line arguments for use in this method.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->SetProgram(juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile).getFullPathName().toStdString());
    //command_line->AppendSwitch("--enable-unsafe-webgpu");
    //command_line->AppendSwitchWithValue("--use-webgpu-adapter", "d3d11");
    command_line->AppendArgument("--enable-unsafe-webgpu");
    command_line->AppendArgument("--use-webgpu-adapter=d3d11");
    //command_line->AppendSwitchWithValue("switch3", "val3");
    //command_line->AppendSwitchWithValue("switch4", "val 4");
    //command_line->AppendArgument("arg1");
    //command_line->AppendArgument("arg 2");

    // CEF applications have multiple sub-processes (render, GPU, etc) that share
    // the same executable. This function checks the command-line and, if this is
    // a sub-process, executes the appropriate logic.
    exit_code = CefExecuteProcess(main_args, nullptr, sandbox_info);
    if (exit_code >= 0) {
        // The sub-process has completed so return here.
        return exit_code;
    }

    //CefString command_line_string = ::GetCommandLineW();
    //CefString append_string = L" --enable-unsafe-webgpu --use-webgpu-adapter=d3d11";
    //command_line_string = command_line_string.ToString16().insert(command_line_string.ToString16().length() - 2, append_string.ToString16());

    //juce::Logger::outputDebugString(command_line_string.ToString());
    //std::cout << command_line_string.ToString() << "\n" << std::endl;

    //command_line->InitFromString(command_line_string);

    //command_line->AppendArgument(L"--enable-unsafe-webgpu");
    //command_line->AppendArgument(L"--use-webgpu-adapter=d3d11");

    {
        juce::Logger::outputDebugString(command_line->GetProgram().ToString());

        CefCommandLine::ArgumentList cef_arg_list;
        command_line->GetArguments(cef_arg_list);
        for (const auto& arg : cef_arg_list)
        {
            juce::Logger::outputDebugString(arg.ToString());
            std::cout << arg  << std::endl;
        }
    }
    
    //{
    //    auto gcl = CefCommandLine::GetGlobalCommandLine();
    //    CefCommandLine::ArgumentList cef_arg_list;
    //    gcl->GetArguments(cef_arg_list);
    //    for (const auto& arg : cef_arg_list)
    //    {
    //        juce::Logger::outputDebugString(arg.ToString());
    //        std::cout << arg << "\n" << std::endl;
    //    }
    //}

    // Specify CEF global settings here.
    CefSettings settings;

    juce::String juce_path =
        juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile)
        .getParentDirectory()
        .getChildFile("Cache")
        .getFullPathName();
    cef_string_utf8_to_utf16(juce_path.toRawUTF8(), juce_path.length(), &settings.root_cache_path);

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

        audio_sink->getDeviceSelector()->setBounds(0, 0, 400, 800);

        audio_sink->getDSPModuleEditor()->setBounds(400, 0, 800, 800);

        mainWindow->getContentComponent()->addAndMakeVisible(audio_sink->getDeviceSelector());
        mainWindow->getContentComponent()->addAndMakeVisible(audio_sink->getDSPModuleEditor());
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
