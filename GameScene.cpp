// [GameScene.cpp]
#include "GameScene.h"
#include <cmath>

using namespace KamataEngine;

GameScene::~GameScene() {
	ClearCurrentActors();
	delete player_;
	delete hookStitch_;
	delete mapChipField_;
}

void GameScene::ClearCurrentActors() {
	delete bossA_;
	bossA_ = nullptr;
	delete bossB_;
	bossB_ = nullptr;
	delete stakeL_;
	stakeL_ = nullptr;
	delete stakeR_;
	stakeR_ = nullptr;
	for (Enemy* enemy : fodder_) {
		delete enemy;
	}
	fodder_.clear();
	fodderSection_.clear();
	fodderSpawns_.clear();
	targets_.clear();
	world_.doors.clear();
	blockSprites_.clear();
	blockPositions_.clear();
}

void GameScene::Initialize() {
	whiteTexture_ = TextureManager::Load("white.png");
	backSprite_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});

	for (int i = 0; i < 3; ++i) {
		playerHpSprites_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	}
	for (int i = 0; i < 8; ++i) {
		doorSprites_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	}
	bossAHpBack_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	bossAHpFill_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	bossBHpBack_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	bossBHpFill_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	for (int i = 0; i < kMaxFodderUi; ++i) {
		fodderHpBack_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
		fodderHpFill_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	}

	player_ = new Player();
	player_->Initialize(whiteTexture_);
	hookStitch_ = new HookStitch();
	hookStitch_->Initialize(whiteTexture_);

	mapChipField_ = new MapChipField();
	BuildWorld();
}

void GameScene::BuildWorld() {
	ClearCurrentActors();
	if (hookStitch_) {
		hookStitch_->Clear();
	}

	world_ = GetWorldDesc();
	cameraSection_ = 1;
	saveSection_ = 1;
	cleared1_ = false;
	cleared2_ = false;
	cleared3_ = false;
	sectionSpawns_ = {world_.spawn1, world_.spawn2, world_.spawn3, world_.spawnBoss};

	mapChipField_->LoadMapChipCsv("Resources/map.csv");

	int bossCount = 0;
	std::array<bool, 4> hasSpawn{};

	const uint32_t h = mapChipField_->GetNumBlockHorizontal();
	const uint32_t v = mapChipField_->GetNumBlockVertical();
	for (uint32_t y = 0; y < v; ++y) {
		for (uint32_t x = 0; x < h; ++x) {
			const int ix = static_cast<int>(x);
			const int iy = static_cast<int>(y);
			const MapChipType type = mapChipField_->GetMapChipTypeByIndex(ix, iy);
			const Vector2 cell = mapChipField_->GetMapChipPositionByIndex(ix, iy);
			const int section = GetSectionByTileX(ix);

			if (type == MapChipType::kBlock) {
				blockSprites_.push_back(Sprite::Create(whiteTexture_, {0.0f, 0.0f}));
				blockPositions_.push_back(cell);
			} else if (type == MapChipType::kPlayer) {
				if (section >= 1 && section <= 4 && !hasSpawn[section - 1]) {
					Vector2 spawn = cell;
					spawn.y = mapChipField_->SnapFeetToFloor(cell.x + 8.0f, 64.0f);
					sectionSpawns_[section - 1] = spawn;
					hasSpawn[section - 1] = true;
				}
			} else if (type == MapChipType::kFodder) {
				FodderSpawn spawn;
				spawn.position = cell;
				spawn.position.y = mapChipField_->SnapFeetToFloor(cell.x + 8.0f, 56.0f);
				spawn.minX = spawn.position.x - 128.0f;
				spawn.maxX = spawn.position.x + 128.0f;
				spawn.section = section;

				Enemy* enemy = new Enemy();
				enemy->Initialize(whiteTexture_, spawn.position, spawn.minX, spawn.maxX);
				fodder_.push_back(enemy);
				fodderSection_.push_back(section);
				fodderSpawns_.push_back(spawn);
				targets_.push_back(enemy);
			} else if (type == MapChipType::kDoor) {
				DoorDesc door;
				door.x = cell.x;
				door.y = cell.y;
				door.width = MapChipField::kTileWidth;
				door.height = MapChipField::kTileHeight;
				door.openAfterSection = section;
				door.leadsToSection = section + 1;
				world_.doors.push_back(door);
			} else if (type == MapChipType::kStake) {
				Vector2 spawn = cell;
				spawn.y = mapChipField_->SnapFeetToFloor(cell.x + 8.0f, 160.0f);
				Stake* stake = new Stake();
				stake->Initialize(whiteTexture_, spawn);
				if (!stakeL_) {
					stakeL_ = stake;
				} else if (!stakeR_) {
					stakeR_ = stake;
				}
				targets_.push_back(stake);
			} else if (type == MapChipType::kBoss) {
				Vector2 spawn = cell;
				spawn.y = mapChipField_->SnapFeetToFloor(cell.x + 8.0f, 120.0f);
				Boss* boss = new Boss();
				const Vector4 color = (bossCount == 0) ? Vector4{0.95f, 0.25f, 0.25f, 1.0f} : Vector4{0.95f, 0.55f, 0.2f, 1.0f};
				boss->Initialize(whiteTexture_, spawn, color, spawn.x - 160.0f, spawn.x + 160.0f);
				if (bossCount == 0) {
					bossA_ = boss;
				} else {
					bossB_ = boss;
				}
				++bossCount;
				targets_.push_back(boss);
			}
		}
	}

	player_->Reset(GetSectionSpawn(1));
	player_->SetMapBounds(RoomLeft(1), RoomRight(1));
	phase_ = Phase::kPlay;
	enterWait_ = 30;
	UpdateCamera();
}

