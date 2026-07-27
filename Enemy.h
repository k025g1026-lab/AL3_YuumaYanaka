// [Enemy.h]
#pragma once
#include "AABB.h"
#include "KamataEngine.h"

class Player;

/// <summary>
/// 敵
/// </summary>
class Enemy {
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

	/// <summary>
	/// ワールド座標を取得
	/// </summary>
	KamataEngine::Vector3 GetWorldPosition();

	/// <summary>
	/// AABBを取得
	/// </summary>
	AABB GetAABB();

	/// <summary>
	/// 衝突応答
	/// </summary>
	void OnCollision(const Player* player);

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// カメラ
	const KamataEngine::Camera* camera_ = nullptr;

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.05f;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 最初の角度[度]
	static inline const float kWalkMotionAngleStart = -15.0f;
	// 最後の角度[度]
	static inline const float kWalkMotionAngleEnd = 15.0f;
	// アニメーションの周期となる時間[秒]
	static inline const float kWalkMotionTime = 0.5f;

	// 経過時間
	float walkTimer_ = 0.0f;

	// 当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
};