#pragma once

#include <JuceHeader.h>

#include <include/cef_app.h>

//==============================================================================
// Implement application-level callbacks for the browser process.
class CefSimpleApp
    : public CefApp
    , public CefBrowserProcessHandler
{
 public:
    //==============================================================================
     CefSimpleApp();
    ~CefSimpleApp() override;

    // CefApp methods:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override 
    {
        return this;
    }

    // CefBrowserProcessHandler methods:
    void OnContextInitialized() override;
    CefRefPtr<CefClient> GetDefaultClient() override;

private:
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(CefSimpleApp);
};

