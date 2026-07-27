// [DeathParticles.cpp]
#define NOMINMAX
#include "DeathParticles.h"
#include "Matrix.h"
#include <algorithm>
#include <cassert>
#include <cmath>

void DeathParticles::Initialize(KamataEngine::Model* model, const KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NULLポインタチェック
	assert(model);
	assert(camera);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
	}

	// 色変更オブジェクトの初期化
	objectColor_.Initialize();
	// RGBA順で初期化（R=1, G=1, B=1, A=1）
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};
}

void DeathParticles::Update() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// カウンターを1フレーム分の秒数進める
	counter_ += 1.0f / 60.0f;

	// 存続時間の上限に達したら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		// 終了扱いにする
		isFinished_ = true;
	}

	// フェードアウト（アルファを徐々に下げる）
	// color_.x = R, color_.y = G, color_.z = B, color_.w = A
	color_.w = std::clamp(1.0f - (counter_ / kDuration), 0.0f, 1.0f);
	// 色変更オブジェクトに色の数値を設定する
	objectColor_.SetColor(color_);

	// 8方向に移動させる
	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// 基本となる速度ベクトル
		KamataEngine::Vector3 velocity = {kSpeed, 0.0f, 0.0f};
		// 回転角を計算する
		float angle = kAngleUnit * i;
		// Z軸まわりに回転（手動計算）
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		KamataEngine::Vector3 rotatedVelocity = {velocity.x * cosA - velocity.y * sinA, velocity.x * sinA + velocity.y * cosA, velocity.z};
		// 移動処理
		worldTransforms_[i].translation_.x += rotatedVelocity.x;
		worldTransforms_[i].translation_.y += rotatedVelocity.y;
		worldTransforms_[i].translation_.z += rotatedVelocity.z;
	}

	// ワールド変換の更新
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);
		worldTransform.TransferMatrix();
	}
}

void DeathParticles::Draw() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// モデルの描画（色変更オブジェクトを渡す）
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
}