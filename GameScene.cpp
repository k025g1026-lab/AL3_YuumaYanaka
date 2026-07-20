// [GameScene.cpp]
#include "GameScene.h"
#include "2d/ImGuiManager.h"
#include "Matrix.h"

using namespace KamataEngine;

GameScene::~GameScene() {
	delete player_;
	player_ = nullptr;

	// ブロックの解放（二次元）
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete modelBlock_;
	modelBlock_ = nullptr;

	delete debugCamera_;
	debugCamera_ = nullptr;
}

void GameScene::Initialize() {
	// 通常カメラの初期化
	camera_.Initialize();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// 自キャラ関連
	Model* playerModel = Model::Create();
	uint32_t playerTexture = TextureManager::Load("mario.jpg");
	player_ = new Player();
	player_->Initialize(playerModel, playerTexture, &debugCamera_->GetCamera());

	// ブロック用モデルの生成
	modelBlock_ = Model::Create();

	// 要素数
	const uint32_t kNumBlockVertical = 10;
	const uint32_t kNumBlockHorizontal = 20;
	// ブロック1個分のサイズ
	const float kBlockWidth = 2.0f;
	const float kBlockHeight = 2.0f;

	// 要素数を変更する（縦方向のブロック数）
	worldTransformBlocks_.resize(kNumBlockVertical);

	// ブロックの生成（穴開けパターン）
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		// 1列分の要素数を設定（横方向のブロック数）
		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			// 穴を開ける条件
			if ((j + i % 2) % 2 == 1) {
				worldTransformBlocks_[i][j] = new WorldTransform();
				worldTransformBlocks_[i][j]->Initialize();
				worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
				worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * i;
			} else {
				worldTransformBlocks_[i][j] = nullptr; // 穴
			}
		}
	}
}

void GameScene::Update() {
	// デバッグカメラ有効フラグのトグル
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_0)) { // 切り替えキー（例：0キー）
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	// デバッグカメラの更新
	if (isDebugCameraActive_) {
		debugCamera_->Update();
	}

	// 自キャラの更新
	player_->Update();

	// ブロックの更新
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			// アフィン変換行列の作成
			worldTransformBlock->matWorld_ = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);
			// 定数バッファに転送する
			worldTransformBlock->TransferMatrix();
		}
	}
}

void GameScene::Draw() {
	// 3Dモデル描画前処理
	Model::PreDraw();

	// 自キャラの描画
	player_->Draw();

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			modelBlock_->Draw(*worldTransformBlock, isDebugCameraActive_ ? debugCamera_->GetCamera() : camera_);
		}
	}

	// 3Dモデル描画後処理
	Model::PostDraw();
}