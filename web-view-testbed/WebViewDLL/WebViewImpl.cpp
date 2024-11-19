#include "WebViewImpl.h"
#include <WebView2.h>
#include <wrl.h>
#include <wil/com.h>
using namespace Microsoft::WRL;

static     ICoreWebView2Controller* webviewController = nullptr;
static     ICoreWebView2* webview = nullptr;
static ICoreWebView2Environment* webviewEnv = nullptr;

void InitializeWebViewImpl(HWND hWnd)
{
    OleInitialize(NULL);

    // Step 3 - Create a single WebView within the parent window
// Locate the browser and set up the environment for WebView
    //CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
    //    Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
    //        [hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

    //    // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
    //    env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
    //        [hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
    //        if (controller != nullptr) {
    //            webviewController = controller;
    //            webviewController->get_CoreWebView2(&webview);
    //        }

    //        // Add a few settings for the webview
    //        // The demo step is redundant since the values are the default settings
    //        wil::com_ptr<ICoreWebView2Settings> settings;
    //        webview->get_Settings(&settings);
    //        settings->put_IsScriptEnabled(TRUE);
    //        settings->put_AreDefaultScriptDialogsEnabled(TRUE);
    //        settings->put_IsWebMessageEnabled(TRUE);

    //        // Resize WebView to fit the bounds of the parent window
    //        RECT bounds;
    //        GetClientRect(hWnd, &bounds);
    //        webviewController->put_Bounds(bounds);

    //        // Schedule an async task to navigate to Bing
    //        webview->Navigate(L"https://www.bing.com/");

    //        // Step 4 - Navigation events

    //        // Step 5 - Scripting

    //        // Step 6 - Communication between host and web content

    //        return S_OK;
    //    }).Get());
    //    return S_OK;
    //}).Get());
    static HRESULT CreateEnvResult = -1;
    static HRESULT CreateControllerResult = -1;

    wil::unique_cotaskmem_string version_info;
    HRESULT hr = GetAvailableCoreWebView2BrowserVersionString(nullptr, &version_info);

    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
        webviewEnv = env;
        CreateEnvResult = result;
        // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
        webviewEnv->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
            CreateControllerResult = result;
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
            webview->Navigate(L"https://playpw.fun/");
            webviewController->put_IsVisible(TRUE);

            SendMessage(hWnd, WM_PAINT, 0, 0);
            // Step 4 - Navigation events

            // Step 5 - Scripting

            // Step 6 - Communication between host and web content

            return S_OK;
        }).Get());
        return S_OK;
    }).Get());
}

