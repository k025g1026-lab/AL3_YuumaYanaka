#include "GameScene.h"
#include <KamataEngine.h>
#include <Windows.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize(L"GC1A_08_ヤナカ_ユウマ_AL2");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	GameScene* gameScene = new GameScene();
	gameScene->Initialize();

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		gameScene->Update();

		dxCommon->PreDraw();

		gameScene->Draw();

		// ここに描画処理を追加する
		dxCommon->PostDraw();
	}

	delete gameScene;
	gameScene = nullptr;

	return 0;
}