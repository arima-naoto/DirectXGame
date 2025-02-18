#include "WinApp.h"

// ウィンドウプロシージャ
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

WinApp* WinApp::GetInstance()
{
	static WinApp instance;
	return &instance;
}

void WinApp::CreateGameWindow(std::wstring title, int currentWidth, int currentHeight) {

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = (WNDPROC)WindowProc;
	wc.lpszClassName = (L"DirectXGame");
	wc.hInstance = GetModuleHandle(0);

	RegisterClassEx(&wc);

	RECT wrc = { 0, 0, currentWidth,currentHeight };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);


	hwnd = CreateWindow(wc.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, wc.hInstance, nullptr);

	ShowWindow(hwnd, SW_SHOW);
}

bool WinApp::ProcessMessage()
{
	MSG msg = {};

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT) {
		return true;
	}

	return false;
}

void WinApp::Terminate() {
	UnregisterClass(wc.lpszClassName, wc.hInstance);
}

HWND WinApp::GetHWND() { return hwnd; }

WNDCLASSEX WinApp::GetWndClass() { return wc;  }


