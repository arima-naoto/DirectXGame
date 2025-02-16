#pragma once

#ifdef _DEBUG
#include "d3d12.h"
#include "imgui.h"
#include "wrl.h"
using Microsoft::WRL::ComPtr;
#endif // _DEBUG

class WinApp;
class DirectXCommon;


class ImGuiManager
{
public:
	static ImGuiManager* GetInstance();

	void Initialize(WinApp* win, DirectXCommon* dxCommon);

	void Begin();

	void End();

	void Draw();

	void Finalize();

private:


	DirectXCommon* dxCommon_ = nullptr;

#ifdef _DEBUG
	ComPtr<ID3D12DescriptorHeap> ret_;
#endif // _DEBUG

private:

	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager& obj) = delete;
	ImGuiManager &operator=(const ImGuiManager& obj) = delete;

};

