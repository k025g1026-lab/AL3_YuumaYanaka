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

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// デスフラグ
	bool isDead_ = false;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// 微小な余白
	static inline const float kBlank = 0.01f;

	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.5f;

	// 壁接触時の速度減衰率
	static inline const float kAttenuationWall = 0.5f;

	// 定数
	static inline const float kAcceleration = 0.1f;
	static inline const float kAttenuation = 0.2f;
	static inline const float kLimitRunSpeed = 1.0f;
	static inline const float kTimeTurn = 0.3f;
	static inline const float kGravityAcceleration = 0.3f;
	static inline const float kLimitFallSpeed = 1.0f;
	static inline const float kJumpAcceleration = 1.0f;

	// 角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool ceiling = false;                // 天井衝突フラグ
		bool landing = false;                // 着地フラグ
		bool hitWall = false;                // 壁接触フラグ
		KamataEngine::Vector3 movement = {}; // 移動量
	};

	// 移動入力
	void InputMove();

	// マップ衝突判定
	void MapCollision(CollisionMapInfo& info);

	// マップ衝突判定 上方向
	void MapCollisionUp(CollisionMapInfo& info);

	// マップ衝突判定 下方向
	void MapCollisionDown(CollisionMapInfo& info);

	// マップ衝突判定 右方向
	void MapCollisionRight(CollisionMapInfo& info);

	// マップ衝突判定 左方向
	void MapCollisionLeft(CollisionMapInfo& info);

	// 判定結果を反映して移動させる
	void MoveAfterCollision(const CollisionMapInfo& info);

	// 天井に接触している場合の処理
	void OnCollisionCeiling(const CollisionMapInfo& info);

	// 壁に接触している場合の処理
	void OnCollisionWall(const CollisionMapInfo& info);

	// 接地状態の切り替え処理
	void SwitchOnGround(const CollisionMapInfo& info);

	// 指定した角の座標計算
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);
};