// [GameScene.cpp]
#include "GameScene.h"
#include <cmath>

using namespace KamataEngine;

GameScene::~GameScene() {
	delete player_;
	delete bossA_;
	delete bossB_;
	delete stakeL_;
	delete stakeR_;
	delete hookStitch_;
}

void GameScene::Initialize() {
	whiteTexture_ = TextureManager::Load("white.png");

	backSprite_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	groundSprite_ = Sprite::Create(whiteTexture_, {0.0f, kGroundY});

	for (int i = 0; i < 3; ++i) {
		playerHpSprites_[i] = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	}
	bossAHpBack_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	bossAHpFill_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	bossBHpBack_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});
	bossBHpFill_ = Sprite::Create(whiteTexture_, {0.0f, 0.0f});

	player_ = new Player();
	player_->Initialize(whiteTexture_);
	player_->Reset({180.0f, 500.0f});

	bossA_ = new Boss();
	bossA_->Initialize(whiteTexture_, {360.0f, 400.0f}, {0.95f, 0.25f, 0.25f, 1.0f}, 220.0f, 560.0f);

	bossB_ = new Boss();
	bossB_->Initialize(whiteTexture_, {780.0f, 400.0f}, {0.95f, 0.55f, 0.2f, 1.0f}, 640.0f, 980.0f);

	stakeL_ = new Stake();
	stakeL_->Initialize(whiteTexture_, {70.0f, kGroundY - 160.0f});

	stakeR_ = new Stake();
	stakeR_->Initialize(whiteTexture_, {1180.0f, kGroundY - 160.0f});

	hookStitch_ = new HookStitch();
	hookStitch_->Initialize(whiteTexture_);

	targets_ = {bossA_, bossB_, stakeL_, stakeR_};
	phase_ = Phase::kPlay;
}

void GameScene::CheckStitchOverlaps() {
	if (!player_ || !hookStitch_) {
		return;
	}

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
		if (!target || target->GetKind() != StitchTarget::Kind::kBoss || target->IsDead()) {
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
	if (!player_) {
		return;
	}

	AABB2 p = player_->GetAABB();
	Vector2 pos = player_->GetPosition();

	for (StitchTarget* target : targets_) {
		if (!target || target->GetKind() != StitchTarget::Kind::kBoss || target->IsDead()) {
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

		if (pos.y + (p.max.y - p.min.y) > kGroundY) {
			pos.y = kGroundY - (p.max.y - p.min.y);
		}

		player_->SetPosition(pos);
		p = player_->GetAABB();
	}
}

void GameScene::Update() {
	if (phase_ == Phase::kPlay) {
		player_->Update(kGroundY);
		bossA_->Update();
		bossB_->Update();
		stakeL_->Update();
		stakeR_->Update();

		CheckStitchOverlaps();
		hookStitch_->Update(player_, targets_);

		if (player_->InvincibleJustEnded()) {
			PushPlayerOutOfBosses();
		}

		CheckPlayerHits();

		if (bossA_->IsDead() && bossB_->IsDead()) {
			phase_ = Phase::kClear;
		}
		if (player_->IsDead()) {
			phase_ = Phase::kGameOver;
		}
	} else {
		Input* input = Input::GetInstance();
		if (input->TriggerKey(DIK_R) || input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)) {
			player_->Reset({180.0f, 500.0f});
			delete bossA_;
			delete bossB_;
			bossA_ = new Boss();
			bossB_ = new Boss();
			bossA_->Initialize(whiteTexture_, {360.0f, 400.0f}, {0.95f, 0.25f, 0.25f, 1.0f}, 220.0f, 560.0f);
			bossB_->Initialize(whiteTexture_, {780.0f, 400.0f}, {0.95f, 0.55f, 0.2f, 1.0f}, 640.0f, 980.0f);
			hookStitch_->Clear();
			targets_ = {bossA_, bossB_, stakeL_, stakeR_};
			phase_ = Phase::kPlay;
		}
	}
}

void GameScene::DrawHp() {
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
		if (i < playerHp) {
			sprite->SetColor({0.35f, 0.85f, 1.0f, 1.0f});
		} else {
			sprite->SetColor({0.2f, 0.25f, 0.3f, 1.0f});
		}
		sprite->Draw();
	}

	auto drawBar = [](Boss* boss, Sprite* back, Sprite* fill) {
		if (!boss || boss->IsDead() || !back || !fill) {
			return;
		}
		const float barW = boss->GetSize().x;
		const float barH = 10.0f;
		const float x = boss->GetPosition().x;
		const float y = boss->GetPosition().y - 18.0f;
		const float ratio = static_cast<float>(boss->GetHp()) / static_cast<float>(boss->GetMaxHp());

		back->SetRotation(0.0f);
		back->SetColor({0.1f, 0.1f, 0.1f, 0.9f});
		back->SetPosition({x, y});
		back->SetSize({barW, barH});
		back->Draw();

		fill->SetRotation(0.0f);
		fill->SetColor({0.9f, 0.2f, 0.2f, 1.0f});
		fill->SetPosition({x, y});
		fill->SetSize({barW * ratio, barH});
		fill->Draw();
	};

	drawBar(bossA_, bossAHpBack_, bossAHpFill_);
	drawBar(bossB_, bossBHpBack_, bossBHpFill_);
}

void GameScene::Draw() {
	Sprite::PreDraw();

	if (backSprite_) {
		backSprite_->SetColor({0.08f, 0.09f, 0.12f, 1.0f});
		backSprite_->SetPosition({0.0f, 0.0f});
		backSprite_->SetSize({1280.0f, 720.0f});
		backSprite_->Draw();
	}
	if (groundSprite_) {
		groundSprite_->SetColor({0.18f, 0.18f, 0.22f, 1.0f});
		groundSprite_->SetPosition({0.0f, kGroundY});
		groundSprite_->SetSize({1280.0f, 80.0f});
		groundSprite_->Draw();
	}

	stakeL_->Draw();
	stakeR_->Draw();
	bossA_->Draw();
	bossB_->Draw();
	player_->Draw();
	hookStitch_->Draw(player_);
	DrawHp();

	Sprite::PostDraw();
}