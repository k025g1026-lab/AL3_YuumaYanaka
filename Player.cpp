// [Player.cpp]
#include "Player.h"
#include "Matrix.h"
#include <cassert>

void Player::Initialize(KamataEngine::Model* model, const KamataEngine::Camera* camera) {
	// NULLポインタチェック
	assert(model);
	assert(camera);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
}

void Player::Update() {
	// アフィン変換行列を計算してメンバ変数に代入
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	// 定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Player::Draw() {
	// 自キャラの描画
	model_->Draw(worldTransform_, *camera_);
}