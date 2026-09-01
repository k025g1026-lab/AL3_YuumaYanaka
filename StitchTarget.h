// [StitchTarget.h]
#pragma once
#include "AABB2.h"
#include "KamataEngine.h"

class StitchTarget {
public:
	enum class Kind {
		kBoss,
		kStake,
		kFodder,
	};

	virtual ~StitchTarget() = default;

	virtual void Update() = 0;
	virtual void Draw(const KamataEngine::Vector2& camera) = 0;

	virtual AABB2 GetAABB() const = 0;
	virtual KamataEngine::Vector2 GetPosition() const = 0;
	virtual void SetPosition(const KamataEngine::Vector2& position) = 0;

	virtual bool CanStitch() const = 0;
	virtual bool IsFixed() const = 0;
	virtual float GetMass() const = 0;
	virtual Kind GetKind() const = 0;
	virtual bool IsDead() const = 0;

	virtual void OnCinchHit(int damage) = 0;
	virtual void ApplyKnockback(const KamataEngine::Vector2& velocity) { (void)velocity; }

	bool IsStitchCooling() const { return stitchCoolDown_ > 0; }
	void StartStitchCoolDown(int frames) { stitchCoolDown_ = frames; }
	void UpdateStitchCoolDown() {
		if (stitchCoolDown_ > 0) {
			--stitchCoolDown_;
		}
	}

protected:
	int stitchCoolDown_ = 0;
};