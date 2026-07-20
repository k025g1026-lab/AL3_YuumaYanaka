// [main.cpp]
#include "GameScene.h"
#include <KamataEngine.h>
#include <Windows.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize(L"GC2A_06_ヤナカ_ユウマ_AL3");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ImGuiManagerインスタンスの取得
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	GameScene* gameScene = new GameScene();
	gameScene->Initialize();

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		// ImGui受付開始
		imguiManager->Begin();

		gameScene->Update();

		// ImGui受付終了
		imguiManager->End();

		dxCommon->PreDraw();

		gameScene->Draw();

		// 軸方向表示の描画
		AxisIndicator::GetInstance()->Draw();

		// ImGui描画
		imguiManager->Draw();

		// ここに描画処理を追加する.
		dxCommon->PostDraw();
	}

	delete gameScene;
	gameScene = nullptr;

	return 0;
}