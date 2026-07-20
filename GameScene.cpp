// [GameScene.cpp]
#include "GameScene.h"

using namespace KamataEngine;

GameScene::~GameScene() {
	delete player_;
	player_ = nullptr;
	delete debugCamera_;
	debugCamera_ = nullptr;
}

void GameScene::Initialize() {
	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// 3Dモデルの生成（自キャラ用）
	Model* model = Model::Create();
	uint32_t textureHandle = TextureManager::Load("mario.jpg");

	// 自キャラの生成
	player_ = new Player();
	// 自キャラの初期化
	player_->Initialize(model, textureHandle, &debugCamera_->GetCamera());
}

void GameScene::Update() {
	// デバッグカメラの更新
	debugCamera_->Update();

	// 自キャラの更新
	player_->Update();
}

void GameScene::Draw() {
	// 3Dモデル描画前処理
	Model::PreDraw();

	// 自キャラの描画
	player_->Draw();

	// 3Dモデル描画後処理
	Model::PostDraw();
}