Vector2 GameScene::GetSectionSpawn(int section) const {
	int index = section - 1;
	if (index < 0) {
		index = 0;
	}
	if (index > 3) {
		index = 3;
	}
	return sectionSpawns_[index];
}

void GameScene::EnterSection(int section) {
	cameraSection_ = section;
	if (section > saveSection_) {
		saveSection_ = section;
	}
	player_->SetMapBounds(RoomLeft(section), RoomRight(section));
	player_->SetPosition(GetSectionSpawn(section));
	if (hookStitch_) {
		hookStitch_->Clear();
	}
	enterWait_ = 30;
	UpdateCamera();
}

void GameScene::UpdateCamera() { camera_.SetFixed({RoomLeft(cameraSection_), 0.0f}); }

bool GameScene::IsSectionCleared(int section) const {
	if (section == 1) {
		return cleared1_;
	}
	if (section == 2) {
		return cleared2_;
	}
	if (section == 3) {
		return cleared3_;
	}
	return false;
}

void GameScene::CheckSectionClear() {
	auto sectionDead = [&](int section) {
		bool any = false;
		for (size_t i = 0; i < fodder_.size(); ++i) {
			if (fodderSection_[i] != section) {
				continue;
			}
			any = true;
			if (fodder_[i] && !fodder_[i]->IsDead()) {
				return false;
			}
		}
		return any;
	};

	if (!cleared1_ && sectionDead(1)) {
		cleared1_ = true;
	}
	if (!cleared2_ && sectionDead(2)) {
		cleared2_ = true;
	}
	if (!cleared3_ && sectionDead(3)) {
		cleared3_ = true;
	}

	if (bossA_ && bossB_ && bossA_->IsDead() && bossB_->IsDead()) {
		phase_ = Phase::kClear;
	}
}

