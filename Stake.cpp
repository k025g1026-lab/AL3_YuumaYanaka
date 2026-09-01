// [Stake.cpp]
#include "Stake.h"

using namespace KamataEngine;

void Stake::Initialize(uint32_t textureHandle, const Vector2& position) {
	sprite_ = Sprite::Create(textureHandle, {0.0f, 0.0f});
	position_ = position;
}

AABB2 Stake::GetAABB() const {
	AABB2 aabb;
	aabb.min = position_;
	aabb.max = {position_.x + size_.x, position_.y + size_.y};
	return aabb;
}

void Stake::Update() { UpdateStitchCoolDown(); }

void Stake::Draw() {
	if (!sprite_) {
		return;
	}
	sprite_->SetColor({0.75f, 0.75f, 0.7f, 1.0f});
	sprite_->SetPosition(position_);
	sprite_->SetSize(size_);
	sprite_->Draw();
}