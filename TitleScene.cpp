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
	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 1000.0f;

	// タイトルフォントモデル生成
	modelTitleFont_ = Model::CreateFromOBJ("titleFont", true);

	// ワールド変換の初期化
	worldTransformTitleFont_.Initialize();
	worldTransformTitleFont_.translation_ = {0.0f, 2.0f, 0.0f};
	worldTransformTitleFont_.scale_ = {1.0f, 1.0f, 1.0f};
}

void TitleScene::Update() {
	// スペースキーで終了
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
		finished_ = true;
	}

	// ワールド変換の更新
	worldTransformTitleFont_.matWorld_ = MakeAffineMatrix(worldTransformTitleFont_.scale_, worldTransformTitleFont_.rotation_, worldTransformTitleFont_.translation_);
	worldTransformTitleFont_.TransferMatrix();
}

void TitleScene::Draw() {
	// 3Dモデル描画前処理
	Model::PreDraw();

	// タイトルフォントの描画
	modelTitleFont_->Draw(worldTransformTitleFont_, camera_);

	// 3Dモデル描画後処理
	Model::PostDraw();
}