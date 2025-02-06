#pragma once
#include "Windows.h"
#include "string"

class WinApp
{
public:
	static const int windowWidth = 1280;
	static const int windowHeight = 720;


public:

	static WinApp* GetInstance();

	void CreateGameWindow(std::wstring title = L"DirectXGame", int currentWidth = WinApp::windowWidth,
		int currentHeight = WinApp::windowHeight);

	bool ProcessMessage();

	void Terminate();

	HWND GetHWND();

private:

	WinApp() = default;
	~WinApp() = default;
	WinApp(const WinApp& obj) = delete;
	WinApp operator==(const WinApp& obj) = delete;

private:

	HWND hwnd;
	WNDCLASSEX wc;
	RECT wrc;

};

