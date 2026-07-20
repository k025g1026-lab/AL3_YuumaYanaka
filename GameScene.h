// [GameScene.h]
#pragma once
#include "Player.h"
#include "Skydome.h"
#include <kamataengine.h>
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

	// デストラクタ
	~GameScene();

private:
	// 自キャラ
	Player* player_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;

	// 3Dモデル（skydome用）
	KamataEngine::Model* modelSkydome_ = nullptr;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// 通常カメラ
	KamataEngine::Camera camera_;

	// デバッグカメラ有効フラグ
	bool isDebugCameraActive_ = false;

	// ブロック用モデル
	KamataEngine::Model* modelBlock_ = nullptr;

	// ブロック用のワールドトランスフォーム（二次元vector）
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
};