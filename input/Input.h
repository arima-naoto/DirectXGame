#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#include "array"
#include "wrl.h"
using Microsoft::WRL::ComPtr;

/// <summary>
/// インプットクラス
/// </summary>
class Input
{
public:
	struct MouseState {
		LONG x = 0;
		LONG y = 0;
		BYTE button[3] = {};
	};

public:// メンバ関数

	/// <summary>
	/// シングルトン
	/// </summary>
	/// <returns></returns>
	static Input* GetInstance();
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();
	/// <summary>
	/// 押されている間
	/// </summary>
	/// <param name="targetKey">対象のキー</param>
	/// <returns></returns>
	bool PushKey(BYTE targetKey) const;

private:// メンバ関数

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Input() = default;
	/// <summary>
	/// デストラクタ
	/// </summary>
	~Input() = default;
	/// <summary>
	/// コピーコンストラクタ
	/// </summary>
	/// <param name="obj">インプット</param>
	Input(const Input& obj) = delete;
	/// <summary>
	/// コピー演算子
	/// </summary>
	/// <param name="obj"></param>
	/// <returns></returns>
	Input &operator=(const Input& obj) = delete;

	/// <summary>
	/// DirectInputの初期化
	/// </summary>
	void InitializeDirectInput();
	/// <summary>
	/// キーボードデバイスの生成
	/// </summary>
	void CreateKeyboradDevice();
	/// <summary>
	/// 入力データの形式設定
	/// </summary>
	void SetInputData();
	/// <summary>
	/// 排他制御データのレベル
	/// </summary>
	void MutualExclusionLevel();



private:

	ComPtr<IDirectInput8> directInput = nullptr;
	ComPtr<IDirectInputDevice8> keyboard = nullptr;
	ComPtr<IDirectInputDevice8> mouse = nullptr;

	std::array<BYTE,256> keys = {};
	MouseState mouseState;

};

