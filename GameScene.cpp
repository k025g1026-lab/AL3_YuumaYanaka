// [GameScene.cpp]
#include "GameScene.h"
#include <cmath>

// BuildWorld         : 1本のワールドを生成。最初は1面カメラ
// EnterSection       : 扉タッチ後。カメラ・移動範囲・セーブを次の部屋へ
// UpdateCamera       : 今の部屋の左端を固定カメラにする
// CheckSectionClear  : その部屋の雑魚が全滅したら扉解放フラグを立てる
// BlockClosedDoors   : 未クリアの扉は壁
// TryEnterNextArea   : 開いた扉に重なったら次エリアへ
// ResetSectionEnemies: 死亡復帰時、今の部屋の敵だけ初期化。前の部屋の撃破は残す
// RespawnFromSave    : ボス死は3面開始地点へ

using namespace KamataEngine;

GameScene::~GameScene() {
	ClearCurrentActors();
	delete player_;
	delete hookStitch_;
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
	targets_.clear();
}

void GameScene::Initialize() {
	whiteTexture_ = TextureManager::Load("white.png");

	backSprite_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	groundSprite_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});

	for (int i = 0; i < 3; ++i) {
		playerHpSprites_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
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

	BuildWorld();
}

Vector2 GameScene::GetSectionSpawn(int section) const {
	if (section <= 1) {
		return world_.spawn1;
	}
	if (section == 2) {
		return world_.spawn2;
	}
	if (section == 3) {
		return world_.spawn3;
	}
	return world_.spawnBoss;
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
	UpdateCamera();
}

void GameScene::UpdateCamera() { camera_.SetFixed({RoomLeft(cameraSection_), 0.0f}); }

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

	player_->Reset(world_.spawn1);
	player_->SetMapBounds(RoomLeft(1), RoomRight(1));

	for (const FodderSpawn& spawn : world_.fodder) {
		Enemy* enemy = new Enemy();
		enemy->Initialize(whiteTexture_, spawn.position, spawn.minX, spawn.maxX);
		fodder_.push_back(enemy);
		fodderSection_.push_back(spawn.section);
		targets_.push_back(enemy);
	}

	const float bossLeft = RoomLeft(4);
	bossA_ = new Boss();
	bossA_->Initialize(whiteTexture_, {bossLeft + 360.0f, 400.0f}, {0.95f, 0.25f, 0.25f, 1.0f}, bossLeft + 220.0f, bossLeft + 560.0f);
	bossB_ = new Boss();
	bossB_->Initialize(whiteTexture_, {bossLeft + 760.0f, 400.0f}, {0.95f, 0.55f, 0.2f, 1.0f}, bossLeft + 640.0f, bossLeft + 980.0f);
	stakeL_ = new Stake();
	stakeL_->Initialize(whiteTexture_, {bossLeft + 70.0f, world_.groundY - 160.0f});
	stakeR_ = new Stake();
	stakeR_->Initialize(whiteTexture_, {bossLeft + 1180.0f, world_.groundY - 160.0f});
	targets_.push_back(bossA_);
	targets_.push_back(bossB_);
	targets_.push_back(stakeL_);
	targets_.push_back(stakeR_);

	phase_ = Phase::kPlay;
	UpdateCamera();
}

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

	for (const DoorDesc& door : world_.doors) {
		if (IsSectionCleared(door.openAfterSection)) {
			continue;
		}
		AABB2 d;
		d.min = {door.x, world_.groundY - door.height};
		d.max = {door.x + door.width, world_.groundY};
		if (!IsCollision(p, d)) {
			continue;
		}
		pos.x = d.min.x - (p.max.x - p.min.x);
		player_->SetPosition(pos);
		p = player_->GetAABB();
	}
}

void GameScene::TryEnterNextArea() {
	AABB2 p = player_->GetAABB();
	for (const DoorDesc& door : world_.doors) {
		if (!IsSectionCleared(door.openAfterSection)) {
			continue;
		}
		if (door.leadsToSection != cameraSection_ + 1) {
			continue;
		}
		AABB2 d;
		d.min = {door.x, world_.groundY - door.height};
		d.max = {door.x + door.width, world_.groundY};
		if (IsCollision(p, d)) {
			EnterSection(door.leadsToSection);
			break;
		}
	}
}

