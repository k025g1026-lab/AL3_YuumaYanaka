// [GameScene.cpp]
#include "GameScene.h"
#include "2d/ImGuiManager.h"
#include "Matrix.h"

using namespace KamataEngine;

GameScene::~GameScene() {
	delete player_;
	player_ = nullptr;

	delete skydome_;
	skydome_ = nullptr;

	delete modelSkydome_;
	modelSkydome_ = nullptr;

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

	// カメラのfarZを適度に大きい値に変更する
	camera_.farZ = 1000.0f; // 天球の大きさに合わせて調整

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// 自キャラ関連
	Model* playerModel = Model::CreateFromOBJ("player", true);
	player_ = new Player();
	player_->Initialize(playerModel, &camera_);

	// 天球モデル生成
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);

	// 天球クラス生成
	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_);

	// ブロック用モデルの生成
	modelBlock_ = Model::CreateFromOBJ("block", true);

	// 要素数
	const uint32_t kNumBlockVertical = 10;
	const uint32_t kNumBlockHorizontal = 20;
	// ブロック1個分のサイズ
	const float kBlockWidth = 1.0f;
	const float kBlockHeight = 1.0f;

	// 要素数を変更する（縦方向のブロック数）
	worldTransformBlocks_.resize(kNumBlockVertical);

	// ブロックの生成（穴開けパターン）
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
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
	if (Input::GetInstance()->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	debugCamera_->Update();
	// デバッグカメラの更新
	if (isDebugCameraActive_) {
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}

	// 自キャラの更新
	player_->Update();

	// 天球の更新
	skydome_->Update();

	// ブロックの更新
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			worldTransformBlock->matWorld_ = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);
			worldTransformBlock->TransferMatrix();
		}
	}
}

void GameScene::Draw() {
	// 3Dモデル描画前処理
	Model::PreDraw();

	// 自キャラの描画
	player_->Draw();

	// 天球の描画
	skydome_->Draw(camera_);

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	// 3Dモデル描画後処理
	Model::PostDraw();
}