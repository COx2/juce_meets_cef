#pragma once

#include <JuceHeader.h>

#include <include/cef_client.h>

#include <list>

//==============================================================================
class CefSimpleHandler
    : public CefClient
    , public CefDisplayHandler
    , public CefLifeSpanHandler
    , public CefLoadHandler
    , public CefAudioHandler
{
 public:
    //==============================================================================
    explicit CefSimpleHandler(bool use_views);
    ~CefSimpleHandler();

    // Provide access to the single global instance of this object.
    static CefSimpleHandler* GetInstance();

    //==============================================================================
    // CefClient methods:
    virtual CefRefPtr<CefAudioHandler> GetAudioHandler() override
    {
        return this;
    }

    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override 
    {
        return this;
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
    {
        return this;
    }

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override
    { 
        return this;
    }

    //==============================================================================
    // CefDisplayHandler methods:
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                                const CefString& title) override;

    //==============================================================================
    // CefLifeSpanHandler methods:
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    //==============================================================================
    // CefLoadHandler methods:
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            ErrorCode errorCode,
                            const CefString& errorText,
                            const CefString& failedUrl) override;

    //==============================================================================
    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool force_close);

    bool IsClosing() const { return is_closing_; }

    // Returns true if the Chrome runtime is enabled.
    static bool IsChromeRuntimeEnabled();

    //==============================================================================
    // CefAudioHandler methods :
    virtual bool GetAudioParameters(CefRefPtr<CefBrowser> browser,
        CefAudioParameters& params) override;

    virtual void OnAudioStreamStarted(CefRefPtr<CefBrowser> browser,
        const CefAudioParameters& params,
        int channels) override;

    virtual void OnAudioStreamPacket(CefRefPtr<CefBrowser> browser,
        const float** data,
        int frames,
        int64_t pts) override;

    virtual void OnAudioStreamStopped(CefRefPtr<CefBrowser> browser) override;

    virtual void OnAudioStreamError(CefRefPtr<CefBrowser> browser,
        const CefString& message) override;

private:
    //==============================================================================
    //// Platform-specific implementation.
    //void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
    //                        const CefString& title);

    // True if the application is using the Views framework.
    const bool use_views_;

    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browser_list_;

    bool is_closing_;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(CefSimpleHandler);
};