void GameScene::ResetSectionEnemies(int section) {
	if (section >= 4) {
		const float bossLeft = RoomLeft(4);
		delete bossA_;
		delete bossB_;
		bossA_ = new Boss();
		bossB_ = new Boss();
		bossA_->Initialize(whiteTexture_, {bossLeft + 360.0f, 400.0f}, {0.95f, 0.25f, 0.25f, 1.0f}, bossLeft + 220.0f, bossLeft + 560.0f);
		bossB_->Initialize(whiteTexture_, {bossLeft + 760.0f, 400.0f}, {0.95f, 0.55f, 0.2f, 1.0f}, bossLeft + 640.0f, bossLeft + 980.0f);
	} else {
		for (size_t i = 0; i < fodder_.size(); ++i) {
			if (fodderSection_[i] != section) {
				continue;
			}
			delete fodder_[i];
			const FodderSpawn& spawn = world_.fodder[i];
			fodder_[i] = new Enemy();
			fodder_[i]->Initialize(whiteTexture_, spawn.position, spawn.minX, spawn.maxX);
		}
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
		if (pos.y + (p.max.y - p.min.y) > world_.groundY) {
			pos.y = world_.groundY - (p.max.y - p.min.y);
		}
		player_->SetPosition(pos);
		p = player_->GetAABB();
	}
}

void GameScene::Update() {
	if (phase_ == Phase::kPlay) {
		player_->Update(world_.groundY);
		BlockClosedDoors();
		TryEnterNextArea();

		if (bossA_) {
			bossA_->Update();
		}
		if (bossB_) {
			bossB_->Update();
		}
		if (stakeL_) {
			stakeL_->Update();
		}
		if (stakeR_) {
			stakeR_->Update();
		}
		for (Enemy* enemy : fodder_) {
			if (enemy) {
				enemy->Update();
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
		sprite->SetPosition({door.x - cam.x, world_.groundY - door.height - cam.y});
		sprite->SetSize({door.width, door.height});
		sprite->Draw();
	}
}

void GameScene::DrawHp() {
	const Vector2 cam = camera_.GetOffset();

	const int playerHp = player_->GetHp();
	const int playerMax = player_->GetMaxHp();
	for (int i = 0; i < playerMax; ++i) {
		Sprite* sprite = playerHpSprites_[i];
		if (!sprite) {
			continue;
		}
		sprite->SetRotation(0.0f);
		sprite->SetPosition({24.0f + static_cast<float>(i) * 36.0f, 20.0f});
		sprite->SetSize({28.0f, 28.0f});
		sprite->SetColor(i < playerHp ? Vector4{0.35f, 0.85f, 1.0f, 1.0f} : Vector4{0.2f, 0.25f, 0.3f, 1.0f});
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
		Sprite* back = fodderHpBack_[i];
		Sprite* fill = fodderHpFill_[i];
		if (!enemy || enemy->IsDead() || !back || !fill) {
			continue;
		}
		const float barW = enemy->GetSize().x;
		const float x = enemy->GetPosition().x - cam.x;
		const float y = enemy->GetPosition().y - 14.0f - cam.y;
		const float ratio = static_cast<float>(enemy->GetHp()) / static_cast<float>(enemy->GetMaxHp());
		back->SetRotation(0.0f);
		back->SetColor({0.1f, 0.1f, 0.1f, 0.9f});
		back->SetPosition({x, y});
		back->SetSize({barW, 8.0f});
		back->Draw();
		fill->SetRotation(0.0f);
		fill->SetColor({0.35f, 0.9f, 0.35f, 1.0f});
		fill->SetPosition({x, y});
		fill->SetSize({barW * ratio, 8.0f});
		fill->Draw();
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
	if (groundSprite_) {
		groundSprite_->SetColor({0.18f, 0.18f, 0.22f, 1.0f});
		groundSprite_->SetPosition({RoomLeft(cameraSection_) - cam.x, world_.groundY - cam.y});
		groundSprite_->SetSize({1280.0f, 80.0f});
		groundSprite_->Draw();
	}

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