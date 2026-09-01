// [Boss.cpp]
#include "Boss.h"
#include <cmath>

using namespace KamataEngine;

void Boss::Initialize(uint32_t textureHandle, const Vector2& position, const Vector4& color, float minX, float maxX) {
	sprite_ = Sprite::Create(textureHandle, {0.0f, 0.0f});
	position_ = position;
	color_ = color;
	minX_ = minX;
	maxX_ = maxX;
	hp_ = kMaxHp;
	velocity_ = {};
	patrolDir_ = 1;
}

AABB2 Boss::GetAABB() const {
	AABB2 aabb;
	aabb.min = position_;
	aabb.max = {position_.x + size_.x, position_.y + size_.y};
	return aabb;
}

void Boss::OnCinchHit(int damage) {
	if (hp_ <= 0) {
		return;
	}
	hp_ -= damage;
	if (hp_ < 0) {
		hp_ = 0;
	}
	hitFlash_ = 10;
}

void Boss::ApplyKnockback(const Vector2& velocity) { velocity_ = velocity; }

void Boss::Update() {
	UpdateStitchCoolDown();

	if (hitFlash_ > 0) {
		--hitFlash_;
	}

	// ノックバックが小さいときだけ左右移動
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
		velocity_.x *= 0.92f;
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

void Boss::Draw() {
	if (!sprite_ || hp_ <= 0) {
		return;
	}
	Vector4 color = color_;
	if (hitFlash_ > 0) {
		color = {1.0f, 1.0f, 1.0f, 1.0f};
	}
	sprite_->SetColor(color);
	sprite_->SetPosition(position_);
	sprite_->SetSize(size_);
	sprite_->Draw();
}