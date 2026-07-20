// [Skydome.cpp]
#include "Skydome.h"

void Skydome::Initialize(KamataEngine::Model* model) {
	// 引数として受け取ったモデルを記録
	model_ = model;

	// ワールド変換の初期化
	worldTransform_.Initialize();
}

void Skydome::Update() {
	// 天球の更新処理（必要に応じて追記）
}

void Skydome::Draw(const KamataEngine::Camera& camera) {
	// 天球の描画
	model_->Draw(worldTransform_, camera);
}