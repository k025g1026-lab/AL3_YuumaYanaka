// [MapChipField.cpp]
#include "MapChipField.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>

namespace {
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock},
    {"2", MapChipType::kGoal },
    {"3", MapChipType::kEnemy},
};
}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	ResetMapChipData();

	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	std::stringstream mapChipCsv;
	mapChipCsv << file.rdbuf();
	file.close();

	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		std::string line;
		getline(mapChipCsv, line);
		std::istringstream lineStream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			getline(lineStream, word, ',');

			if (mapChipTable.count(word) != 0) {
				mapChipData_.data[i][j] = mapChipTable[word];
			} else {
				mapChipData_.data[i][j] = MapChipType::kBlank;
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (kNumBlockHorizontal <= xIndex) {
		return MapChipType::kBlank;
	}
	if (kNumBlockVirtical <= yIndex) {
		return MapChipType::kBlank;
	}
	return mapChipData_.data[yIndex][xIndex];
}

KamataEngine::Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) {
	return KamataEngine::Vector3(kBlockWidth * xIndex + kBlockWidth / 2.0f, kBlockHeight * (kNumBlockVirtical - 1 - yIndex) + kBlockHeight / 2.0f, 0.0f);
}

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position) {
	IndexSet indexSet = {};
	indexSet.xIndex = static_cast<uint32_t>(std::floor(position.x / kBlockWidth));
	indexSet.yIndex = static_cast<uint32_t>(kNumBlockVirtical - 1 - std::floor(position.y / kBlockHeight));
	return indexSet;
}

MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {
	KamataEngine::Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;
	return rect;
}