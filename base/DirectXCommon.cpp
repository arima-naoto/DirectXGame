#include "DirectXCommon.h"
#include "cassert"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

DirectXCommon* DirectXCommon::GetInstance()
{
	static DirectXCommon instance;
	return &instance;
}

void DirectXCommon::Initialize(WinApp* win, int32_t backBufferWidth, int32_t backBufferHeight)
{
	// Check NULL pointer
	assert(win);
	assert(4 <= backBufferWidth && backBufferWidth <= 4096);
	assert(4 <= backBufferHeight && backBufferHeight <= 4096);

	win_ = win;
	backBufferWidth_ = backBufferWidth;
	backBufferHeight_ = backBufferHeight;

	InitializeDXGIDevice();

	InitializeCommand();

	CreateSwapChain();

	CreateFinalRenderTarget();

	CreateDepthBuffer();

	CreateFence();

}

void DirectXCommon::BeginDraw()
{
	auto bbIdx = swapChain_->GetCurrentBackBufferIndex();

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffers_[bbIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdList_->ResourceBarrier(1, &barrier);

	auto rtvH = rtvHeaps_->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto dsvH = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
	cmdList_->OMSetRenderTargets(1, &rtvH, false, &dsvH);
	cmdList_->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	float clearColor[] = { 0.2745f, 0.5098f, 0.7059f, 1.0f };
	cmdList_->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	CD3DX12_VIEWPORT viewport = {};
	viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, float(backBufferWidth_), float(backBufferHeight_));
	cmdList_->RSSetViewports(1, &viewport);

	CD3DX12_RECT scissorRect = {};
	scissorRect = CD3DX12_RECT(LONG(0.0f), LONG(0.0f),
		LONG(scissorRect.left + backBufferWidth_), LONG(scissorRect.top + backBufferHeight_));
	cmdList_->RSSetScissorRects(1, &scissorRect);

}

void DirectXCommon::EndDraw()
{
	auto bbIdx = swapChain_->GetCurrentBackBufferIndex();

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer_[bbIdx].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,D3D12_RESOURCE_STATE_PRESENT);

	cmdList_->ResourceBarrier(1, &barrier);
	cmdList_->Close();

	ID3D12CommandList* cmdLists[] = { cmdList_.Get() };
	cmdQueue_->ExecuteCommandLists(1, cmdLists);



	cmdQueue_->Signal(fence.Get(), ++fenceVal);
	if (fence->GetCompletedValue() != fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	cmdAllocator_->Reset();
	cmdList_->Reset(cmdAllocator_.Get(), nullptr);
	swapChain_->Present(1, 0);
}

ID3D12Device* DirectXCommon::GetDevice() { return dev_.Get(); }

IDXGIFactory6* DirectXCommon::GetFactory() { return dxgiFactory_.Get(); }

ID3D12CommandAllocator* DirectXCommon::GetAllocator() { return cmdAllocator_.Get(); }

ID3D12GraphicsCommandList* DirectXCommon::GetCmdList() { return cmdList_.Get(); }

ID3D12CommandQueue* DirectXCommon::GetCmdQueue() { return cmdQueue_.Get(); }

ID3D12Fence* DirectXCommon::GetFence() { return fence.Get(); }

size_t DirectXCommon::GetBackBuffer() { return backBuffers_.size(); }

UINT64 DirectXCommon::GetFenceVal() { return fenceVal; }

void DirectXCommon::InitializeDXGIDevice() {
	HRESULT result = S_FALSE;

#ifdef _DEBUG

	ComPtr<ID3D12Debug> debugLayer = nullptr;
	result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	if (SUCCEEDED(result)) {
		debugLayer->EnableDebugLayer();
	}

#endif // _DEBUG

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
	};

	result = CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory_.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(result));

	std::vector<ComPtr<IDXGIAdapter>> adapters;
	ComPtr<IDXGIAdapter> tmpAdapter = nullptr;

	for (int i = 0; dxgiFactory_->EnumAdapters(i, &tmpAdapter) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(tmpAdapter);
	}

	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;

		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			break;
		}
	}

	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels) {
		if (SUCCEEDED(D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(dev_.ReleaseAndGetAddressOf())))) {
			featureLevel = lv;
			break;
		}
	}


}

void DirectXCommon::InitializeCommand()
{
	HRESULT result = S_FALSE;

	result = dev_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(cmdAllocator_.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(result));

	result = dev_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator_.Get(), nullptr, IID_PPV_ARGS(cmdList_.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(result));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = dev_->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(cmdQueue_.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(result));
}

void DirectXCommon::CreateSwapChain() {
	HRESULT result = S_FALSE;

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = backBufferWidth_;
	swapchainDesc.Height = backBufferHeight_;
	swapchainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = dxgiFactory_->CreateSwapChainForHwnd(
		cmdQueue_.Get(), win_->GetHWND(), &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)swapChain_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(result));

}

void DirectXCommon::CreateFinalRenderTarget() {
	HRESULT result = S_FALSE;

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = swapChain_->GetDesc(&swcDesc);

	D3D12_DESCRIPTOR_HEAP_DESC heapdesc = {};
	heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapdesc.NodeMask = 0;
	heapdesc.NumDescriptors = 2;
	heapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	result = dev_->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(rtvHeaps_.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(result));

	backBuffers_.resize(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps_->GetCPUDescriptorHandleForHeapStart();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (UINT idx = 0; idx < backBuffers_.size(); ++idx) {
		result = swapChain_->GetBuffer(idx, IID_PPV_ARGS(&backBuffers_[idx]));
		assert(SUCCEEDED(result));
		rtvDesc.Format = backBuffers_[idx]->GetDesc().Format;
		dev_->CreateRenderTargetView(backBuffers_[idx].Get(), &rtvDesc, handle);
		handle.ptr += dev_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
}

void DirectXCommon::CreateDepthBuffer() {

	HRESULT result = S_FALSE;

	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
		backBufferWidth_, backBufferHeight_, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	CD3DX12_HEAP_PROPERTIES depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0);

	CD3DX12_CLEAR_VALUE depthClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	
	result = dev_->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(depthBuffer_.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(result));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	result = dev_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap_.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(result));
	

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	dev_->CreateDepthStencilView(depthBuffer_.Get(),&dsvDesc,dsvHeap_->GetCPUDescriptorHandleForHeapStart());

}

void DirectXCommon::CreateFence()
{
	HRESULT result = S_FALSE;
	result = dev_->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(result));
}
