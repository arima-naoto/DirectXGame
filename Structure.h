#pragma once
#include "DirectXMath.h"
using namespace DirectX;
#include "d3dcompiler.h"
#pragma comment(lib,"d3dcompiler.lib")

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};

struct TexRGBA {
	unsigned int R, G, B, A;
};
