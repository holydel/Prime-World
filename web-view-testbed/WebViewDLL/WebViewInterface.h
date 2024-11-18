#pragma once

extern "C"
{
	__declspec(dllexport) void* InitializeWebView(void* handle);
	__declspec(dllexport) void HideWebView(void* handle);
	__declspec(dllexport) void ShowWebView(void* handle);
	__declspec(dllexport) void FreeWebView(void* handle);
}