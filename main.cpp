// [main.cpp]
#include "GameScene.h"
#include "TitleScene.h"
#include <KamataEngine.h>
#include <Windows.h>

using namespace KamataEngine;

// シーン（型）
enum class Scene {
	kUnknown = 0,
	kTitle,
	kGame,
};

// 現在シーン（型）
Scene scene = Scene::kUnknown;

// シーンのポインタ
TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;

/// <summary>
/// シーン切り替え処理
/// </summary>
void ChangeScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kGame;
			// 旧シーンの解放
			delete titleScene;
			titleScene = nullptr;
			// 新シーンの生成と初期化
			gameScene = new GameScene();
			gameScene->Initialize();
		}
		break;
	case Scene::kGame:
		if (gameScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;
			// 旧シーンの解放
			delete gameScene;
			gameScene = nullptr;
			// 新シーンの生成と初期化
			titleScene = new TitleScene();
			titleScene->Initialize();
		}
		break;
	}
}

/// <summary>
/// 現在シーンの更新
/// </summary>
void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	}
}

/// <summary>
/// 現在シーンの描画
/// </summary>
void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize(L"GC2A_06_ヤナカ_ユウマ_AL3");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ImGuiManagerインスタンスの取得
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	// 最初のシーンの初期化
	scene = Scene::kTitle;
	titleScene = new TitleScene();
	titleScene->Initialize();

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		// ImGui受付開始
		imguiManager->Begin();

		// シーン切り替え
		ChangeScene();
		// 現在シーン更新
		UpdateScene();

		// ImGui受付終了
		imguiManager->End();

		dxCommon->PreDraw();

		// 現在シーンの描画
		DrawScene();

		// 軸方向表示の描画
		AxisIndicator::GetInstance()->Draw();

		// ImGui描画
		imguiManager->Draw();

		// ここに描画処理を追加する.
		dxCommon->PostDraw();
	}

	// シーン解放処理
	delete titleScene;
	titleScene = nullptr;
	delete gameScene;
	gameScene = nullptr;

	return 0;
}