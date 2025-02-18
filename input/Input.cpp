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
	CreateMouseDevice();
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

	Input::UpdateMouseState();
}

bool Input::PushKey(BYTE targetKey) const {
	return (keys[targetKey] & 0x80) != 0;
}

const Input::MouseState& Input::GetMouseState() const
{
	// TODO: return ステートメントをここに挿入します
	return mouseState;
}

bool Input::IsMouseButtonDown(int32_t buttonIndex) const
{
	if (buttonIndex < 0 || buttonIndex >= 3) return false;
	return mouseState.button[buttonIndex] != 0;
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

void Input::CreateMouseDevice()
{
	WinApp* win = WinApp::GetInstance();

	HRESULT result;
	result = directInput->CreateDevice(GUID_SysMouse, &mouse, nullptr);
	assert(SUCCEEDED(result));

	result = mouse->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(result));

	result = mouse->SetCooperativeLevel(win->GetHWND(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));

}

// 入データ形式のセット
void Input::SetInputData() {
	HRESULT result;
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
}

// 排他制御レベルのセット
void Input::MutualExclusionLevel() {
	HRESULT result;
	WinApp* win = WinApp::GetInstance();
	result = keyboard->SetCooperativeLevel(win->GetHWND(),
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::UpdateMouseState()
{
	if (mouse) {
		DIMOUSESTATE mouseStateData;
		HRESULT hr = mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseStateData);
		if (SUCCEEDED(hr)) {
			mouseState.x = mouseStateData.lX;
			mouseState.y = mouseStateData.lY;

			// Update button states
			mouseState.button[0] = mouseStateData.rgbButtons[0];  // Left button
			mouseState.button[1] = mouseStateData.rgbButtons[1];  // Middle button
			mouseState.button[2] = mouseStateData.rgbButtons[2];  // Right button
		}
	}
}
