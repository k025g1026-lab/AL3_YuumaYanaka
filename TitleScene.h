// [TitleScene.h]
#pragma once
#include "KamataEngine.h"

/// <summary>
/// タイトルシーン
/// </summary>
class TitleScene {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 終了フラグのgetter
	/// </summary>
	bool IsFinished() const { return finished_; }

	/// <summary>
	/// デストラクタ
	/// </summary>
	~TitleScene();

private:
	// 終了フラグ
	bool finished_ = false;

	// カメラ
	KamataEngine::Camera camera_;

	// タイトルフォントモデル
	KamataEngine::Model* modelTitleFont_ = nullptr;

	// タイトルフォントのワールド変換
	KamataEngine::WorldTransform worldTransformTitleFont_;
};