#pragma once
#include "fbxsdk.h"
#include "vector"
#include "d3d12.h"
#include "wrl.h"
#include "DirectXMath.h"



using Microsoft::WRL::ComPtr;
using DirectX::XMFLOAT3;

class DirectXCommon;

struct Vertex {
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 uv;
};

class FBXLoader{
public:

	static FBXLoader* GetIsntance();

	void Initialize(DirectXCommon*dxCommon);
	bool LoadFBX(const std::string& fileName);
	void CreateBuffers();
	void LoadShaders();
	void CretaPipelineState();
	void Render();
	void Release();

	const std::vector<Vertex>& GetVertices() const;

private:

	void ParseFBX(FbxNode* node);

private:

	FBXLoader() = default;
	~FBXLoader() = default;
	FBXLoader(const FBXLoader& obj) = delete;
	FBXLoader operator==(const FBXLoader& obj) = delete;

private:

	DirectXCommon* dxCommon_ = nullptr;

	FbxManager* fbxManager_ = nullptr;
	FbxScene* fbxScene_ = nullptr;


	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indices_;

	ComPtr<ID3D12Resource> vertexBuffer_;
	ComPtr<ID3D12Resource> indexBuffer_;

	D3D12_VERTEX_BUFFER_VIEW vbView_;
	D3D12_INDEX_BUFFER_VIEW ibView_;

	ComPtr<ID3DBlob> vertexShader_;
	ComPtr<ID3DBlob> pixelShader_;
	ComPtr<ID3D12PipelineState> pipelineState_;
	ComPtr<ID3D12RootSignature> rootSignature_;
};

