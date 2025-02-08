﻿#include "WinApp.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"

#include "DirectXMath.h"
using namespace DirectX;
#include "d3dcompiler.h"
#pragma comment(lib,"d3dcompiler.lib")

#include "DirectXTex.h"
#pragma comment(lib,"DirectXTex.lib")

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};

struct TexRGBA {
	unsigned int R, G, B, A;
};

/// <summary>
/// アライメントにそろえたサイズを返す
/// </summary>
/// <param name="size"> 元のサイズ </param>
/// <param name="alignment"> アライメントサイズ </param>
/// <returns> アライメントに揃えたサイズ </returns>
size_t AlignmentedSize(size_t size, size_t alignment) {
	return size + alignment - size % alignment;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WinApp* win = nullptr;
	DirectXCommon* dxCommon = nullptr;

	win = WinApp::GetInstance();
	win->CreateGameWindow();

	dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(win);

	Vertex vertices[] = {
		   {{-1.0f, -1.0f, 0.0f },{0.0f,1.0f}},
		   {{-1.0f,  1.0f, 0.0f },{0.0f,0.0f}},
		   {{ 1.0f, -1.0f, 0.0f },{1.0f,1.0f}},
		   {{ 1.0f,  1.0f, 0.0f },{1.0f,0.0f}},
	};

	HRESULT result = S_FALSE;

#pragma region 頂点バッファーの生成

	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	CD3DX12_RESOURCE_DESC resdesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));

	//D3D12_RESOURCE_DESC resdesc = {};
	//resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	//resdesc.Width = ;
	//resdesc.Height = 1;
	//resdesc.DepthOrArraySize = 1;
	//resdesc.MipLevels = 1;
	//resdesc.Format = DXGI_FORMAT_UNKNOWN;
	//resdesc.SampleDesc.Count = 1;
	//resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	//resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;
	result = dxCommon->GetDevice()->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));

#pragma endregion

	// 頂点情報のコピー
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

#pragma region 頂点バッファービューとインデックスバッファビューの生成

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vertices);
	vbView.StrideInBytes = sizeof(vertices[0]);

	unsigned short indices[] = {
		0,1,2,2,1,3
	};

	ID3D12Resource* idxBuff = nullptr;
	resdesc.Width = sizeof(indices);

	result = dxCommon->GetDevice()->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idxBuff));

	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);


	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

#pragma endregion

#pragma region シェーダーファイルの読み込み

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	result = D3DCompileFromFile(L"Shader/BasicVertexShader.hlsl",
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &vsBlob, &errorBlob);

	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());

			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	result = D3DCompileFromFile(L"Shader/BasicPixelShader.hlsl",
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &psBlob, &errorBlob);
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());

			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

#pragma endregion

	// 頂点レイアウトの作成
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{
			"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{
			"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,
			0,D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		}
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	// シェーダーのセット
	gpipeline.pRootSignature = nullptr;
	gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();


	// サンプルマスクとラスタライザーステートの設定
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	renderTargetBlendDesc.LogicOpEnable = false;
	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	gpipeline.RasterizerState.MultisampleEnable = false;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpipeline.RasterizerState.DepthClipEnable = true;

	// 入力レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// レンダーターゲットの設定
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;

	// アンチエイリアシングのためのサンプル数設定
	gpipeline.SampleDesc.Count = 1;
	gpipeline.SampleDesc.Quality = 0;

	// ルートシグネチャの生成
	ID3D12RootSignature* rootSignature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// ディスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE descTblRange[2] = {};
	descTblRange[0].NumDescriptors = 1;
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange[0].BaseShaderRegister = 0;
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descTblRange[1].NumDescriptors = 1;
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange[1].BaseShaderRegister = 0;
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ルートパラメーターの作成
	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam.DescriptorTable.pDescriptorRanges = descTblRange;
	rootparam.DescriptorTable.NumDescriptorRanges = 2;
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootSignatureDesc.pParameters = &rootparam;
	rootSignatureDesc.NumParameters = 1;

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;


	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	result = dxCommon->GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	rootSigBlob->Release();
	gpipeline.pRootSignature = rootSignature;

	// グラフィックスパイプラインステートオブジェクトの生成
	ID3D12PipelineState* pipelineState = nullptr;
	result = dxCommon->GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelineState));

	TexMetadata metadata = {};
	ScratchImage scratchImg = {};

	result = LoadFromWICFile(L"Resources/textest.png", WIC_FLAGS_NONE, &metadata, scratchImg);
	auto img = scratchImg.GetImage(0, 0, 0);

	// テスクチャデータの作成
	std::vector<TexRGBA> textureData(256 * 256);
	for (auto& rgba : textureData) {
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
		rgba.A = 255;
	}

#pragma region アップロード用リソースの作成

	// 中間バッファーとしてのアップロードヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProp = {};
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeapProp.CreationNodeMask = 0;
	uploadHeapProp.VisibleNodeMask = 0;

	// リソース設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	
	// 中間バッファーの作成
	ID3D12Resource* uploadBuff = nullptr;
	result = dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,&resDesc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&uploadBuff));

