// [Player.cpp]
#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include "Matrix.h"
#include <algorithm>
#include <cassert>
#include <numbers>

void Player::Initialize(KamataEngine::Model* model, const KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NULLポインタチェック
	assert(model);
	assert(camera);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 初期回転
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	turnFirstRotationY_ = worldTransform_.rotation_.y;
}

void Player::Update() {
	// ①移動入力
	InputMove();

	// ②移動量を加味して衝突判定する
	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.movement = velocity_;
	// マップ衝突チェック
	MapCollision(collisionMapInfo);

	// ③判定結果を反映して移動させる
	MoveAfterCollision(collisionMapInfo);

	// ④天井に接触している場合の処理
	OnCollisionCeiling(collisionMapInfo);

	// ⑤壁に接触している場合の処理（今回は未実装）

	// ⑥接地状態の切り替え（今回は未実装）

	// ⑦旋回制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		float t = 1.0f - (turnTimer_ / kTimeTurn);
		worldTransform_.rotation_.y = (1.0f - t) * turnFirstRotationY_ + t * destinationRotationY;
	}

	// ⑧行列計算
	// アフィン変換行列を計算してメンバ変数に代入
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	// 定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Player::Draw() {
	// 自キャラの描画
	model_->Draw(worldTransform_, *camera_);
}

void Player::InputMove() {
	// 接地状態
	if (onGround_) {
		// 左右移動操作
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT) || KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT)) {
			if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT)) {
				if (lrDirection_ != LRDirection::kRight) {
					velocity_.x *= (1.0f - kAttenuation);
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
				lrDirection_ = LRDirection::kRight;
				velocity_.x += kAcceleration;
			} else if (KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT)) {
				if (lrDirection_ != LRDirection::kLeft) {
					velocity_.x *= (1.0f - kAttenuation);
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
				lrDirection_ = LRDirection::kLeft;
				velocity_.x -= kAcceleration;
			}
		} else {
			velocity_.x *= (1.0f - kAttenuation);
		}

		// ジャンプ入力
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_.y += kJumpAcceleration;
			onGround_ = false;
		}
	} else {
		// 落下速度
		velocity_.y += -kGravityAcceleration;
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
	// 地面との当たり判定（仮）
	bool landing = false;
	if (velocity_.y < 0) {
		if (worldTransform_.translation_.y <= 1.0f) {
			landing = true;
		}
	}

	// 接地 / 空中切り替え
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		}
	} else {
		// 着地
		if (landing) {
			// めり込み排斥
			worldTransform_.translation_.y = 1.0f;
			// 摩擦で横方向速度が減衰する
			velocity_.x *= (1.0f - kAttenuation);
			// 下方向速度をリセット
			velocity_.y = 0.0f;
			// 接地状態に移行
			onGround_ = true;
		}
	}

	// 速度制限（左右）
	velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
}

void Player::MapCollision(CollisionMapInfo& info) {
	MapCollisionUp(info);
	MapCollisionDown(info);
	MapCollisionRight(info);
	MapCollisionLeft(info);
}

void Player::MapCollisionUp(CollisionMapInfo& info) {
	// 上昇あり？
	if (info.movement.y <= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<KamataEngine::Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		KamataEngine::Vector3 movedCenter = {worldTransform_.translation_.x + info.movement.x, worldTransform_.translation_.y + info.movement.y, worldTransform_.translation_.z + info.movement.z};
		positionsNew[i] = CornerPosition(movedCenter, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]); // 移動後の自キャラ上端座標
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		// Y移動量 = (ブロック下端 - 移動前自キャラ座標) - (自キャラの半径 + 微小な余白)
		float yMovement = (rect.bottom - worldTransform_.translation_.y) - (kHeight / 2.0f + kBlank);
		info.movement.y = std::max(0.0f, yMovement);
		// 天井に当たったことを記録する
		info.ceiling = true;
	}
}

void Player::MapCollisionDown(CollisionMapInfo& info) {
	// 今回は未実装
	(void)info;
}

void Player::MapCollisionRight(CollisionMapInfo& info) {
	// 今回は未実装
	(void)info;
}

void Player::MapCollisionLeft(CollisionMapInfo& info) {
	// 今回は未実装
	(void)info;
}

void Player::MoveAfterCollision(const CollisionMapInfo& info) {
	// 移動
	worldTransform_.translation_.x += info.movement.x;
	worldTransform_.translation_.y += info.movement.y;
	worldTransform_.translation_.z += info.movement.z;
}

void Player::OnCollisionCeiling(const CollisionMapInfo& info) {
	// 天井に当たった？
	if (info.ceiling) {
		// DebugTextが使えない環境向けにコメントアウト（必要なら有効化）
		// DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0;
	}
}

KamataEngine::Vector3 Player::CornerPosition(const KamataEngine::Vector3& center, Corner corner) {
	// オフセットテーブル
	KamataEngine::Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, // kRightBottom
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, // kLeftBottom
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0}, // kRightTop
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0}, // kLeftTop
	};
	return {center.x + offsetTable[static_cast<uint32_t>(corner)].x, center.y + offsetTable[static_cast<uint32_t>(corner)].y, center.z + offsetTable[static_cast<uint32_t>(corner)].z};
}