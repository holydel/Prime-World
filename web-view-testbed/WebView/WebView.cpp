// WebView.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>;
#include "WebViewInstance.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

WebViewInstance* gWebViewInstance = nullptr;

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR nCmdLine, int nCmdShow)
{
	const LPCSTR CLASS_NAME = "WindowClass";
	WNDCLASSEXA wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);

	wcex.lpszClassName = CLASS_NAME;
	RegisterClassExA(&wcex);



	HWND hwnd = CreateWindowExA(
		WS_EX_CONTROLPARENT, CLASS_NAME,
		"My First Window",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		NULL,
		NULL,
		hInstance,
		NULL);

	//WS_EX_CONTROLPARENT, GetWindowClass(), szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
	//	0, CW_USEDEFAULT, 0, nullptr, nullptr, g_hInstance, nullptr

	if (hwnd == 0) {
		return 0;
	}

	
	//inhect webview
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	nCmdShow = 1;
	MSG msg = { };
	gWebViewInstance = new WebViewInstance(hwnd);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
	return 0;
}


//int main()
//{
//	return wWinMain2(GetModuleHandleA(0), 0, SW_SHOW);
//}

HWND hHwndButton = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		//case WM_CREATE:
		//	gWebViewInstance = new WebViewInstance(hwnd);

		//	hHwndButton = CreateWindowA(
		//		"BUTTON", "Click Me", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		//		50, 50, 100, 50, hwnd, NULL, NULL, NULL);
		//	break;
		//case WM_DESTROY: {}PostQuitMessage(0); return 0;
		//case WM_PAINT:
		//{
		//	PAINTSTRUCT ps;
		//	HDC hdc = BeginPaint(hwnd, &ps);
		//	//FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 5));
		//	EndPaint(hwnd, &ps);
		//}return 0;
		//case WM_CLOSE:
		//{
		//	if (MessageBoxA(hwnd, "Close Window?", "Close", MB_OKCANCEL) == IDOK) {
		//		DestroyWindow(hwnd);
		//	}
		//}return 0;
		//}
	case WM_COMMAND:
	{
		// Handle commands here
		return 0;
	}
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
