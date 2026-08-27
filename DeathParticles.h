// [DeathParticles.h]
#pragma once
#include "KamataEngine.h"
#include <array>
#include <numbers>

/// <summary>
/// デス演出用パーティクル
/// </summary>
class DeathParticles {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

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
	bool IsFinished() const { return isFinished_; }

private:
	KamataEngine::Model* model_ = nullptr;
	const KamataEngine::Camera* camera_ = nullptr;

	static inline const uint32_t kNumParticles = 8;
	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_;

	static inline const float kDuration = 1.0f;
	static inline const float kSpeed = 0.1f;
	static inline const float kAngleUnit = 2.0f * std::numbers::pi_v<float> / kNumParticles;

	bool isFinished_ = false;
	float counter_ = 0.0f;

	KamataEngine::ObjectColor objectColor_;
	// RGBA順
	KamataEngine::Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
};