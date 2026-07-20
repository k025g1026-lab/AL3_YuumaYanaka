//[matrix.cpp]
#include "Matrix.h"
using namespace KamataEngine;
KamataEngine::Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	return result;
}

KamataEngine::Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = std::sin(radian);
	result.m[2][1] = -std::sin(radian);
	result.m[2][2] = std::cos(radian);
	return result;
}

KamataEngine::Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = std::cos(radian);
	result.m[0][2] = -std::sin(radian);
	result.m[2][0] = std::sin(radian);
	result.m[2][2] = std::cos(radian);
	return result;
}

KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);
	result.m[1][0] = -std::sin(radian);
	result.m[1][1] = std::cos(radian);
	return result;
}

KamataEngine::Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	return result;
}

KamataEngine::Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	return result;
}

KamataEngine::Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

KamataEngine::Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	// XYZ順の回転行列合成（サンプルに準拠）
	Matrix4x4 rotateMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));
	// 回転 * スケール
	Matrix4x4 rotateScaleMatrix = Multiply(scaleMatrix, rotateMatrix);
	// 平行移動 * (回転 * スケール)
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
	Matrix4x4 result = Multiply(rotateScaleMatrix, translateMatrix);
	return result;
}
