#include "WebViewImpl.h"
#include <WebView2.h>
#include <wrl.h>
#include <wil/com.h>


void InitializeWebViewImpl(HWND hWnd)
{
    CoInitialize(nullptr);

    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

        // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
        env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
            if (controller != nullptr) {
                webviewController = controller;
                webviewController->get_CoreWebView2(&webview);
            }

            // Add a few settings for the webview
            // The demo step is redundant since the values are the default settings
            wil::com_ptr<ICoreWebView2Settings> settings;
            webview->get_Settings(&settings);
            settings->put_IsScriptEnabled(TRUE);
            settings->put_AreDefaultScriptDialogsEnabled(TRUE);
            settings->put_IsWebMessageEnabled(TRUE);

            // Resize WebView to fit the bounds of the parent window
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            webviewController->put_Bounds(bounds);

            // Schedule an async task to navigate to Bing
            webview->Navigate(L"https://www.bing.com/");

            // Step 4 - Navigation events

            // Step 5 - Scripting

            // Step 6 - Communication between host and web content

            return S_OK;
        }).Get());
        return S_OK;
    }).Get());
}