void GameScene::BlockClosedDoors() {
	Vector2 pos = player_->GetPosition();
	AABB2 p = player_->GetAABB();
	const float playerW = p.max.x - p.min.x;

	for (const DoorDesc& door : world_.doors) {
		if (IsSectionCleared(door.openAfterSection)) {
			continue;
		}

		AABB2 d;
		d.min = {door.x, door.y};
		d.max = {door.x + door.width, door.y + door.height};
		if (!IsCollision(p, d)) {
			continue;
		}

		const float overlapFromLeft = p.max.x - d.min.x;
		const float overlapFromRight = d.max.x - p.min.x;
		if (overlapFromRight < overlapFromLeft) {
			pos.x = d.max.x;
		} else {
			pos.x = d.min.x - playerW;
		}
		player_->SetPosition(pos);
		p = player_->GetAABB();
	}
}

void GameScene::TryEnterNextArea() {
	if (enterWait_ > 0) {
		return;
	}

	const Vector2 center = player_->GetCenter();
	for (const DoorDesc& door : world_.doors) {
		if (!IsSectionCleared(door.openAfterSection)) {
			continue;
		}
		if (door.leadsToSection != cameraSection_ + 1) {
			continue;
		}

		const float doorCenterX = door.x + door.width * 0.5f;
		const float doorCenterY = door.y + door.height * 0.5f;
		const float hitW = door.width * 0.25f;
		const float hitH = door.height * 0.25f;

		if (std::abs(center.x - doorCenterX) <= hitW && std::abs(center.y - doorCenterY) <= hitH) {
			EnterSection(door.leadsToSection);
			break;
		}
	}
}

void GameScene::ResetSectionEnemies(int section) {
	if (section >= 4) {
		BuildWorld();
		EnterSection(3);
		return;
	}

	for (size_t i = 0; i < fodder_.size(); ++i) {
		if (fodderSection_[i] != section) {
			continue;
		}
		delete fodder_[i];
		const FodderSpawn& spawn = fodderSpawns_[i];
		fodder_[i] = new Enemy();
		fodder_[i]->Initialize(whiteTexture_, spawn.position, spawn.minX, spawn.maxX);
	}

	targets_.clear();
	for (Enemy* enemy : fodder_) {
		targets_.push_back(enemy);
	}
	if (bossA_) {
		targets_.push_back(bossA_);
	}
	if (bossB_) {
		targets_.push_back(bossB_);
	}
	if (stakeL_) {
		targets_.push_back(stakeL_);
	}
	if (stakeR_) {
		targets_.push_back(stakeR_);
	}
}

void GameScene::RespawnFromSave() {
	if (hookStitch_) {
		hookStitch_->Clear();
	}
	int section = saveSection_;
	if (section >= 4) {
		section = 3;
		saveSection_ = 3;
	}
	ResetSectionEnemies(section);
	cameraSection_ = section;
	player_->Reset(GetSectionSpawn(section));
	player_->SetMapBounds(RoomLeft(section), RoomRight(section));
	phase_ = Phase::kPlay;
	enterWait_ = 30;
	UpdateCamera();
}

void GameScene::CheckStitchOverlaps() {
	AABB2 playerAABB = player_->GetAABB();
	for (StitchTarget* target : targets_) {
		if (!target || !target->CanStitch() || target->IsDead()) {
			continue;
		}
		if (IsCollision(playerAABB, target->GetAABB())) {
			hookStitch_->TryStitch(target);
		}
	}
}

void GameScene::CheckPlayerHits() {
	if (!player_ || player_->IsDead() || player_->IsInvincible() || player_->IsDashing() || hookStitch_->IsCinching()) {
		return;
	}
	AABB2 playerAABB = player_->GetAABB();
	for (StitchTarget* target : targets_) {
		if (!target || target->IsDead()) {
			continue;
		}
		if (target->GetKind() != StitchTarget::Kind::kBoss && target->GetKind() != StitchTarget::Kind::kFodder) {
			continue;
		}
		if (IsCollision(playerAABB, target->GetAABB())) {
			player_->OnDamaged();
			hookStitch_->Clear();
			break;
		}
	}
}

