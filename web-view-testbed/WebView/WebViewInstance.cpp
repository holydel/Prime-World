#include "WebViewInstance.h"

#define WIN32_LEAN_AND_MEAN
#define NO_MIN_MAX
#include <Windows.h>

struct WebViewInstanceImpl
{
	HWND winHandle;
	HMODULE dll;

	void* (*InitializeWebView)(void* handle);
	void (*HideWebView)(void* handle);
	void (*ShowWebView)(void* handle);
	void (*FreeWebView)(void* handle);
};

WebViewInstance::WebViewInstance(HWND hwnd)
{
	pimpl = new WebViewInstanceImpl();
	pimpl->winHandle = hwnd;
	pimpl->dll = LoadLibraryA("WebViewDLL.dll");

	if (pimpl->dll)
	{
		pimpl->InitializeWebView = (decltype(pimpl->InitializeWebView))GetProcAddress(pimpl->dll, "InitializeWebView");
		pimpl->HideWebView = (decltype(pimpl->HideWebView))GetProcAddress(pimpl->dll, "HideWebView");
		pimpl->ShowWebView = (decltype(pimpl->ShowWebView))GetProcAddress(pimpl->dll, "ShowWebView");
		pimpl->FreeWebView = (decltype(pimpl->FreeWebView))GetProcAddress(pimpl->dll, "FreeWebView");

		pimpl->InitializeWebView(hwnd);
	}
}

WebViewInstance::~WebViewInstance()
{
	delete pimpl;
}
