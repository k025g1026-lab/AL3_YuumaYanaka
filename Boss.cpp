// [Boss.cpp]
#include "Boss.h"

using namespace KamataEngine;

void Boss::Initialize(uint32_t textureHandle, const Vector2& position, const Vector4& color) {
	sprite_ = Sprite::Create(textureHandle, {0.0f, 0.0f});
	position_ = position;
	color_ = color;
	hp_ = 6;
	velocity_ = {};
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

void Boss::ApplyKnockback(const Vector2& velocity) {
	velocity_ = velocity;
}

void Boss::Update() {
	UpdateStitchCoolDown();

	if (hitFlash_ > 0) {
		--hitFlash_;
	}

	velocity_.y += kGravity;
	position_.x += velocity_.x;
	position_.y += velocity_.y;
	velocity_.x *= 0.92f;

	if (position_.y + size_.y >= kGroundY) {
		position_.y = kGroundY - size_.y;
		if (velocity_.y > 0.0f) {
			velocity_.y = 0.0f;
		}
	}
	if (position_.x < 80.0f) {
		position_.x = 80.0f;
	}
	if (position_.x > 1280.0f - 80.0f - size_.x) {
		position_.x = 1280.0f - 80.0f - size_.x;
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
