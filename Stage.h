// [Stage.h]
#pragma once
#include "KamataEngine.h"
#include <vector>

enum class StageId {
	kStage1,
	kStage2,
	kStage3,
	kBoss,
};

struct FodderSpawn {
	KamataEngine::Vector2 position{};
	float minX = 0.0f;
	float maxX = 0.0f;
	int section = 1;
};

struct DoorDesc {
	float x = 0.0f;
	float width = 48.0f;
	float height = 220.0f;
	int openAfterSection = 1;
	int leadsToSection = 2;
};

struct WorldDesc {
	float groundY = 640.0f;
	float mapTop = 0.0f;
	float mapBottom = 720.0f;
	float roomW = 1280.0f;

	KamataEngine::Vector2 spawn1{180.0f, 500.0f};
	KamataEngine::Vector2 spawn2{1460.0f, 500.0f};
	KamataEngine::Vector2 spawn3{2740.0f, 500.0f};
	KamataEngine::Vector2 spawnBoss{4020.0f, 500.0f};

	std::vector<FodderSpawn> fodder;
	std::vector<DoorDesc> doors;
};

inline float RoomLeft(int section) { return static_cast<float>(section - 1) * 1280.0f; }

inline float RoomRight(int section) { return static_cast<float>(section) * 1280.0f; }

inline WorldDesc GetWorldDesc() {
	WorldDesc world;
	world.fodder = {
	    {{520.0f, 520.0f},  400.0f,  720.0f,  1},
        {{900.0f, 520.0f},  780.0f,  1080.0f, 1},
        {{1680.0f, 520.0f}, 1560.0f, 1880.0f, 2},
        {{2180.0f, 520.0f}, 2040.0f, 2360.0f, 2},
	    {{2960.0f, 520.0f}, 2840.0f, 3160.0f, 3},
        {{3280.0f, 520.0f}, 3160.0f, 3480.0f, 3},
        {{3580.0f, 520.0f}, 3460.0f, 3720.0f, 3},
	};
	world.doors = {
	    {1232.0f, 48.0f, 220.0f, 1, 2},
	    {2512.0f, 48.0f, 220.0f, 2, 3},
	    {3792.0f, 48.0f, 220.0f, 3, 4},
	};
	return world;
}