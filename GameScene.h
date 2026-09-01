// [GameScene.h]
#pragma once
#include "Boss.h"
#include "HookStitch.h"
#include "Player.h"
#include "Stake.h"
#include <kamataengine.h>
#include <vector>

class GameScene {
public:
	void Initialize();
	void Update();
	void Draw();
	~GameScene();

private:
	enum class Phase {
		kPlay,
		kClear,
		kGameOver,
	};

	void CheckStitchOverlaps();
	void CheckPlayerHits();

	Player* player_ = nullptr;
	Boss* bossA_ = nullptr;
	Boss* bossB_ = nullptr;
	Stake* stakeL_ = nullptr;
	Stake* stakeR_ = nullptr;
	HookStitch* hookStitch_ = nullptr;

	std::vector<StitchTarget*> targets_;

	KamataEngine::Sprite* groundSprite_ = nullptr;
	KamataEngine::Sprite* backSprite_ = nullptr;

	uint32_t whiteTexture_ = 0;
	Phase phase_ = Phase::kPlay;

	static inline const float kGroundY = 640.0f;
};