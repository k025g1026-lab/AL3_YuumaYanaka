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

	delete mapChipField_;
	mapChipField_ = nullptr;

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
	camera_.farZ = 1000.0f;

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// 自キャラ関連
	Model* playerModel = Model::CreateFromOBJ("player", true);
	player_ = new Player();

	// マップチップフィールド生成
	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/map.csv");

	// 初期位置をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18); // 左上から適当な位置（調整してください）
	player_->Initialize(playerModel, &camera_, playerPosition);

	// 天球モデル生成
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);

	// 天球クラス生成
	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_);

	// ブロック用モデルの生成
	modelBlock_ = Model::CreateFromOBJ("block", true);

	// ブロック生成
	GenerateBlocks();
}

void GameScene::Update() {
	// デバッグカメラ有効フラグのトグル
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	// デバッグカメラの更新
	if (isDebugCameraActive_) {
		debugCamera_->Update();
	}

	// カメラの処理
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

void GameScene::GenerateBlocks() {
	// 要素数
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVertical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	worldTransformBlocks_.resize(numBlockVertical);

	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				worldTransformBlocks_[i][j] = new WorldTransform();
				worldTransformBlocks_[i][j]->Initialize();
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			} else {
				worldTransformBlocks_[i][j] = nullptr;
			}
		}
	}
}