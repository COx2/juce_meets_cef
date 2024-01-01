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
    , public CefRenderHandler
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

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override 
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

    //==============================================================================
    ///
    /// Return the handler for accessibility notifications. If no handler is
    /// provided the default implementation will be used.
    ///
    /*--cef()--*/
    virtual CefRefPtr<CefAccessibilityHandler> GetAccessibilityHandler() override {
        return nullptr;
    }

    ///
    /// Called to retrieve the root window rectangle in screen DIP coordinates.
    /// Return true if the rectangle was provided. If this method returns false
    /// the rectangle from GetViewRect will be used.
    ///
    /*--cef()--*/
    virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override
    {
        rect = { 0, 0, 400, 400 };
        return true;
    }

    ///
    /// Called to retrieve the view rectangle in screen DIP coordinates. This
    /// method must always provide a non-empty rectangle.
    ///
    /*--cef()--*/
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override
    {
        rect = { 0, 0, 400, 400 };
    }

    ///
    /// Called to retrieve the translation from view DIP coordinates to screen
    /// coordinates. Windows/Linux should provide screen device (pixel)
    /// coordinates and MacOS should provide screen DIP coordinates. Return true
    /// if the requested coordinates were provided.
    ///
    /*--cef()--*/
    virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
        int viewX,
        int viewY,
        int& screenX,
        int& screenY) override 
    {
        return false;
    }

    ///
    /// Called to allow the client to fill in the CefScreenInfo object with
    /// appropriate values. Return true if the |screen_info| structure has been
    /// modified.
    ///
    /// If the screen info rectangle is left empty the rectangle from GetViewRect
    /// will be used. If the rectangle is still empty or invalid popups may not be
    /// drawn correctly.
    ///
    /*--cef()--*/
    virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
        CefScreenInfo& screen_info) override
    {
        return false;
    }

    ///
    /// Called when the browser wants to show or hide the popup widget. The popup
    /// should be shown if |show| is true and hidden if |show| is false.
    ///
    /*--cef()--*/
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override {}

    ///
    /// Called when the browser wants to move or resize the popup widget. |rect|
    /// contains the new location and size in view coordinates.
    ///
    /*--cef()--*/
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override {
    }

    ///
    /// Called when an element should be painted. Pixel values passed to this
    /// method are scaled relative to view coordinates based on the value of
    /// CefScreenInfo.device_scale_factor returned from GetScreenInfo. |type|
    /// indicates whether the element is the view or the popup widget. |buffer|
    /// contains the pixel data for the whole image. |dirtyRects| contains the set
    /// of rectangles in pixel coordinates that need to be repainted. |buffer|
    /// will be |width|*|height|*4 bytes in size and represents a BGRA image with
    /// an upper-left origin. This method is only called when
    /// CefWindowInfo::shared_texture_enabled is set to false.
    ///
    /*--cef()--*/
    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height) override
    {
    }

    ///
    /// Called when an element has been rendered to the shared texture handle.
    /// |type| indicates whether the element is the view or the popup widget.
    /// |dirtyRects| contains the set of rectangles in pixel coordinates that need
    /// to be repainted. |shared_handle| is the handle for a D3D11 Texture2D that
    /// can be accessed via ID3D11Device using the OpenSharedResource method. This
    /// method is only called when CefWindowInfo::shared_texture_enabled is set to
    /// true, and is currently only supported on Windows.
    ///
    /*--cef()--*/
    virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        void* shared_handle) override 
    {}

    ///
    /// Called to retrieve the size of the touch handle for the specified
    /// |orientation|.
    ///
    /*--cef()--*/
    virtual void GetTouchHandleSize(CefRefPtr<CefBrowser> browser,
        cef_horizontal_alignment_t orientation,
        CefSize& size) override 
    {}

    ///
    /// Called when touch handle state is updated. The client is responsible for
    /// rendering the touch handles.
    ///
    /*--cef()--*/
    virtual void OnTouchHandleStateChanged(CefRefPtr<CefBrowser> browser,
        const CefTouchHandleState& state) override 
    {}

    ///
    /// Called when the user starts dragging content in the web view. Contextual
    /// information about the dragged content is supplied by |drag_data|.
    /// (|x|, |y|) is the drag start location in screen coordinates.
    /// OS APIs that run a system message loop may be used within the
    /// StartDragging call.
    ///
    /// Return false to abort the drag operation. Don't call any of
    /// CefBrowserHost::DragSource*Ended* methods after returning false.
    ///
    /// Return true to handle the drag operation. Call
    /// CefBrowserHost::DragSourceEndedAt and DragSourceSystemDragEnded either
    /// synchronously or asynchronously to inform the web view that the drag
    /// operation has ended.
    ///
    /*--cef()--*/
    virtual bool StartDragging(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDragData> drag_data,
        DragOperationsMask allowed_ops,
        int x,
        int y) override 
    {
        return false;
    }

    ///
    /// Called when the web view wants to update the mouse cursor during a
    /// drag & drop operation. |operation| describes the allowed operation
    /// (none, move, copy, link).
    ///
    /*--cef()--*/
    virtual void UpdateDragCursor(CefRefPtr<CefBrowser> browser,
        DragOperation operation) override
    {}

    ///
    /// Called when the scroll offset has changed.
    ///
    /*--cef()--*/
    virtual void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser,
        double x,
        double y) override
    {}

    ///
    /// Called when the IME composition range has changed. |selected_range| is the
    /// range of characters that have been selected. |character_bounds| is the
    /// bounds of each character in view coordinates.
    ///
    /*--cef()--*/
    virtual void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,
        const CefRange& selected_range,
        const RectList& character_bounds) override
    {}

    ///
    /// Called when text selection has changed for the specified |browser|.
    /// |selected_text| is the currently selected text and |selected_range| is
    /// the character range.
    ///
    /*--cef(optional_param=selected_text,optional_param=selected_range)--*/
    virtual void OnTextSelectionChanged(CefRefPtr<CefBrowser> browser,
        const CefString& selected_text,
        const CefRange& selected_range) override
    {}

    ///
    /// Called when an on-screen keyboard should be shown or hidden for the
    /// specified |browser|. |input_mode| specifies what kind of keyboard
    /// should be opened. If |input_mode| is CEF_TEXT_INPUT_MODE_NONE, any
    /// existing keyboard for this browser should be hidden.
    ///
    /*--cef()--*/
    virtual void OnVirtualKeyboardRequested(CefRefPtr<CefBrowser> browser,
        TextInputMode input_mode) override 
    {}

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