void GameScene::PushPlayerOutOfBosses() {
	AABB2 p = player_->GetAABB();
	Vector2 pos = player_->GetPosition();
	for (StitchTarget* target : targets_) {
		if (!target || target->IsDead()) {
			continue;
		}
		if (target->GetKind() != StitchTarget::Kind::kBoss && target->GetKind() != StitchTarget::Kind::kFodder) {
			continue;
		}
		AABB2 b = target->GetAABB();
		if (!IsCollision(p, b)) {
			continue;
		}
		const float pushRight = b.max.x - p.min.x;
		const float pushLeft = p.max.x - b.min.x;
		const float pushDown = b.max.y - p.min.y;
		const float pushUp = p.max.y - b.min.y;
		float dx = (pushRight < pushLeft) ? pushRight : -pushLeft;
		float dy = (pushDown < pushUp) ? pushDown : -pushUp;
		if (std::abs(dx) <= std::abs(dy)) {
			pos.x += dx + (dx >= 0.0f ? 2.0f : -2.0f);
		} else {
			pos.y += dy + (dy >= 0.0f ? 2.0f : -2.0f);
		}
		player_->SetPosition(pos);
		p = player_->GetAABB();
	}
}

void GameScene::Update() {
	if (phase_ == Phase::kPlay) {
		if (enterWait_ > 0) {
			--enterWait_;
			UpdateCamera();
			return;
		}

		player_->Update(mapChipField_);
		BlockClosedDoors();
		TryEnterNextArea();

		if (bossA_) {
			bossA_->Update(mapChipField_);
		}
		if (bossB_) {
			bossB_->Update(mapChipField_);
		}
		if (stakeL_) {
			stakeL_->Update();
		}
		if (stakeR_) {
			stakeR_->Update();
		}
		for (Enemy* enemy : fodder_) {
			if (enemy) {
				enemy->Update(mapChipField_);
			}
		}

		CheckStitchOverlaps();
		hookStitch_->Update(player_, targets_);
		if (player_->InvincibleJustEnded()) {
			PushPlayerOutOfBosses();
		}
		CheckPlayerHits();
		CheckSectionClear();
		UpdateCamera();

		if (player_->IsDead()) {
			phase_ = Phase::kGameOver;
		}
	} else if (phase_ == Phase::kGameOver) {
		Input* input = Input::GetInstance();
		if (input->TriggerKey(DIK_R) || input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)) {
			RespawnFromSave();
		}
	} else if (phase_ == Phase::kClear) {
		Input* input = Input::GetInstance();
		if (input->TriggerKey(DIK_R) || input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)) {
			BuildWorld();
		}
	}
}

void GameScene::DrawBlocks() {
	const Vector2 cam = camera_.GetOffset();
	for (size_t i = 0; i < blockSprites_.size(); ++i) {
		Sprite* sprite = blockSprites_[i];
		if (!sprite) {
			continue;
		}
		sprite->SetRotation(0.0f);
		sprite->SetColor({0.22f, 0.24f, 0.28f, 1.0f});
		sprite->SetPosition({blockPositions_[i].x - cam.x, blockPositions_[i].y - cam.y});
		sprite->SetSize({MapChipField::kTileWidth, MapChipField::kTileHeight});
		sprite->Draw();
	}
}

void GameScene::DrawDoors() {
	const Vector2 cam = camera_.GetOffset();
	for (size_t i = 0; i < world_.doors.size() && i < doorSprites_.size(); ++i) {
		const DoorDesc& door = world_.doors[i];
		Sprite* sprite = doorSprites_[i];
		if (!sprite) {
			continue;
		}
		const bool opened = IsSectionCleared(door.openAfterSection);
		sprite->SetRotation(0.0f);
		sprite->SetColor(opened ? Vector4{0.25f, 0.55f, 0.3f, 0.45f} : Vector4{0.55f, 0.35f, 0.2f, 1.0f});
		sprite->SetPosition({door.x - cam.x, door.y - cam.y});
		sprite->SetSize({door.width, door.height});
		sprite->Draw();
	}
}

