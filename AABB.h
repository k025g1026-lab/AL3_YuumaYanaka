// [AABB.h]
#pragma once
#include "KamataEngine.h"

/// <summary>
/// 軸平行境界ボックス
/// </summary>
struct AABB {
	KamataEngine::Vector3 min; // 最小点
	KamataEngine::Vector3 max; // 最大点
};

/// <summary>
/// AABB同士の交差判定
/// </summary>
inline bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	if (aabb1.max.x < aabb2.min.x || aabb1.min.x > aabb2.max.x) {
		return false;
	}
	if (aabb1.max.y < aabb2.min.y || aabb1.min.y > aabb2.max.y) {
		return false;
	}
	if (aabb1.max.z < aabb2.min.z || aabb1.min.z > aabb2.max.z) {
		return false;
	}
	return true;
}