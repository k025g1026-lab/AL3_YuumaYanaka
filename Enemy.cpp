// [Enemy.cpp]
#include "Enemy.h"
#include <cmath>

using namespace KamataEngine;

void Enemy::Initialize(uint32_t textureHandle, const Vector2& position, float minX, float maxX) {
	sprite_ = Sprite::Create(textureHandle, {0.0f, 0.0f});
	position_ = position;
	minX_ = minX;
	maxX_ = maxX;
	hp_ = kMaxHp;
	velocity_ = {};
	patrolDir_ = 1;
}

AABB2 Enemy::GetAABB() const {
	AABB2 aabb;
	aabb.min = position_;
	aabb.max = {position_.x + size_.x, position_.y + size_.y};
	return aabb;
}

void Enemy::OnCinchHit(int damage) {
	if (hp_ <= 0) {
		return;
	}
	hp_ -= damage;
	if (hp_ < 0) {
		hp_ = 0;
	}
	hitFlash_ = 8;
}

void Enemy::ApplyKnockback(const Vector2& velocity) { velocity_ = velocity; }

void Enemy::Update() {
	UpdateStitchCoolDown();
	if (hp_ <= 0) {
		return;
	}
	if (hitFlash_ > 0) {
		--hitFlash_;
	}

	if (std::abs(velocity_.x) < 1.0f) {
		position_.x += static_cast<float>(patrolDir_) * kPatrolSpeed;
		if (position_.x <= minX_) {
			position_.x = minX_;
			patrolDir_ = 1;
		}
		if (position_.x >= maxX_) {
			position_.x = maxX_;
			patrolDir_ = -1;
		}
		velocity_.x *= 0.8f;
	} else {
		position_.x += velocity_.x;
		velocity_.x *= 0.90f;
	}

	velocity_.y += kGravity;
	position_.y += velocity_.y;
	if (position_.y + size_.y >= kGroundY) {
		position_.y = kGroundY - size_.y;
		if (velocity_.y > 0.0f) {
			velocity_.y = 0.0f;
		}
	}
}

void Enemy::Draw(const Vector2& camera) {
	if (!sprite_ || hp_ <= 0) {
		return;
	}
	Vector4 color = {0.45f, 0.9f, 0.4f, 1.0f};
	if (hitFlash_ > 0) {
		color = {1.0f, 1.0f, 1.0f, 1.0f};
	}
	sprite_->SetColor(color);
	sprite_->SetPosition({position_.x - camera.x, position_.y - camera.y});
	sprite_->SetSize(size_);
	sprite_->Draw();
}