void GameScene::DrawHp() {
	const Vector2 cam = camera_.GetOffset();
	for (int i = 0; i < player_->GetMaxHp(); ++i) {
		Sprite* sprite = playerHpSprites_[i];
		if (!sprite) {
			continue;
		}
		sprite->SetRotation(0.0f);
		sprite->SetPosition({24.0f + static_cast<float>(i) * 36.0f, 20.0f});
		sprite->SetSize({28.0f, 28.0f});
		sprite->SetColor(i < player_->GetHp() ? Vector4{0.35f, 0.85f, 1.0f, 1.0f} : Vector4{0.2f, 0.25f, 0.3f, 1.0f});
		sprite->Draw();
	}

	auto drawBossBar = [&](Boss* boss, Sprite* back, Sprite* fill) {
		if (!boss || boss->IsDead() || !back || !fill) {
			return;
		}
		const float barW = boss->GetSize().x;
		const float x = boss->GetPosition().x - cam.x;
		const float y = boss->GetPosition().y - 18.0f - cam.y;
		const float ratio = static_cast<float>(boss->GetHp()) / static_cast<float>(boss->GetMaxHp());
		back->SetRotation(0.0f);
		back->SetColor({0.1f, 0.1f, 0.1f, 0.9f});
		back->SetPosition({x, y});
		back->SetSize({barW, 10.0f});
		back->Draw();
		fill->SetRotation(0.0f);
		fill->SetColor({0.9f, 0.2f, 0.2f, 1.0f});
		fill->SetPosition({x, y});
		fill->SetSize({barW * ratio, 10.0f});
		fill->Draw();
	};
	if (cameraSection_ >= 4) {
		drawBossBar(bossA_, bossAHpBack_, bossAHpFill_);
		drawBossBar(bossB_, bossBHpBack_, bossBHpFill_);
	}

	for (size_t i = 0; i < fodder_.size() && i < static_cast<size_t>(kMaxFodderUi); ++i) {
		Enemy* enemy = fodder_[i];
		if (!enemy || enemy->IsDead()) {
			continue;
		}
		const float barW = enemy->GetSize().x;
		const float x = enemy->GetPosition().x - cam.x;
		const float y = enemy->GetPosition().y - 14.0f - cam.y;
		const float ratio = static_cast<float>(enemy->GetHp()) / static_cast<float>(enemy->GetMaxHp());
		fodderHpBack_[i]->SetRotation(0.0f);
		fodderHpBack_[i]->SetColor({0.1f, 0.1f, 0.1f, 0.9f});
		fodderHpBack_[i]->SetPosition({x, y});
		fodderHpBack_[i]->SetSize({barW, 8.0f});
		fodderHpBack_[i]->Draw();
		fodderHpFill_[i]->SetRotation(0.0f);
		fodderHpFill_[i]->SetColor({0.35f, 0.9f, 0.35f, 1.0f});
		fodderHpFill_[i]->SetPosition({x, y});
		fodderHpFill_[i]->SetSize({barW * ratio, 8.0f});
		fodderHpFill_[i]->Draw();
	}
}

void GameScene::Draw() {
	const Vector2 cam = camera_.GetOffset();

	Sprite::PreDraw();
	if (backSprite_) {
		backSprite_->SetColor({0.08f, 0.09f, 0.12f, 1.0f});
		backSprite_->SetPosition({0.0f, 0.0f});
		backSprite_->SetSize({1280.0f, 720.0f});
		backSprite_->Draw();
	}
	DrawBlocks();
	DrawDoors();
	if (stakeL_) {
		stakeL_->Draw(cam);
	}
	if (stakeR_) {
		stakeR_->Draw(cam);
	}
	if (bossA_) {
		bossA_->Draw(cam);
	}
	if (bossB_) {
		bossB_->Draw(cam);
	}
	for (Enemy* enemy : fodder_) {
		if (enemy) {
			enemy->Draw(cam);
		}
	}
	player_->Draw(cam);
	hookStitch_->Draw(player_, cam);
	DrawHp();
	Sprite::PostDraw();
}