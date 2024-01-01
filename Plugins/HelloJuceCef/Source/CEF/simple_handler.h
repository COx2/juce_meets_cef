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
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override {
    return this;
    }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
    return this;
    }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

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

private:
    //==============================================================================
    // CefAudioHandler methods :
    ///
    /// Called on the UI thread to allow configuration of audio stream parameters.
    /// Return true to proceed with audio stream capture, or false to cancel it.
    /// All members of |params| can optionally be configured here, but they are
    /// also pre-filled with some sensible defaults.
    ///
    /*--cef()--*/
    virtual bool GetAudioParameters(CefRefPtr<CefBrowser> browser,
        CefAudioParameters& params) {
        return true;
    }

    ///
    /// Called on a browser audio capture thread when the browser starts
    /// streaming audio. OnAudioStreamStopped will always be called after
    /// OnAudioStreamStarted; both methods may be called multiple times
    /// for the same browser. |params| contains the audio parameters like
    /// sample rate and channel layout. |channels| is the number of channels.
    ///
    /*--cef()--*/
    virtual void OnAudioStreamStarted(CefRefPtr<CefBrowser> browser,
        const CefAudioParameters& params,
        int channels) override;

    ///
    /// Called on the audio stream thread when a PCM packet is received for the
    /// stream. |data| is an array representing the raw PCM data as a floating
    /// point type, i.e. 4-byte value(s). |frames| is the number of frames in the
    /// PCM packet. |pts| is the presentation timestamp (in milliseconds since the
    /// Unix Epoch) and represents the time at which the decompressed packet
    /// should be presented to the user. Based on |frames| and the
    /// |channel_layout| value passed to OnAudioStreamStarted you can calculate
    /// the size of the |data| array in bytes.
    ///
    /*--cef()--*/
    virtual void OnAudioStreamPacket(CefRefPtr<CefBrowser> browser,
        const float** data,
        int frames,
        int64_t pts) override;

    ///
    /// Called on the UI thread when the stream has stopped. OnAudioSteamStopped
    /// will always be called after OnAudioStreamStarted; both methods may be
    /// called multiple times for the same stream.
    ///
    /*--cef()--*/
    virtual void OnAudioStreamStopped(CefRefPtr<CefBrowser> browser) override;

    ///
    /// Called on the UI or audio stream thread when an error occurred. During the
    /// stream creation phase this callback will be called on the UI thread while
    /// in the capturing phase it will be called on the audio stream thread. The
    /// stream will be stopped immediately.
    ///
    /*--cef()--*/
    virtual void OnAudioStreamError(CefRefPtr<CefBrowser> browser,
        const CefString& message) override;

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
