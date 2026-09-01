// [HookStitch.h]
#pragma once
#include "KamataEngine.h"
#include "StitchTarget.h"
#include <vector>

class Player;

class HookStitch {
public:
	void Initialize(uint32_t textureHandle);
	void Update(Player* player, const std::vector<StitchTarget*>& targets);
	void Draw(Player* player);

	void TryStitch(StitchTarget* target);
	void Cinch(const std::vector<StitchTarget*>& allTargets);
	void Clear();

	int GetCount() const { return static_cast<int>(stitched_.size()); }
	bool IsCinching() const { return cinchTimer_ > 0; }

private:
	bool Contains(StitchTarget* target) const;
	KamataEngine::Vector2 CalcCentroid() const;
	void DrawThread(const KamataEngine::Vector2& from, const KamataEngine::Vector2& to, const KamataEngine::Vector4& color);

	std::vector<StitchTarget*> stitched_;
	std::vector<KamataEngine::Sprite*> threadSprites_;

	int cinchTimer_ = 0;
	bool pendingResolve_ = false;

	static inline const int kMaxStitch = 4;
	static inline const int kCoolDown = 20;
	static inline const int kCinchDuration = 12;
	static inline const float kCinchSpeed = 18.0f;
};