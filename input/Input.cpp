#include "Input.h"
#include "WinApp.h"
#include "cassert"
#include "Input.h"

// シングルトンの定義
Input* Input::GetInstance() {
	static Input instance;
	return &instance;
}

// 初期化
void Input::Initialize() {
	InitializeDirectInput();
	CreateKeyboradDevice();
	SetInputData();
	MutualExclusionLevel();
}

// 更新
void Input::Update() {
	// キーボード情報の取得開始
	HRESULT result = keyboard->GetDeviceState(sizeof(keys), keys.data());

	if (FAILED(result)) {
		keyboard->Acquire(); // デバイスを取得し直す
		keyboard->GetDeviceState(sizeof(keys), keys.data()); // 再取得
	}
}

bool Input::PushKey(BYTE targetKey) const {
	return (keys[targetKey] & 0x80) != 0;
}

// DirectInputの初期化
void Input::InitializeDirectInput() {

	WinApp* win = WinApp::GetInstance();

	HRESULT result;

	result = DirectInput8Create(
		win->GetWndClass().hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));
}

// キーボードデバイスの生成
void Input::CreateKeyboradDevice()
{
	HRESULT result;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));
}

// 入データ形式のセット
void Input::SetInputData() {
	HRESULT result;
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
}

void Input::MutualExclusionLevel()
{
	WinApp* win = WinApp::GetInstance();
	HRESULT result;
	result = keyboard->SetCooperativeLevel(
		win->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

