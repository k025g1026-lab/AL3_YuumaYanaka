// [GameScene.cpp]
#include "GameScene.h"

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

	player_ = new Player();
	player_->Initialize(whiteTexture_);
	player_->Reset({180.0f, 500.0f});

	bossA_ = new Boss();
	bossA_->Initialize(whiteTexture_, {420.0f, 400.0f}, {0.95f, 0.25f, 0.25f, 1.0f});

	bossB_ = new Boss();
	bossB_->Initialize(whiteTexture_, {760.0f, 400.0f}, {0.95f, 0.55f, 0.2f, 1.0f});

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

void GameScene::Update() {
	if (phase_ == Phase::kPlay) {
		player_->Update(kGroundY);
		bossA_->Update();
		bossB_->Update();
		stakeL_->Update();
		stakeR_->Update();

		CheckStitchOverlaps();
		hookStitch_->Update(player_, targets_);
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
			bossA_->Initialize(whiteTexture_, {420.0f, 400.0f}, {0.95f, 0.25f, 0.25f, 1.0f});
			bossB_->Initialize(whiteTexture_, {760.0f, 400.0f}, {0.95f, 0.55f, 0.2f, 1.0f});
			hookStitch_->Clear();
			targets_ = {bossA_, bossB_, stakeL_, stakeR_};
			phase_ = Phase::kPlay;
		}
	}
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

	Sprite::PostDraw();
}