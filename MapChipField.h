// [MapChipField.h]
#pragma once
#include <kamataengine.h>
#include <map>
#include <string>
#include <vector>

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {
public:
	// 定数
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	static inline const uint32_t kNumBlockVertical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	// 初期化
	void Initialize();

	// リセット
	void ResetMapChipData();

	// CSV読み込み
	void LoadMapChipCsv(const std::string& filePath);

	// マップチップタイプ取得
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) const;

	// マップチップ座標取得
	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const;

	// 要素数取得
	uint32_t GetNumBlockVertical() const { return kNumBlockVertical; }
	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

private:
	MapChipData mapChipData_;
};

// マップチップテーブル（匿名名前空間）
namespace {
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock},
};
}