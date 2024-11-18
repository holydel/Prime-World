#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NO_MIN_MAX
#define NO_MIN_MAX
#endif

#include <Windows.h>

struct WebViewInstanceImpl;

struct WebViewInstance
{
	WebViewInstance(HWND hwnd);
	~WebViewInstance();
private:
	WebViewInstanceImpl* pimpl = nullptr;
};