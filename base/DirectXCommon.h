#pragma once
#include "string"
#include "vector"
#include "d3d12.h"
#include "dxgi1_6.h"
#include "d3dx12.h"

#include "wrl.h"
#include "WinApp.h"

class DirectXCommon {

public: // menber function

	static DirectXCommon* GetInstance();

	void Initialize(WinApp* win, int32_t backBufferWidth = WinApp::windowWidth,
		int32_t backBufferHeight = WinApp::windowHeight);

	void BeginDraw();

	void EndDraw();

	ID3D12Device* GetDevice();

	IDXGIFactory6* GetFactory();

	ID3D12GraphicsCommandList* GetCmdList();

	size_t GetBackBuffer();

private: // menber Variable

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	WinApp* win_;

	ComPtr<ID3D12Device>dev_ = nullptr;
	ComPtr<IDXGIFactory6> dxgiFactory_ = nullptr;
	ComPtr<ID3D12CommandAllocator> cmdAllocator_ = nullptr;
	ComPtr<ID3D12GraphicsCommandList> cmdList_ = nullptr;
	ComPtr<ID3D12CommandQueue> cmdQueue_ = nullptr;
	ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeaps_ = nullptr;
	std::vector<ComPtr<ID3D12Resource>> backBuffer_;
	ComPtr<ID3D12Resource> depthBuffer_ = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeap_ = nullptr;
	ComPtr<ID3D12Fence> fence = nullptr;

	int32_t backBufferWidth_ = 0;
	int32_t backBufferHeight_ = 0;
	UINT64 fenceVal = 0;


private: // menber function

	DirectXCommon() = default;
	~DirectXCommon() = default;
	DirectXCommon(const DirectXCommon& obj) = delete;
	DirectXCommon operator==(const DirectXCommon& obj) = delete;

	void InitializeDXGIDevice();

	void InitializeCommand();

	void CreateSwapChain();

	void CreateFinalRenderTarget();

	void CreateDepthBuffer();

	void CreateFence();

};

