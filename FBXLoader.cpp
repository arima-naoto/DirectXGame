#include "FBXLoader.h"
#include "DirectXCommon.h"
#include "cassert"
#include "fstream"
#include "d3dcompiler.h"

#pragma comment(lib,"d3dcompiler.lib")

FBXLoader* FBXLoader::GetIsntance()
{
	static FBXLoader instance;
	return &instance;
}

void FBXLoader::Initialize(DirectXCommon* dxCommon){

	dxCommon_ = dxCommon;

	fbxManager_ = FbxManager::Create();
	if (!fbxManager_) {
		assert(false && "Failed to create FBX Manager.");
	}

	FbxIOSettings* ios = FbxIOSettings::Create(fbxManager_, IOSROOT);
	fbxManager_->SetIOSettings(ios);


}

bool FBXLoader::LoadFBX(const std::string& fileName){
	if (!fbxManager_) {
		assert(false && "FBXManager is not initialized.");
		return false;
	}

	fbxScene_ = FbxScene::Create(fbxManager_, "MyScene");
	if (!fbxScene_) {
		assert(false && "Failed to create FBX Scene.");
		return false;
	}

	FbxImporter* importer = FbxImporter::Create(fbxManager_, "");
	if (!importer->Initialize(fileName.c_str(), -1, fbxManager_->GetIOSettings())) {
		importer->Destroy();
		return false;
	}

	if (!importer->Import(fbxScene_)) {
		importer->Destroy();
		fbxScene_->Destroy();
		fbxScene_ = nullptr;
		return false;
	}

	FbxNode* rootNode = fbxScene_->GetRootNode();
	if (rootNode) {
		ParseFBX(rootNode);
	}

	importer->Destroy();

	return true;
}

void FBXLoader::CreateBuffers() {
	HRESULT result = S_FALSE;

	size_t vertexBufferSize = sizeof(Vertex) * vertices_.size();

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = vertexBufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = dxCommon_->GetDevice()->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer_));
	assert(SUCCEEDED(result));

	void* mappedData = nullptr;
	vertexBuffer_->Map(0, nullptr, &mappedData);
	memcpy(mappedData, vertices_.data(), vertexBufferSize);
	vertexBuffer_->Unmap(0, nullptr);

	vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vbView_.SizeInBytes = static_cast<UINT>(vertexBufferSize);
	vbView_.StrideInBytes = sizeof(Vertex);

}
void FBXLoader::LoadShaders(){
	HRESULT result = S_FALSE;

	ComPtr<ID3DBlob> errorBlob;

	std::wstring shaderName[2] = {
		L"FBXVertexShader.hlsl",
		L"FBXPixelShader.hlsl"
	};

	result = D3DCompileFromFile(shaderName[0].c_str(), nullptr, nullptr, "main", "vs_5_1",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader_, &errorBlob);
	assert(SUCCEEDED(result));

	result = D3DCompileFromFile(shaderName[1].c_str(), nullptr, nullptr, "main", "ps_5_1",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader_, &errorBlob);
	assert(SUCCEEDED(result));

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob>signature;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	dxCommon_->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));

}

void FBXLoader::CretaPipelineState(){
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature_.Get();
	psoDesc.VS = { vertexShader_->GetBufferPointer(),vertexShader_->GetBufferSize() };
	psoDesc.PS = { pixelShader_->GetBufferPointer(),pixelShader_->GetBufferSize() };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.NumRenderTargets = 1;

	dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
}

void FBXLoader::Render(){
	dxCommon_->GetCmdList()->IASetVertexBuffers(0, 1, &vbView_);
	dxCommon_->GetCmdList()->DrawInstanced(static_cast<UINT>(vertices_.size()), 1, 0, 0);
}

void FBXLoader::Release(){
	if (fbxManager_) {
		fbxManager_->Destroy();
		fbxManager_ = nullptr;
	}
}
const std::vector<Vertex>& FBXLoader::GetVertices() const {
	return vertices_;
};


void FBXLoader::ParseFBX(FbxNode* node){
	if (!node) return;

	FbxMesh* mesh = node->GetMesh();
	if (mesh) {
		int vertexCount = mesh->GetControlPointsCount();
		FbxVector4* controlPoints = mesh->GetControlPoints();

		for (int i = 0; i < vertexCount; i++) {
			Vertex vertex;
			vertex.position = XMFLOAT3(
				static_cast<float>(controlPoints[i][0]),
				static_cast<float>(controlPoints[i][1]),
				static_cast<float>(controlPoints[i][2])
			);
			vertices_.push_back(vertex);
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++) {
		ParseFBX(node->GetChild(i));
	}
}