#pragma endregion

	D3D12_HEAP_PROPERTIES texheapProp = {};
	texheapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	texheapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texheapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texheapProp.CreationNodeMask = 0;
	texheapProp.VisibleNodeMask = 0;

	// リソース設定
	resDesc.Format = metadata.format;
	resDesc.Width = static_cast<UINT>(metadata.width);
	resDesc.Height = static_cast<UINT>(metadata.height);
	resDesc.DepthOrArraySize = static_cast<UINT>(metadata.arraySize);
	resDesc.MipLevels = static_cast<UINT>(metadata.mipLevels);
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	ID3D12Resource* texBuff = nullptr;
	result = dxCommon->GetDevice()->CreateCommittedResource(&texheapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texBuff));

	// 画像データーをGPUに転送する
	result = texBuff->WriteToSubresource(0, nullptr, img->pixels, 
		static_cast<UINT>(img->rowPitch), static_cast<UINT>(img->slicePitch));

	XMMATRIX worldMat = XMMatrixRotationY(XM_PIDIV4);
	XMFLOAT3 eye(0, 0, -5);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);

	// アスペクト比
	float aspectRatio = static_cast<float>(WinApp::windowWidth) / static_cast<float>(WinApp::windowHeight);

	// ビュー行列
	XMMATRIX viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	// プロジェクション行列
	XMMATRIX projMat = XMMatrixPerspectiveFovLH(XM_PIDIV2, aspectRatio, 1.0f, 10.0f);

	// 定数バッファの作成
	ID3D12Resource* constBuff = nullptr;

	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(worldMat) + 0xff) & ~0xff);

	dxCommon->GetDevice()->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuff));

	// マップによる定数のコピー
	XMMATRIX* mapMatrix;
	result = constBuff->Map(0, nullptr, (void**)&mapMatrix);
	*mapMatrix = worldMat * viewMat * projMat;


	// アップロードリソースへのマップ
	uint8_t* mapforImg = nullptr;
	result = uploadBuff->Map(0, nullptr, (void**)&mapforImg);
	std::copy_n(img->pixels, img->slicePitch, mapforImg);
	uploadBuff->Unmap(0, nullptr);

	auto srcAddress = img->pixels;
	auto rowpitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	for (int y = 0; y < img->height; y++) {
		std::copy_n(srcAddress, rowpitch, mapforImg);
		srcAddress += img->rowPitch;
		mapforImg += rowpitch;
	}

	D3D12_TEXTURE_COPY_LOCATION src = {};

	// コピー元(アップロード側)設定
	src.pResource = uploadBuff;
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = static_cast<UINT>(metadata.width);
	src.PlacedFootprint.Footprint.Height = static_cast<UINT>(metadata.height);
	src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metadata.depth);
	src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
	src.PlacedFootprint.Footprint.Format = img->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = texBuff;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	{
		dxCommon->GetCmdList()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = texBuff;
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		dxCommon->GetCmdList()->ResourceBarrier(1, &BarrierDesc);
		dxCommon->GetCmdList()->Close();

		ID3D12CommandList* cmdLists[] = { dxCommon->GetCmdList() };
		dxCommon->GetCmdQueue()->ExecuteCommandLists(1, cmdLists);

		ID3D12Fence* fence = dxCommon->GetFence();
		UINT64 fenceVal = dxCommon->GetFenceVal();

		dxCommon->GetCmdQueue()->Signal(fence, ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		dxCommon->GetAllocator()->Reset();
		dxCommon->GetCmdList()->Reset(dxCommon->GetAllocator(), nullptr);
	}

#pragma region シェーダーリソースビュー

	// ディスクリプタヒープの作成
	ID3D12DescriptorHeap* basicDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};

	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 2;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	result = dxCommon->GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	// シェーダーリソースビューの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// ディスクリプタの先頭ハンドルを取得しておく
	auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
	// シェーダーリソースビューの作成
	dxCommon->GetDevice()->CreateShaderResourceView(texBuff, &srvDesc, basicHeapHandle);
	basicHeapHandle.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(constBuff->GetDesc().Width);

	// 定数バッファビューの作成
	dxCommon->GetDevice()->CreateConstantBufferView(&cbvDesc, basicHeapHandle);

#pragma endregion

	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	imguiManager->Initialize(win, dxCommon);

	float angle = 0.0f;

	while (true) {
		if (win->ProcessMessage()) {
			break;
		}

		imguiManager->Begin();		

		
		angle += 0.01f;
		worldMat = XMMatrixRotationY(angle);
		*mapMatrix = worldMat * viewMat * projMat;


		imguiManager->End();

		dxCommon->BeginDraw();
		imguiManager->Draw();

#pragma region 四角形の描画命令

		dxCommon->GetCmdList()->SetPipelineState(pipelineState);

		dxCommon->GetCmdList()->SetGraphicsRootSignature(rootSignature);

		dxCommon->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dxCommon->GetCmdList()->IASetVertexBuffers(0, 1, &vbView);
		dxCommon->GetCmdList()->IASetIndexBuffer(&ibView);

		dxCommon->GetCmdList()->SetGraphicsRootSignature(rootSignature);
		dxCommon->GetCmdList()->SetDescriptorHeaps(1, &basicDescHeap);
		dxCommon->GetCmdList()->SetGraphicsRootDescriptorTable(0,
			basicDescHeap->GetGPUDescriptorHandleForHeapStart());
		
		dxCommon->GetCmdList()->DrawIndexedInstanced(6, 1, 0, 0, 0);


#pragma endregion
		
		dxCommon->EndDraw();
	}

	imguiManager->Finalize();
	win->Terminate();

	return 0;
}