// [Enemy.h]
#pragma once
#include "AABB.h"
#include "KamataEngine.h"

class MapChipField;
class Player;
class Enemy;

class Enemy {
public:
	/// <summary>
	/// 初期化
	/// </summary>
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
	/// 自キャラとの衝突
	/// </summary>
	void OnCollision(const Player* player);

	/// <summary>
	/// 敵同士の衝突
	/// </summary>
	void OnCollisionEnemy(Enemy* other);

	/// <summary>
	/// マップチップフィールドをセット
	/// </summary>
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

private:
	void UpdateFacing();
	void CheckMapWall();
	void CheckMapGround();

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	const KamataEngine::Camera* camera_ = nullptr;

	// 速度（初期は左方向）
	KamataEngine::Vector3 velocity_ = {-0.05f, 0.0f, 0.0f};

	// 接地フラグ
	bool onGround_ = false;

	// 歩行アニメーション用
	float walkTimer_ = 0.0f;

	// マップ
	MapChipField* mapChipField_ = nullptr;

	// 当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.01f;

	// 重力（プレイヤーと同じ緩め設定）
	static inline const float kGravityAcceleration = 0.15f;
	static inline const float kLimitFallSpeed = 0.7f;
};