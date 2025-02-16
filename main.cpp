#include "WinApp.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WinApp* win = nullptr;
	DirectXCommon* dxCommon = nullptr;

	win = WinApp::GetInstance();
	win->CreateGameWindow();

	dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(win);

	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	imguiManager->Initialize(win, dxCommon);

	while (true) {
		if (win->ProcessMessage()) {
			break;
		}

		imguiManager->Begin();		
		imguiManager->End();

		dxCommon->BeginDraw();
		imguiManager->Draw();
		
		dxCommon->EndDraw();
	}

	imguiManager->Finalize();
	win->Terminate();

	return 0;
}