// [GameScene.cpp]
#include "GameScene.h"
#include "2d/ImGuiManager.h"
#include "AABB.h"
#include "Matrix.h"
#include <cmath>
#include <numbers>

using namespace KamataEngine;

GameScene::~GameScene() {
	delete player_;
	player_ = nullptr;

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	delete modelEnemy_;
	modelEnemy_ = nullptr;

	delete deathParticles_;
	deathParticles_ = nullptr;

	delete modelParticle_;
	modelParticle_ = nullptr;

	delete skydome_;
	skydome_ = nullptr;

	delete modelSkydome_;
	modelSkydome_ = nullptr;

	delete cameraController_;
	cameraController_ = nullptr;

	delete mapChipField_;
	mapChipField_ = nullptr;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	for (WorldTransform* worldTransformGoal : worldTransformGoals_) {
		delete worldTransformGoal;
	}
	worldTransformGoals_.clear();

	delete modelBlock_;
	modelBlock_ = nullptr;

	delete modelGoal_;
	modelGoal_ = nullptr;

	delete modelGoalFont_;
	modelGoalFont_ = nullptr;

	delete debugCamera_;
	debugCamera_ = nullptr;
}

void GameScene::Initialize() {
	camera_.Initialize();
	camera_.farZ = 1000.0f;

	debugCamera_ = new DebugCamera(1280, 720);

	cameraController_ = new CameraController();
	cameraController_->Initialize();

	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/map.csv");

	Model* playerModel = Model::CreateFromOBJ("player", true);
	player_ = new Player();

	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);
	player_->Initialize(playerModel, &camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);

	cameraController_->SetTarget(player_);

	CameraController::Rect movableArea = {11.0f, 88.0f, 6.0f, 100.0f};
	cameraController_->SetMovableArea(movableArea);

	modelEnemy_ = Model::CreateFromOBJ("enemy", true);

	const uint32_t numBlockVertical = mapChipField_->GetNumBlockVertical();
	const uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();
	for (uint32_t y = 0; y < numBlockVertical; ++y) {
		for (uint32_t x = 0; x < numBlockHorizontal; ++x) {
			if (mapChipField_->GetMapChipTypeByIndex(x, y) != MapChipType::kEnemy) {
				continue;
			}

			Enemy* newEnemy = new Enemy();
			Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
			newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);
			newEnemy->SetMapChipField(mapChipField_);
			enemies_.push_back(newEnemy);
		}
	}

	modelParticle_ = Model::CreateFromOBJ("deathParticle", true);

	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_);

	modelBlock_ = Model::CreateFromOBJ("block", true);
	modelGoal_ = Model::CreateFromOBJ("Goal", true);

	modelGoalFont_ = Model::CreateFromOBJ("Goalfont", true);
	worldTransformGoalFont_.Initialize();

	clearCamera_.Initialize();
	clearCamera_.farZ = 1000.0f;

	GenerateBlocks();

	phase_ = Phase::kPlay;
	finished_ = false;
	isClearSetup_ = false;
	clearTimer_ = 0.0f;
}

