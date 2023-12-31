// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"

#include <sstream>
#include <string>

#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

namespace {

SimpleHandler* g_instance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

}  // namespace

SimpleHandler::SimpleHandler(bool use_views)
    : use_views_(use_views), is_closing_(false) {
  DCHECK(!g_instance);
  g_instance = this;
}

SimpleHandler::~SimpleHandler() {
  g_instance = nullptr;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
  return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  if (use_views_) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefBrowserView> browser_view =
        CefBrowserView::GetForBrowser(browser);
    if (browser_view) {
      CefRefPtr<CefWindow> window = browser_view->GetWindow();
      if (window) {
        window->SetTitle(title);
      }
    }
  } else if (!IsChromeRuntimeEnabled()) {
    // Set the title of the window using platform APIs.
    PlatformTitleChange(browser, title);
  }
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Allow Chrome to show the error page.
  if (IsChromeRuntimeEnabled()) {
    return;
  }

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED) {
    return;
  }

  // Display a load error message using a data: URI.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::CloseAllBrowsers, this,
                                       force_close));
    return;
  }

  if (browser_list_.empty()) {
    return;
  }

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it) {
    (*it)->GetHost()->CloseBrowser(force_close);
  }
}

// static
bool SimpleHandler::IsChromeRuntimeEnabled() {
  static int value = -1;
  if (value == -1) {
    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();
    value = command_line->HasSwitch("enable-chrome-runtime") ? 1 : 0;
  }
  return value == 1;
}

bool SimpleHandler::GetAudioParameters(CefRefPtr<CefBrowser> browser, CefAudioParameters& params)
{
    params.sample_rate = 48000;
    //params.sample_rate = 16000;
    params.channel_layout = CEF_CHANNEL_LAYOUT_STEREO;
    //params.channel_layout = CEF_CHANNEL_LAYOUT_MONO;
    params.frames_per_buffer = 1024;
    return true;

    //if (params.sample_rate == 44100)
    //{
    //    return false;
    //}
    //else if (params.sample_rate == 48000)
    //{
    //    return true;
    //}

    //return false;
}

void SimpleHandler::OnAudioStreamStarted(CefRefPtr<CefBrowser> browser, const CefAudioParameters& params, int channels)
{
    if (!juceAudioSink_.expired())
    {
        juceAudioSink_.lock()->audioParameters = params;

        juceAudioSink_.lock()->onAudioStreamStarted();
    }
}

void SimpleHandler::OnAudioStreamPacket(CefRefPtr<CefBrowser> browser, const float** data, int frames, int64_t pts)
{
    if (!juceAudioSink_.expired())
    {
        auto params = juceAudioSink_.lock()->audioParameters;

        juce::AudioBuffer<float> buffer_to_send;
        const auto samples = frames;
        buffer_to_send.setSize(2, samples);
        buffer_to_send.clear();

        if (params.channel_layout == CEF_CHANNEL_LAYOUT_MONO)
        {
            for (int frame_idx = 0; frame_idx < frames; frame_idx++)
            {
                auto* f_l = data[0];
                buffer_to_send.setSample(0, frame_idx, f_l[frame_idx]);
            }
        }
        else if (params.channel_layout == CEF_CHANNEL_LAYOUT_STEREO)
        {
            for (int frame_idx = 0; frame_idx < frames; frame_idx++)
            {
                auto* f_l = data[0];
                auto* f_r = data[1];
                buffer_to_send.setSample(0, frame_idx, f_l[frame_idx]);
                buffer_to_send.setSample(1, frame_idx, f_r[frame_idx]);
            }
        }

        juceAudioSink_.lock()->onAudioStreamPacket(buffer_to_send);
    }
}

void SimpleHandler::OnAudioStreamStopped(CefRefPtr<CefBrowser> browser)
{
    if (!juceAudioSink_.expired())
    {
        juceAudioSink_.lock()->onAudioStreamStopped();
    }
}

void SimpleHandler::OnAudioStreamError(CefRefPtr<CefBrowser> browser, const CefString& message)
{
}
