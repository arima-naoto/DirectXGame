#include "Game.h"
#include "imgui.h"

Game::Game(){
}

Game::~Game(){
}

void Game::Initialize(){
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
}

void Game::Update(){
}

void Game::Draw(){
}
