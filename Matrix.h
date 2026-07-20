//[matirix.h]
#pragma once
#include "KamataEngine.h"

// 単位行列の作成
KamataEngine::Matrix4x4 MakeIdentity4x4();

// X軸回転行列
KamataEngine::Matrix4x4 MakeRotateXMatrix(float radian);

// Y軸回転行列
KamataEngine::Matrix4x4 MakeRotateYMatrix(float radian);

// Z軸回転行列
KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian);

// スケーリング行列
KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale);

// 平行移動行列
KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate);

// 行列の積
KamataEngine::Matrix4x4 Multiply(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

// 3次元アフィン変換行列の生成
KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotate, const KamataEngine::Vector3& translate);
