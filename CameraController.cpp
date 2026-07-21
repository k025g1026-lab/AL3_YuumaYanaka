// [CameraController.cpp]
#define NOMINMAX
#include "CameraController.h"
#include "Player.h"
#include <algorithm>

KamataEngine::Vector3 Lerp(const KamataEngine::Vector3& a, const KamataEngine::Vector3& b, float t) { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t}; }

void CameraController::Initialize() {
	// カメラの初期化
	camera_.Initialize();
}

void CameraController::Update() {
	if (target_) {
		// 追従対象のワールドトランスフォームを参照
		const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();
		// 追従対象の速度を取得
		const KamataEngine::Vector3& targetVelocity = target_->GetVelocity();
		// 追従対象とオフセットと速度からカメラの目標座標を計算
		targetPosition_.x = targetWorldTransform.translation_.x + targetOffset_.x + targetVelocity.x * kVelocityBias;
		targetPosition_.y = targetWorldTransform.translation_.y + targetOffset_.y + targetVelocity.y * kVelocityBias;
		targetPosition_.z = targetWorldTransform.translation_.z + targetOffset_.z + targetVelocity.z * kVelocityBias;

		// 座標補間によりゆったり追従
		camera_.translation_ = Lerp(camera_.translation_, targetPosition_, kInterpolationRate);

		// 追従対象を画面内に収める補正
		float playerX = targetWorldTransform.translation_.x;
		float playerY = targetWorldTransform.translation_.y;
		camera_.translation_.x = std::max(camera_.translation_.x, playerX + kMargin.left);
		camera_.translation_.x = std::min(camera_.translation_.x, playerX + kMargin.right);
		camera_.translation_.y = std::max(camera_.translation_.y, playerY + kMargin.bottom);
		camera_.translation_.y = std::min(camera_.translation_.y, playerY + kMargin.top);

		// 移動範囲制限
		camera_.translation_.x = std::clamp(camera_.translation_.x, movableArea_.left, movableArea_.right);
		camera_.translation_.y = std::clamp(camera_.translation_.y, movableArea_.bottom, movableArea_.top);

		// 行列を更新
		camera_.UpdateMatrix();
	}
}

void CameraController::Reset() {
	if (target_) {
		// 追従対象のワールドトランスフォームを参照
		const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();
		// 追従対象の速度を取得
		const KamataEngine::Vector3& targetVelocity = target_->GetVelocity();
		// 追従対象とオフセットと速度からカメラの目標座標を計算
		targetPosition_.x = targetWorldTransform.translation_.x + targetOffset_.x + targetVelocity.x * kVelocityBias;
		targetPosition_.y = targetWorldTransform.translation_.y + targetOffset_.y + targetVelocity.y * kVelocityBias;
		targetPosition_.z = targetWorldTransform.translation_.z + targetOffset_.z + targetVelocity.z * kVelocityBias;
		camera_.translation_ = targetPosition_;

		// 移動範囲制限
		camera_.translation_.x = std::clamp(camera_.translation_.x, movableArea_.left, movableArea_.right);
		camera_.translation_.y = std::clamp(camera_.translation_.y, movableArea_.bottom, movableArea_.top);

		// 行列を更新
		camera_.UpdateMatrix();
	}
}