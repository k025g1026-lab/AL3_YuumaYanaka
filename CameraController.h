// [CameraController.h]
#pragma once
#include "KamataEngine.h"

class Player;

class CameraController {
public:
	// 範囲用構造体
	struct Rect {
		float left = 0.0f;    // 左端
		float right = 100.0f; // 右端
		float bottom = 0.0f;  // 下端
		float top = 100.0f;   // 上端
	};

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 追従対象を設定
	/// </summary>
	void SetTarget(Player* target) { target_ = target; }

	/// <summary>
	/// 移動範囲を設定
	/// </summary>
	void SetMovableArea(const Rect& area) { movableArea_ = area; }

	/// <summary>
	/// リセット（瞬間合わせ）
	/// </summary>
	void Reset();

	/// <summary>
	/// カメラ取得
	/// </summary>
	KamataEngine::Camera& GetCamera() { return camera_; }

private:
	// カメラ
	KamataEngine::Camera camera_;
	// 追従対象
	Player* target_ = nullptr;
	// 追従対象とカメラの座標の差（オフセット）
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};
	// カメラの目標座標
	KamataEngine::Vector3 targetPosition_ = {};

	// カメラ移動範囲
	Rect movableArea_;

	// 座標補間割合
	static inline const float kInterpolationRate = 0.1f;
	// 速度掛け率
	static inline const float kVelocityBias = 2.0f;

	// 追従対象の各方向へのカメラ移動範囲（マージン）
	static inline const Rect kMargin = {-5.0f, 5.0f, -5.0f, 5.0f};
};