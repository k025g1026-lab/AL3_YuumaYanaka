// [AABB2.h]
#pragma once
#include "KamataEngine.h"

struct AABB2 {
	KamataEngine::Vector2 min{};
	KamataEngine::Vector2 max{};
};

inline bool IsCollision(const AABB2& a, const AABB2& b) {
	if (a.max.x < b.min.x) {
		return false;
	}
	if (a.min.x > b.max.x) {
		return false;
	}
	if (a.max.y < b.min.y) {
		return false;
	}
	if (a.min.y > b.max.y) {
		return false;
	}
	return true;
}

inline KamataEngine::Vector2 AABBCenter(const AABB2& a) { return {(a.min.x + a.max.x) * 0.5f, (a.min.y + a.max.y) * 0.5f}; }

