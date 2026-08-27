// [Player.cpp]
#define NOMINMAX
#include "Player.h"
#include "Enemy.h"
#include "MapChipField.h"
#include "Matrix.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

void Player::Initialize(Model* model, const Camera* camera, const Vector3& position) {
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	turnFirstRotationY_ = worldTransform_.rotation_.y;
}

void Player::InputMove() {
	const bool pressRight = Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_D);
	const bool pressLeft = Input::GetInstance()->PushKey(DIK_LEFT) || Input::GetInstance()->PushKey(DIK_A);
	const bool pressJump = Input::GetInstance()->PushKey(DIK_UP) || Input::GetInstance()->PushKey(DIK_SPACE) || Input::GetInstance()->PushKey(DIK_W);

	// 左右移動（地上・空中で同じ加速・同じ上限）
	if (pressRight || pressLeft) {
		float accelerationX = 0.0f;
		if (pressRight) {
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			accelerationX += kAcceleration;
			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		} else if (pressLeft) {
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			accelerationX -= kAcceleration;
			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		}
		velocity_.x += accelerationX;
	} else {
		velocity_.x *= (1.0f - kAttenuation);
	}

	velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

	if (onGround_) {
		if (pressJump) {
			velocity_.y += kJumpAcceleration;
			onGround_ = false;
		}
	} else {
		velocity_.y -= kGravityAcceleration;
		if (velocity_.y < -kLimitFallSpeed) {
			velocity_.y = -kLimitFallSpeed;
		}
	}
}

void Player::MapCollision(CollisionMapInfo& info) {
	MapCollisionUp(info);
	MapCollisionDown(info);
	MapCollisionRight(info);
	MapCollisionLeft(info);
}

void Player::MapCollisionUp(CollisionMapInfo& info) {
	if (info.movement.y <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		Vector3 moved = {worldTransform_.translation_.x + info.movement.x, worldTransform_.translation_.y + info.movement.y, worldTransform_.translation_.z + info.movement.z};
		positionsNew[i] = CornerPosition(moved, static_cast<Corner>(i));
	}

	bool hit = false;
	MapChipField::IndexSet indexSet{};

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		hit = true;
	}
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kRightTop));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movement.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
			info.ceiling = true;
		}
	}
}

void Player::MapCollisionDown(CollisionMapInfo& info) {
	if (info.movement.y >= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		Vector3 moved = {worldTransform_.translation_.x + info.movement.x, worldTransform_.translation_.y + info.movement.y, worldTransform_.translation_.z + info.movement.z};
		positionsNew[i] = CornerPosition(moved, static_cast<Corner>(i));
	}

	bool hit = false;
	MapChipField::IndexSet indexSet{};
	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kRightBottom));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movement.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
			info.landing = true;
		}
	}
}

void Player::MapCollisionRight(CollisionMapInfo& info) {
	if (info.movement.x <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		Vector3 moved = {worldTransform_.translation_.x + info.movement.x, worldTransform_.translation_.y + info.movement.y, worldTransform_.translation_.z + info.movement.z};
		positionsNew[i] = CornerPosition(moved, static_cast<Corner>(i));
	}

	bool hit = false;
	MapChipField::IndexSet indexSet{};
	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kRightTop));
		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movement.x = std::min(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
			info.hitWall = true;
		}
	}
}

void Player::MapCollisionLeft(CollisionMapInfo& info) {
	if (info.movement.x >= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		Vector3 moved = {worldTransform_.translation_.x + info.movement.x, worldTransform_.translation_.y + info.movement.y, worldTransform_.translation_.z + info.movement.z};
		positionsNew[i] = CornerPosition(moved, static_cast<Corner>(i));
	}

	bool hit = false;
	MapChipField::IndexSet indexSet{};
	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));
		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movement.x = std::max(0.0f, rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank));
			info.hitWall = true;
		}
	}
}

void Player::MoveAfterCollision(const CollisionMapInfo& info) {
	worldTransform_.translation_.x += info.movement.x;
	worldTransform_.translation_.y += info.movement.y;
	worldTransform_.translation_.z += info.movement.z;
}

void Player::OnCollisionCeiling(const CollisionMapInfo& info) {
	if (info.ceiling) {
		velocity_.y = 0;
	}
}

void Player::OnCollisionWall(const CollisionMapInfo& info) {
	if (info.hitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::SwitchOnGround(const CollisionMapInfo& info) {
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			std::array<Vector3, kNumCorner> positionsNew;
			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_, static_cast<Corner>(i));
			}

			bool hit = false;
			MapChipField::IndexSet indexSet{};

			Vector3 leftBottom = positionsNew[kLeftBottom];
			leftBottom.y -= kBlank;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);
			if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
				hit = true;
			}

			Vector3 rightBottom = positionsNew[kRightBottom];
			rightBottom.y -= kBlank;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);
			if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
				hit = true;
			}

			if (!hit) {
				onGround_ = false;
			}
		}
	} else {
		if (info.landing) {
			onGround_ = true;
			velocity_.y = 0.0f;
		}
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0},
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0},
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0},
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0},
	};
	return {center.x + offsetTable[static_cast<uint32_t>(corner)].x, center.y + offsetTable[static_cast<uint32_t>(corner)].y, center.z + offsetTable[static_cast<uint32_t>(corner)].z};
}

void Player::Update() {
	InputMove();

	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		float t = 1.0f - (turnTimer_ / kTimeTurn);
		worldTransform_.rotation_.y = (1.0f - t) * turnFirstRotationY_ + t * destinationRotationY;
	}

	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.movement = velocity_;
	MapCollision(collisionMapInfo);
	MoveAfterCollision(collisionMapInfo);
	OnCollisionCeiling(collisionMapInfo);
	OnCollisionWall(collisionMapInfo);
	SwitchOnGround(collisionMapInfo);

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::Draw() { model_->Draw(worldTransform_, *camera_); }

Vector3 Player::GetWorldPosition() {
	Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};
	return aabb;
}

void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;
	isDead_ = true;
}