// [Player.h]
#pragma once
#include "KamataEngine.h"

/// <summary>
/// 自キャラ
/// </summary>
class Player {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="camera">カメラ</param>
	/// <param name="position">初期位置</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// カメラ
	const KamataEngine::Camera* camera_ = nullptr;
	// 速度
	KamataEngine::Vector3 velocity_ = {};
	// 接地状態フラグ
	bool onGround_ = true;
	// 左右方向
	enum class LRDirection {
		kRight,
		kLeft,
	};
	LRDirection lrDirection_ = LRDirection::kRight;

	// 回転補間用
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	// 定数
	static inline const float kAcceleration = 0.1f;
	static inline const float kAttenuation = 0.2f;
	static inline const float kLimitRunSpeed = 1.0f;
	static inline const float kTimeTurn = 0.3f;
	static inline const float kGravityAcceleration = 0.3f; // 重力加速度
	static inline const float kLimitFallSpeed = 1.0f;      // 最大落下速度
	static inline const float kJumpAcceleration = 1.0f;    // ジャンプ初速
};