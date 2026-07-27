// [GameScene.cpp]
#include "GameScene.h"
#include "2d/ImGuiManager.h"
#include "AABB.h"
#include "Matrix.h"

using namespace KamataEngine;

GameScene::~GameScene() {
	delete player_;
	player_ = nullptr;

	// 敵の解放
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	delete modelEnemy_;
	modelEnemy_ = nullptr;

	delete skydome_;
	skydome_ = nullptr;

	delete modelSkydome_;
	modelSkydome_ = nullptr;

	delete cameraController_;
	cameraController_ = nullptr;

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

	// カメラコントローラー生成
	cameraController_ = new CameraController();
	cameraController_->Initialize();

	// マップチップフィールド生成
	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/map.csv");

	// 自キャラ関連
	Model* playerModel = Model::CreateFromOBJ("player", true);
	player_ = new Player();

	// 初期位置をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);
	player_->Initialize(playerModel, &camera_, playerPosition);

	// マップチップデータをセット
	player_->SetMapChipField(mapChipField_);

	// カメラコントローラーに追従対象を設定
	cameraController_->SetTarget(player_);

	// 移動範囲の指定
	CameraController::Rect movableArea = {11.0f, 88.0f, 6.0f, 100.0f};
	cameraController_->SetMovableArea(movableArea);

	// 敵モデル生成
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);

	// 敵の生成（複数）
	const int32_t enemyCount = 5;
	for (int32_t i = 0; i < enemyCount; ++i) {
		Enemy* newEnemy = new Enemy();
		// 一体ずつ異なる座標をセット
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(8 + i * 6, 18);
		newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);
		enemies_.push_back(newEnemy);
	}

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
		cameraController_->Update();
		camera_.matView = cameraController_->GetCamera().matView;
		camera_.matProjection = cameraController_->GetCamera().matProjection;
		camera_.TransferMatrix();
	}

	// 自キャラの更新
	player_->Update();

	// 敵の更新
	for (Enemy* enemy : enemies_) {
		if (enemy) {
			enemy->Update();
		}
	}

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

	// 全ての当たり判定を行う（最後に呼ぶ）
	CheckAllCollisions();
}

void GameScene::Draw() {
	// 3Dモデル描画前処理
	Model::PreDraw();

	// 自キャラの描画
	player_->Draw();

	// 敵の描画
	for (Enemy* enemy : enemies_) {
		if (enemy) {
			enemy->Draw();
		}
	}

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

void GameScene::CheckAllCollisions() {
#pragma region 自キャラと敵キャラの当たり判定
	// 判定対象1と2の座標
	AABB aabb1, aabb2;

	// 自キャラの座標
	aabb1 = player_->GetAABB();

	// 自キャラと敵全ての当たり判定
	for (Enemy* enemy : enemies_) {
		if (!enemy) {
			continue;
		}
		// 敵弾の座標
		aabb2 = enemy->GetAABB();

		// AABB同士の交差判定
		if (IsCollision(aabb1, aabb2)) {
			// 自キャラの衝突時コールバックを呼び出す
			player_->OnCollision(enemy);
			// 敵弾の衝突時コールバックを呼び出す
			enemy->OnCollision(player_);
		}
	}
#pragma endregion
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