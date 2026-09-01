// [GameScene.h]
#pragma once
#include "Boss.h"
#include "Camera2D.h"
#include "Enemy.h"
#include "HookStitch.h"
#include "Player.h"
#include "Stage.h"
#include "Stake.h"
#include <array>
#include <kamataengine.h>
#include <vector>

/// <summary>
/// 本編。1〜3面＋ボスを1ワールドとして持ち、
/// 部屋クリア後に扉へ触れると次エリアへカメラを切り替える。
/// </summary>
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

	void BuildWorld();
	void ClearCurrentActors();
	void CheckStitchOverlaps();
	void CheckPlayerHits();
	void PushPlayerOutOfBosses();
	void CheckSectionClear();
	void BlockClosedDoors();
	void TryEnterNextArea();
	void EnterSection(int section);
	void RespawnFromSave();
	void ResetSectionEnemies(int section);
	void UpdateCamera();
	void DrawHp();
	void DrawDoors();

	bool IsSectionCleared(int section) const;
	KamataEngine::Vector2 GetSectionSpawn(int section) const;

	Player* player_ = nullptr;
	Boss* bossA_ = nullptr;
	Boss* bossB_ = nullptr;
	Stake* stakeL_ = nullptr;
	Stake* stakeR_ = nullptr;
	HookStitch* hookStitch_ = nullptr;
	std::vector<Enemy*> fodder_;
	std::vector<int> fodderSection_; // fodder_[i] が何面の敵か
	std::vector<StitchTarget*> targets_;

	KamataEngine::Sprite* groundSprite_ = nullptr;
	KamataEngine::Sprite* backSprite_ = nullptr;
	std::array<KamataEngine::Sprite*, 3> playerHpSprites_{};
	KamataEngine::Sprite* bossAHpBack_ = nullptr;
	KamataEngine::Sprite* bossAHpFill_ = nullptr;
	KamataEngine::Sprite* bossBHpBack_ = nullptr;
	KamataEngine::Sprite* bossBHpFill_ = nullptr;
	std::array<KamataEngine::Sprite*, 3> doorSprites_{};

	static inline const int kMaxFodderUi = 8;
	std::array<KamataEngine::Sprite*, kMaxFodderUi> fodderHpBack_{};
	std::array<KamataEngine::Sprite*, kMaxFodderUi> fodderHpFill_{};

	uint32_t whiteTexture_ = 0;
	Phase phase_ = Phase::kPlay;

	WorldDesc world_{};
	int cameraSection_ = 1; // 今映している部屋
	int saveSection_ = 1;   // 死亡時に戻る部屋
	bool cleared1_ = false;
	bool cleared2_ = false;
	bool cleared3_ = false;

	Camera2D camera_;
};