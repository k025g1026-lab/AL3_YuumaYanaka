// [Enemy.h]
#pragma once
#include "StitchTarget.h"

class Enemy : public StitchTarget {
public:
	void Initialize(uint32_t textureHandle, const KamataEngine::Vector2& position, float minX, float maxX);

	void Update() override;
	void Draw(const KamataEngine::Vector2& camera) override;

	AABB2 GetAABB() const override;
	KamataEngine::Vector2 GetPosition() const override { return position_; }
	void SetPosition(const KamataEngine::Vector2& position) override { position_ = position; }

	bool CanStitch() const override { return hp_ > 0; }
	bool IsFixed() const override { return false; }
	float GetMass() const override { return 1.0f; }
	Kind GetKind() const override { return Kind::kFodder; }
	bool IsDead() const override { return hp_ <= 0; }

	void OnCinchHit(int damage) override;
	void ApplyKnockback(const KamataEngine::Vector2& velocity) override;
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return kMaxHp; }
	KamataEngine::Vector2 GetSize() const { return size_; }

private:
	KamataEngine::Sprite* sprite_ = nullptr;
	KamataEngine::Vector2 position_{};
	KamataEngine::Vector2 velocity_{};
	KamataEngine::Vector2 size_{48.0f, 56.0f};

	static inline const int kMaxHp = 2;
	int hp_ = kMaxHp;
	int hitFlash_ = 0;
	int patrolDir_ = 1;
	float minX_ = 0.0f;
	float maxX_ = 0.0f;

	static inline const float kGravity = 0.55f;
	static inline const float kGroundY = 640.0f;
	static inline const float kPatrolSpeed = 1.4f;
};