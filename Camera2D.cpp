// [Camera2D.cpp]
#define NOMINMAX
#include "Camera2D.h"
#include <algorithm>

using namespace KamataEngine;

void Camera2D::SetFixed(const Vector2& topLeft) { position_ = topLeft; }

void Camera2D::UpdateFollow(const Vector2& playerCenter, float mapLeft, float mapRight, float mapTop, float mapBottom) {
	// プレイヤーを中央より少し左に置く
	position_.x = playerCenter.x - kViewW * 0.38f;

	const float desiredY = playerCenter.y - kViewH * 0.58f;
	if (desiredY > position_.y + kDeadZoneY) {
		position_.y = desiredY - kDeadZoneY;
	} else if (desiredY < position_.y - kDeadZoneY) {
		position_.y = desiredY + kDeadZoneY;
	}

	const float maxX = std::max(mapLeft, mapRight - kViewW);
	const float maxY = std::max(mapTop, mapBottom - kViewH);
	position_.x = std::clamp(position_.x, mapLeft, maxX);
	position_.y = std::clamp(position_.y, mapTop, maxY);
}