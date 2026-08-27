// [TitleScene.cpp]
#include "TitleScene.h"
#include "Matrix.h"
#include <cassert>

using namespace KamataEngine;

TitleScene::~TitleScene() {
	delete modelTitleFont_;
	modelTitleFont_ = nullptr;
}

void TitleScene::Initialize() {
	camera_.Initialize();
	camera_.farZ = 1000.0f;

	modelTitleFont_ = Model::CreateFromOBJ("titleFont", true);

	worldTransformTitleFont_.Initialize();
	worldTransformTitleFont_.translation_ = {0.0f, 2.0f, 0.0f};
	worldTransformTitleFont_.scale_ = {1.0f, 1.0f, 1.0f};
}

void TitleScene::Update() {
	Input* input = Input::GetInstance();
	if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN) || input->IsTriggerMouse(0)) {
		finished_ = true;
	}

	worldTransformTitleFont_.matWorld_ = MakeAffineMatrix(worldTransformTitleFont_.scale_, worldTransformTitleFont_.rotation_, worldTransformTitleFont_.translation_);
	worldTransformTitleFont_.TransferMatrix();
}

void TitleScene::Draw() {
	Model::PreDraw();
	modelTitleFont_->Draw(worldTransformTitleFont_, camera_);
	Model::PostDraw();
}