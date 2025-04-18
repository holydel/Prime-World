#include "WebViewImpl.h"
#include <iostream>
#include <webview.h>

void InitializeWebViewImpl(HWND hWnd)
{
	webview_t w = webview_create(0, hWnd);
	//webview_set_title(w, "Basic Example");
	webview_set_size(w, 1280, 720, WEBVIEW_HINT_NONE);
	//webview_set_html(w, "Thanks for using webview!");
	webview_navigate(w, "https://playpw.fun/");
	webview_run(w);
	//webview_
	//webview_destroy(w);

	//SetParent((HWND)webview_get_window(w), hWnd);
}

void UpdateWebViewImpl(void* handle)
{

}