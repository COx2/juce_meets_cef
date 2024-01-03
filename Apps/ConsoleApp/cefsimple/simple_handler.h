// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"

#include <list>

#include <juce_audio_utils/juce_audio_utils.h>

class JuceAudioSink
{
public:
    virtual ~JuceAudioSink() {};

    virtual void onAudioStreamStarted() = 0;
    virtual void onAudioStreamPacket(juce::AudioBuffer<float>& buffer) = 0;
    virtual void onAudioStreamStopped() = 0;

    CefAudioParameters audioParameters;
private:

    JUCE_LEAK_DETECTOR(JuceAudioSink)
};

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefAudioHandler
{
public:
    std::weak_ptr<JuceAudioSink> juceAudioSink_;

    void addJuceAudioSink(std::shared_ptr<JuceAudioSink> sink)
    {
        juceAudioSink_ = sink;
    }

    void removeJuceAudioSink(std::shared_ptr<JuceAudioSink> sink)
    {
        juceAudioSink_.reset();
    }

  explicit SimpleHandler(bool use_views);
  ~SimpleHandler();

  // Provide access to the single global instance of this object.
  static SimpleHandler* GetInstance();

  // CefClient methods:
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

  virtual CefRefPtr<CefAudioHandler> GetAudioHandler() override
  {
      return this;
  }

  // CefDisplayHandler methods:
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) override;

  // CefLifeSpanHandler methods:
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler methods:
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) override;

  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

  bool IsClosing() const { return is_closing_; }

  // Returns true if the Chrome runtime is enabled.
  static bool IsChromeRuntimeEnabled();

  // CefAudioHandler methods:
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
  // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);

  // True if the application is using the Views framework.
  const bool use_views_;

  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;

  bool is_closing_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
