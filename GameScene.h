// [GameScene.h]
#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "MapChipField.h"
#include "Player.h"
#include "Skydome.h"
#include <kamataengine.h>
#include <list>
#include <vector>

// ゲームシーン
class GameScene {

public:
	void Initialize();
	void Update();
	void Draw();
	void GenerateBlocks();
	void CheckAllCollisions();
	void CheckGoal();
	void ChangePhase();
	bool IsFinished() const { return finished_; }
	~GameScene();

private:
	enum class Phase {
		kPlay,
		kDeath,
		kClear,
	};

	Phase phase_;
	bool finished_ = false;

	Player* player_ = nullptr;

	std::list<Enemy*> enemies_;
	KamataEngine::Model* modelEnemy_ = nullptr;

	DeathParticles* deathParticles_ = nullptr;
	KamataEngine::Model* modelParticle_ = nullptr;

	Skydome* skydome_ = nullptr;
	KamataEngine::Model* modelSkydome_ = nullptr;

	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	KamataEngine::Camera camera_;
	CameraController* cameraController_ = nullptr;
	bool isDebugCameraActive_ = false;

	KamataEngine::Model* modelBlock_ = nullptr;
	KamataEngine::Model* modelGoal_ = nullptr;

	// クリア画面
	KamataEngine::Model* modelGoalFont_ = nullptr;
	KamataEngine::WorldTransform worldTransformGoalFont_;
	KamataEngine::Camera clearCamera_;
	bool isClearSetup_ = false;
	float clearTimer_ = 0.0f;

	MapChipField* mapChipField_ = nullptr;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
	std::vector<KamataEngine::WorldTransform*> worldTransformGoals_;
};