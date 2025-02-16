#include "WinApp.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "Audio.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WinApp* win = nullptr;
	DirectXCommon* dxCommon = nullptr;
	Input* input = nullptr;
	Audio* audio = nullptr;

	win = WinApp::GetInstance();
	win->CreateGameWindow();

	dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(win);

	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	imguiManager->Initialize(win, dxCommon);

	input = Input::GetInstance();
	input->Initialize();

	audio = Audio::GetInstance();
	audio->Initialize();

	while (true) {
		if (win->ProcessMessage()) {
			break;
		}

		imguiManager->Begin();

		input->Update();

		imguiManager->End();

		dxCommon->BeginDraw();
		imguiManager->Draw();
		
		dxCommon->EndDraw();
	}

	audio->Finalize();
	imguiManager->Finalize();
	win->Terminate();

	return 0;
}