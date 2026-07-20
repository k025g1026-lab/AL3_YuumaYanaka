// [Player.cpp]
#define NOMINMAX
#include "Player.h"
#include "Matrix.h"
#include <algorithm>
#include <cassert>
#include <numbers>

void Player::Initialize(KamataEngine::Model* model, const KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NULLポインタチェック
	assert(model);
	assert(camera);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 初期回転
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	turnFirstRotationY_ = worldTransform_.rotation_.y;
}

void Player::Update() {
	// 接地状態
	if (onGround_) {
		// 左右移動操作
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT) || KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT)) {
			if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT)) {
				if (lrDirection_ != LRDirection::kRight) {
					velocity_.x *= (1.0f - kAttenuation);
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
				lrDirection_ = LRDirection::kRight;
				velocity_.x += kAcceleration;
			} else if (KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT)) {
				if (lrDirection_ != LRDirection::kLeft) {
					velocity_.x *= (1.0f - kAttenuation);
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
				lrDirection_ = LRDirection::kLeft;
				velocity_.x -= kAcceleration;
			}
		} else {
			velocity_.x *= (1.0f - kAttenuation);
		}

		// ジャンプ入力
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_.y += kJumpAcceleration;
			onGround_ = false;
		}
	} else {
		// 落下速度
		velocity_.y += -kGravityAcceleration;
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}

	// 速度制限（左右）
	velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

	// 移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	// 地面との当たり判定（仮）
	bool landing = false;
	if (velocity_.y < 0) {
		if (worldTransform_.translation_.y <= 1.0f) {
			landing = true;
		}
	}

	// 接地 / 空中切り替え
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		}
	} else {
		// 着地
		if (landing) {
			// めり込み排斥
			worldTransform_.translation_.y = 1.0f;
			// 摩擦で横方向速度が減衰する
			velocity_.x *= (1.0f - kAttenuation);
			// 下方向速度をリセット
			velocity_.y = 0.0f;
			// 接地状態に移行
			onGround_ = true;
		}
	}

	// 回転制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		float t = 1.0f - (turnTimer_ / kTimeTurn);
		worldTransform_.rotation_.y = (1.0f - t) * turnFirstRotationY_ + t * destinationRotationY;
	}

	// アフィン変換行列を計算してメンバ変数に代入
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	// 定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Player::Draw() {
	// 自キャラの描画
	model_->Draw(worldTransform_, *camera_);
}