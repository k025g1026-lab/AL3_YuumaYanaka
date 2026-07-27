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
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// ブロック生成
	void GenerateBlocks();

	// 全ての当たり判定を行う
	void CheckAllCollisions();

	// フェーズの切り替え
	void ChangePhase();

	/// <summary>
	/// 終了フラグのgetter
	/// </summary>
	bool IsFinished() const { return finished_; }

	// デストラクタ
	~GameScene();

private:
	// ゲームのフェーズ（型）
	enum class Phase {
		kPlay,  // ゲームプレイ
		kDeath, // デス演出
	};

	// ゲームの現在フェーズ（変数）
	Phase phase_;

	// 終了フラグ
	bool finished_ = false;

	// 自キャラ
	Player* player_ = nullptr;

	// 敵
	std::list<Enemy*> enemies_;

	// 敵用モデル
	KamataEngine::Model* modelEnemy_ = nullptr;

	// デスパーティクル
	DeathParticles* deathParticles_ = nullptr;

	// デスパーティクル用モデル
	KamataEngine::Model* modelParticle_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;

	// 3Dモデル（skydome用）
	KamataEngine::Model* modelSkydome_ = nullptr;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// 通常カメラ
	KamataEngine::Camera camera_;

	// カメラコントローラー
	CameraController* cameraController_ = nullptr;

	// デバッグカメラ有効フラグ
	bool isDebugCameraActive_ = false;

	// ブロック用モデル
	KamataEngine::Model* modelBlock_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	// ブロック用のワールドトランスフォーム（二次元vector）
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
};