// [Player.h]
#pragma once
#include "AABB.h"
#include "KamataEngine.h"
#include <array>

class MapChipField;
class Enemy;

/// <summary>
/// 自キャラ
/// </summary>
class Player {
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
	/// ワールドトランスフォーム取得
	/// </summary>
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	/// <summary>
	/// 速度取得
	/// </summary>
	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	/// <summary>
	/// マップチップフィールドをセット
	/// </summary>
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

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
	void OnCollision(const Enemy* enemy);

	/// <summary>
	/// デスフラグのgetter
	/// </summary>
	bool IsDead() const { return isDead_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	const KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Vector3 velocity_ = {};
	bool onGround_ = true;

	enum class LRDirection {
		kRight,
		kLeft,
	};
	LRDirection lrDirection_ = LRDirection::kRight;

	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	MapChipField* mapChipField_ = nullptr;
	bool isDead_ = false;

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.01f;

	static inline const float kAttenuationLanding = 0.5f;
	static inline const float kAttenuationWall = 0.5f;

	static inline const float kAcceleration = 0.05f;
	static inline const float kAttenuation = 0.15f;
	static inline const float kLimitRunSpeed = 0.45f;
	static inline const float kTimeTurn = 0.3f;

	// 高さは旧ジャンプ(初速1.0 / 重力0.15)と同じくらい
	// 滞空だけ長くするため重力を下げ、初速もそれに合わせて調整
	static inline const float kGravityAcceleration = 0.07f;
	static inline const float kLimitFallSpeed = 0.45f;
	static inline const float kJumpAcceleration = 0.68f;

	enum Corner { kRightBottom, kLeftBottom, kRightTop, kLeftTop, kNumCorner };

	struct CollisionMapInfo {
		bool ceiling = false;
		bool landing = false;
		bool hitWall = false;
		KamataEngine::Vector3 movement = {};
	};

	void InputMove();
	void MapCollision(CollisionMapInfo& info);
	void MapCollisionUp(CollisionMapInfo& info);
	void MapCollisionDown(CollisionMapInfo& info);
	void MapCollisionRight(CollisionMapInfo& info);
	void MapCollisionLeft(CollisionMapInfo& info);
	void MoveAfterCollision(const CollisionMapInfo& info);
	void OnCollisionCeiling(const CollisionMapInfo& info);
	void OnCollisionWall(const CollisionMapInfo& info);
	void SwitchOnGround(const CollisionMapInfo& info);
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);
};