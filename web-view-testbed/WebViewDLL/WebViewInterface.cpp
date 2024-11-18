#include "pch.h"
#include "WebViewInterface.h"
#include "WebViewImpl.h"

__declspec(dllexport) void* InitializeWebView(void* handle)
{
	// Initialize COM
	InitializeWebViewImpl((HWND)handle);

	return (void*)42;
}

__declspec(dllexport) void HideWebView(void* handle)
{

}

__declspec(dllexport) void ShowWebView(void* handle)
{

}

__declspec(dllexport) void FreeWebView(void* handle)
{

}