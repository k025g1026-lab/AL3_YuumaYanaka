// [MapChipField.h]
#pragma once
#include "KamataEngine.h"
#include <cstdint>
#include <string>
#include <vector>

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
	kGoal,  // ゴール
	kEnemy, // 敵スポーン
};

class MapChipField {
public:
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	struct Rect {
		float left;
		float right;
		float bottom;
		float top;
	};

	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	uint32_t GetNumBlockVirtical() const { return kNumBlockVirtical; }
	uint32_t GetNumBlockVertical() const { return kNumBlockVirtical; }
	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

private:
	struct MapChipData {
		std::vector<std::vector<MapChipType>> data;
	};

	MapChipData mapChipData_;

	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;
};