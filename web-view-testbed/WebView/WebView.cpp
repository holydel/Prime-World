// WebView.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>;
#include "WebViewInstance.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



int WINAPI wWinMain2(HINSTANCE hInstance, PWSTR nCmdLine, int nCmdShow)
{
	const LPCSTR CLASS_NAME = "WindowClass";
	WNDCLASSA wc = { };
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = CLASS_NAME;
	wc.hInstance = hInstance;
	RegisterClassA(&wc);
	HWND hwnd = CreateWindowExA(
		0,
		CLASS_NAME,
		"My First Window",
		WS_OVERLAPPEDWINDOW,
		200,
		200,
		800,
		600,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (hwnd == 0) {
		return 0;
	}

	WebViewInstance webView(hwnd);
	//inhect webview
	ShowWindow(hwnd, nCmdShow);
	nCmdShow = 1;
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}


int main()
{
	return wWinMain2(GetModuleHandleA(0), 0, SW_SHOW);
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY: {}PostQuitMessage(0); return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 5));
		EndPaint(hwnd, &ps);
	}return 0;
	case WM_CLOSE:
	{
		if (MessageBoxA(hwnd, "Close Window?", "Close", MB_OKCANCEL) == IDOK) {
			DestroyWindow(hwnd);
		}
	}return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
