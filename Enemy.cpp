// [Enemy.cpp]
#define NOMINMAX
#include "Enemy.h"
#include "MapChipField.h"
#include "Matrix.h"
#include "Player.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

using namespace KamataEngine;

void Enemy::Initialize(Model* model, const Camera* camera, const Vector3& position) {
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 初期は左向き
	velocity_ = {-0.05f, 0.0f, 0.0f};
	onGround_ = false;
	UpdateFacing();
}

void Enemy::UpdateFacing() {
	if (velocity_.x < 0.0f) {
		worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;
	} else {
		worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	}
}

void Enemy::CheckMapWall() {
	if (!mapChipField_) {
		return;
	}

	Vector3 checkPos = worldTransform_.translation_;
	if (velocity_.x >= 0.0f) {
		checkPos.x += kWidth / 2.0f;
	} else {
		checkPos.x -= kWidth / 2.0f;
	}

	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(checkPos);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) != MapChipType::kBlock) {
		return;
	}

	MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
	if (velocity_.x > 0.0f) {
		worldTransform_.translation_.x = rect.left - kWidth / 2.0f;
	} else {
		worldTransform_.translation_.x = rect.right + kWidth / 2.0f;
	}

	velocity_.x *= -1.0f;
	UpdateFacing();
}

void Enemy::CheckMapGround() {
	if (!mapChipField_) {
		return;
	}

	Vector3 leftBottom = worldTransform_.translation_;
	leftBottom.x -= kWidth / 2.0f;
	leftBottom.y -= kHeight / 2.0f + kBlank;

	Vector3 rightBottom = worldTransform_.translation_;
	rightBottom.x += kWidth / 2.0f;
	rightBottom.y -= kHeight / 2.0f + kBlank;

	MapChipField::IndexSet leftIndex = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);
	MapChipField::IndexSet rightIndex = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);

	MapChipType leftType = mapChipField_->GetMapChipTypeByIndex(leftIndex.xIndex, leftIndex.yIndex);
	MapChipType rightType = mapChipField_->GetMapChipTypeByIndex(rightIndex.xIndex, rightIndex.yIndex);

	bool hit = (leftType == MapChipType::kBlock) || (rightType == MapChipType::kBlock);

	if (hit) {
		MapChipField::IndexSet indexSet = (leftType == MapChipType::kBlock) ? leftIndex : rightIndex;
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

		worldTransform_.translation_.y = rect.top + kHeight / 2.0f;
		velocity_.y = 0.0f;
		onGround_ = true;
	} else {
		onGround_ = false;
	}
}

void Enemy::Update() {
	walkTimer_ += 1.0f / 60.0f;

	// 重力
	if (!onGround_) {
		velocity_.y -= kGravityAcceleration;
		if (velocity_.y < -kLimitFallSpeed) {
			velocity_.y = -kLimitFallSpeed;
		}
	}

	// 移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	CheckMapGround();
	CheckMapWall();

	float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / 1.0f);
	float degree = 10.0f * 3.14159265f / 180.0f;
	worldTransform_.rotation_.x = degree * param;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Enemy::Draw() { model_->Draw(worldTransform_, *camera_); }

Vector3 Enemy::GetWorldPosition() {
	Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};
	return aabb;
}

void Enemy::OnCollision(const Player* player) { (void)player; }

void Enemy::OnCollisionEnemy(Enemy* other) {
	if (!other) {
		return;
	}

	float deltaX = worldTransform_.translation_.x - other->worldTransform_.translation_.x;
	if (deltaX * velocity_.x < 0.0f) {
		velocity_.x *= -1.0f;
		UpdateFacing();
	}
}