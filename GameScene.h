// [GameScene.h]
#pragma once
#include "Player.h"
#include <kamataengine.h>

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

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
};