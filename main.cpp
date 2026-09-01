// [main.cpp]
#include "GameScene.h"
#include <KamataEngine.h>
#include <Windows.h>

using namespace KamataEngine;

// エントリーポイント。シーン切り替えはまだ GameScene のみ。
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	KamataEngine::Initialize(L"LASH_Stitch");

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
		dxCommon->PostDraw();
	}

	delete gameScene;
	return 0;
}