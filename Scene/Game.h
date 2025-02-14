#pragma once
#include "DirectXCommon.h"
#include "Input.h"

class Game {
public:
	Game();
	~Game();
	void Initialize();
	void Update();
	void Draw();
private:

	DirectXCommon *dxCommon_ = nullptr;
	Input* input_ = nullptr;

};

