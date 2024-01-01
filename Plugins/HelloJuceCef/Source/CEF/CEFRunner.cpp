#include "CEFRunner.h"
#include "simple_app.h"

#if JUCE_WINDOWS
#include <Windows.h>
#endif

CEFRunner::CEFRunner(IHostComponentInformationProvider& hostComponent)
    : juce::Thread("CEF Runner Thread")
    , hostComponentRef_(hostComponent)
{
}

CEFRunner::~CEFRunner()
{
    CefQuitMessageLoop();

    this->stopThread(1000);
}

void* CEFRunner::getNativeHandle() const
{
    std::unique_lock lock(mutex_);

    void* hwnd = nullptr;

    return hwnd;
}

void CEFRunner::quitCEFMessageLoop()
{
    CefQuitMessageLoop();
}

void CEFRunner::run()
{
    int exit_code;

    std::unique_lock lock(mutex_);
    {

    #if defined(ARCH_CPU_32_BITS)
        // Run the main thread on 32-bit Windows using a fiber with the preferred 4MiB
        // stack size. This function must be called at the top of the executable entry
        // point function (`main()` or `wWinMain()`). It is used in combination with
        // the initial stack size of 0.5MiB configured via the `/STACK:0x80000` linker
        // flag on executable targets. This saves significant memory on threads (like
        // those in the Windows thread pool, and others) whose stack size can only be
        // controlled via the linker flag.
        exit_code = CefRunWinMainWithPreferredStackSize(wWinMain, hInstance, lpCmdLine, nCmdShow);
        if (exit_code >= 0) {
            // The fiber has completed so return here.
            return;
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
            return;
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
        settings.chrome_runtime = true;
    #if !defined(CEF_USE_SANDBOX)
        settings.no_sandbox = true;
    #endif

        // SimpleApp implements application-level callbacks for the browser process.
        // It will create the first browser instance in OnContextInitialized() after
        // CEF has initialized.
        CefRefPtr<CefSimpleApp> app(new CefSimpleApp);

        // Initialize the CEF browser process. May return false if initialization
        // fails or if early exit is desired (for example, due to process singleton
        // relaunch behavior).
        if (!CefInitialize(main_args, settings, app.get(), sandbox_info)) {
            return;
        }

    }

    entereRunLoop_ = true;

    // Run the CEF message loop. This will block until CefQuitMessageLoop() is
    // called.
    CefRunMessageLoop();

    // Shut down CEF.
    CefShutdown();
}
