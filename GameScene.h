// [GameScene.h]
#pragma once
#include "Boss.h"
#include "HookStitch.h"
#include "Player.h"
#include "Stake.h"
#include <array>
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
	void PushPlayerOutOfBosses();
	void DrawHp();

	Player* player_ = nullptr;
	Boss* bossA_ = nullptr;
	Boss* bossB_ = nullptr;
	Stake* stakeL_ = nullptr;
	Stake* stakeR_ = nullptr;
	HookStitch* hookStitch_ = nullptr;

	std::vector<StitchTarget*> targets_;

	KamataEngine::Sprite* groundSprite_ = nullptr;
	KamataEngine::Sprite* backSprite_ = nullptr;

	std::array<KamataEngine::Sprite*, 3> playerHpSprites_{};
	KamataEngine::Sprite* bossAHpBack_ = nullptr;
	KamataEngine::Sprite* bossAHpFill_ = nullptr;
	KamataEngine::Sprite* bossBHpBack_ = nullptr;
	KamataEngine::Sprite* bossBHpFill_ = nullptr;

	uint32_t whiteTexture_ = 0;
	Phase phase_ = Phase::kPlay;

	static inline const float kGroundY = 640.0f;
};