void GameScene::Update() {
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	if (isDebugCameraActive_) {
		debugCamera_->Update();
	}

	if (phase_ != Phase::kClear) {
		if (isDebugCameraActive_) {
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			camera_.TransferMatrix();
		} else {
			if (phase_ == Phase::kPlay) {
				cameraController_->Update();
			}
			camera_.matView = cameraController_->GetCamera().matView;
			camera_.matProjection = cameraController_->GetCamera().matProjection;
			camera_.TransferMatrix();
		}
	}

	switch (phase_) {
	case Phase::kPlay:
		skydome_->Update();
		player_->Update();

		for (Enemy* enemy : enemies_) {
			if (enemy) {
				enemy->Update();
			}
		}

		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock) {
					continue;
				}
				worldTransformBlock->matWorld_ = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);
				worldTransformBlock->TransferMatrix();
			}
		}

		for (WorldTransform* worldTransformGoal : worldTransformGoals_) {
			if (!worldTransformGoal) {
				continue;
			}
			worldTransformGoal->matWorld_ = MakeAffineMatrix(worldTransformGoal->scale_, worldTransformGoal->rotation_, worldTransformGoal->translation_);
			worldTransformGoal->TransferMatrix();
		}

		CheckAllCollisions();
		CheckGoal();

		if (player_->IsDead()) {
			phase_ = Phase::kDeath;
			const Vector3 deathParticlesPosition = player_->GetWorldPosition();
			deathParticles_ = new DeathParticles();
			deathParticles_->Initialize(modelParticle_, &camera_, deathParticlesPosition);
		}
		break;

	case Phase::kDeath:
		skydome_->Update();

		for (Enemy* enemy : enemies_) {
			if (enemy) {
				enemy->Update();
			}
		}

		if (deathParticles_) {
			deathParticles_->Update();
		}

		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock) {
					continue;
				}
				worldTransformBlock->matWorld_ = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);
				worldTransformBlock->TransferMatrix();
			}
		}

		for (WorldTransform* worldTransformGoal : worldTransformGoals_) {
			if (!worldTransformGoal) {
				continue;
			}
			worldTransformGoal->matWorld_ = MakeAffineMatrix(worldTransformGoal->scale_, worldTransformGoal->rotation_, worldTransformGoal->translation_);
			worldTransformGoal->TransferMatrix();
		}

		if (deathParticles_ && deathParticles_->IsFinished()) {
			finished_ = true;
		}
		break;

	case Phase::kClear:
		if (!isClearSetup_) {
			// タイトル画面と同じ見方にする
			clearCamera_.Initialize();
			clearCamera_.farZ = 1000.0f;
			clearCamera_.UpdateMatrix();

			worldTransformGoalFont_.Initialize();
			worldTransformGoalFont_.translation_ = {0.0f, 2.0f, 0.0f};
			isClearSetup_ = true;
			clearTimer_ = 0.0f;
		}

		clearTimer_ += 1.0f / 60.0f;
		skydome_->Update();

		// 文字をゆっくり揺らす
		worldTransformGoalFont_.translation_.y = 2.0f + std::sin(clearTimer_ * 2.0f) * 0.2f;
		worldTransformGoalFont_.rotation_.y = std::sin(clearTimer_) * 0.15f;
		worldTransformGoalFont_.matWorld_ = MakeAffineMatrix(worldTransformGoalFont_.scale_, worldTransformGoalFont_.rotation_, worldTransformGoalFont_.translation_);
		worldTransformGoalFont_.TransferMatrix();

		Input* input = Input::GetInstance();
		if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN) || input->IsTriggerMouse(0)) {
			finished_ = true;
		}
		break;
	}
}

void GameScene::Draw() {
	Model::PreDraw();

	if (phase_ == Phase::kClear) {
		// タイトル画面と同じ構成：天球 + 文字
		skydome_->Draw(clearCamera_);
		if (modelGoalFont_) {
			modelGoalFont_->Draw(worldTransformGoalFont_, clearCamera_);
		}
		Model::PostDraw();
		return;
	}

	if (!player_->IsDead()) {
		player_->Draw();
	}

	for (Enemy* enemy : enemies_) {
		if (enemy) {
			enemy->Draw();
		}
	}

	if (deathParticles_) {
		deathParticles_->Draw();
	}

	skydome_->Draw(camera_);

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	for (WorldTransform* worldTransformGoal : worldTransformGoals_) {
		if (!worldTransformGoal) {
			continue;
		}
		modelGoal_->Draw(*worldTransformGoal, camera_);
	}

	Model::PostDraw();
}

void GameScene::CheckAllCollisions() {
#pragma region 自キャラと敵キャラの当たり判定
	AABB aabb1, aabb2;
	aabb1 = player_->GetAABB();

	for (Enemy* enemy : enemies_) {
		if (!enemy) {
			continue;
		}
		aabb2 = enemy->GetAABB();
		if (IsCollision(aabb1, aabb2)) {
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}
#pragma endregion

#pragma region 敵同士の当たり判定
	for (auto it1 = enemies_.begin(); it1 != enemies_.end(); ++it1) {
		if (!(*it1)) {
			continue;
		}
		auto it2 = it1;
		++it2;
		for (; it2 != enemies_.end(); ++it2) {
			if (!(*it2)) {
				continue;
			}
			if (IsCollision((*it1)->GetAABB(), (*it2)->GetAABB())) {
				(*it1)->OnCollisionEnemy(*it2);
				(*it2)->OnCollisionEnemy(*it1);
			}
		}
	}
#pragma endregion
}

void GameScene::CheckGoal() {
	if (!player_ || !mapChipField_) {
		return;
	}

	const Vector3 playerPos = player_->GetWorldPosition();
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(playerPos);

	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kGoal) {
		phase_ = Phase::kClear;
	}
}

void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		break;
	case Phase::kDeath:
		break;
	case Phase::kClear:
		break;
	}
}

void GameScene::GenerateBlocks() {
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVertical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVertical);
	worldTransformGoals_.clear();

	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);
			if (type == MapChipType::kBlock) {
				worldTransformBlocks_[i][j] = new WorldTransform();
				worldTransformBlocks_[i][j]->Initialize();
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			} else if (type == MapChipType::kGoal) {
				worldTransformBlocks_[i][j] = nullptr;

				WorldTransform* goal = new WorldTransform();
				goal->Initialize();
				goal->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				worldTransformGoals_.push_back(goal);
			} else {
				worldTransformBlocks_[i][j] = nullptr;
			}
		}
	}
}