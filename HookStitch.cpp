// [HookStitch.cpp]
#define NOMINMAX
#include "HookStitch.h"
#include "Boss.h"
#include "Player.h"
#include <cmath>

using namespace KamataEngine;

void HookStitch::Initialize(uint32_t textureHandle) {
	stitched_.clear();
	for (int i = 0; i < kMaxStitch; ++i) {
		markSprites_.push_back(Sprite::Create(textureHandle, {0.0f, 0.0f}));
		markCrossSprites_.push_back(Sprite::Create(textureHandle, {0.0f, 0.0f}));
	}
	for (int i = 0; i < kDotCount; ++i) {
		dotSprites_.push_back(Sprite::Create(textureHandle, {0.0f, 0.0f}));
	}
}

bool HookStitch::Contains(StitchTarget* target) const {
	for (StitchTarget* stitched : stitched_) {
		if (stitched == target) {
			return true;
		}
	}
	return false;
}

void HookStitch::Clear() {
	stitched_.clear();
	cinchTimer_ = 0;
	pendingResolve_ = false;
}

void HookStitch::TryStitch(StitchTarget* target) {
	if (!target || !target->CanStitch() || target->IsDead()) {
		return;
	}
	if (target->IsStitchCooling()) {
		return;
	}
	if (Contains(target)) {
		return;
	}

	stitched_.push_back(target);
	target->StartStitchCoolDown(kCoolDown);

	if (static_cast<int>(stitched_.size()) > kMaxStitch) {
		stitched_.erase(stitched_.begin());
	}
}

Vector2 HookStitch::CalcCentroid() const {
	Vector2 sum{};
	int count = 0;
	for (StitchTarget* target : stitched_) {
		if (!target) {
			continue;
		}
		AABB2 aabb = target->GetAABB();
		Vector2 c = AABBCenter(aabb);
		sum.x += c.x;
		sum.y += c.y;
		++count;
	}
	if (count == 0) {
		return {};
	}
	return {sum.x / static_cast<float>(count), sum.y / static_cast<float>(count)};
}

bool HookStitch::CheckCinchCollision() const {
	for (size_t i = 0; i < stitched_.size(); ++i) {
		StitchTarget* a = stitched_[i];
		if (!a) {
			continue;
		}
		for (size_t j = i + 1; j < stitched_.size(); ++j) {
			StitchTarget* b = stitched_[j];
			if (!b) {
				continue;
			}
			// 固定同士は無視。動く対象が他の縫い対象に当たったら衝突
			if (a->IsFixed() && b->IsFixed()) {
				continue;
			}
			if (IsCollision(a->GetAABB(), b->GetAABB())) {
				return true;
			}
		}
	}
	return false;
}

void HookStitch::ResolveCinch(const Vector2& centroid) {
	bool hasBossA = false;
	bool hasBossB = false;
	bool hasStake = false;
	int bossIndex = 0;
	for (StitchTarget* target : stitched_) {
		if (!target) {
			continue;
		}
		if (target->GetKind() == StitchTarget::Kind::kStake) {
			hasStake = true;
		} else if (target->GetKind() == StitchTarget::Kind::kBoss) {
			if (bossIndex == 0) {
				hasBossA = true;
			} else {
				hasBossB = true;
			}
			++bossIndex;
		}
	}

	int damage = 1;
	if (hasBossA && hasBossB && hasStake) {
		damage = 3;
	} else if (stitched_.size() >= 3) {
		damage = 2;
	}

	for (StitchTarget* target : stitched_) {
		if (!target) {
			continue;
		}
		if (target->GetKind() != StitchTarget::Kind::kBoss && target->GetKind() != StitchTarget::Kind::kFodder) {
			continue;
		}

		target->OnCinchHit(damage);

		AABB2 aabb = target->GetAABB();
		Vector2 center = AABBCenter(aabb);
		target->ApplyKnockback({(center.x - centroid.x) * 0.08f, -8.0f});
	}

	Clear();
}

void HookStitch::Cinch(Player* player) {
	if (cinchTimer_ > 0) {
		return;
	}
	if (stitched_.size() < 2) {
		return;
	}
	cinchTimer_ = kCinchDuration;
	pendingResolve_ = true;
	if (player) {
		player->SetInvincible(kCinchInvincible);
	}
}

void HookStitch::Update(Player* player, const std::vector<StitchTarget*>& targets) {
	(void)targets;
	++markAnim_;

	for (auto it = stitched_.begin(); it != stitched_.end();) {
		if (!(*it) || (*it)->IsDead()) {
			it = stitched_.erase(it);
		} else {
			++it;
		}
	}

	if (cinchTimer_ > 0) {
		const Vector2 centroid = CalcCentroid();
		for (StitchTarget* target : stitched_) {
			if (!target || target->IsFixed()) {
				continue;
			}
			AABB2 aabb = target->GetAABB();
			Vector2 center = AABBCenter(aabb);
			Vector2 diff{centroid.x - center.x, centroid.y - center.y};
			float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
			if (len > 1.0f) {
				Vector2 next = target->GetPosition();
				next.x += diff.x / len * kCinchSpeed;
				next.y += diff.y / len * kCinchSpeed;
				target->SetPosition(next);
			}
		}

		--cinchTimer_;

		// F押下ではダメージを入れない。ぶつかった瞬間、または時間切れ
		if (pendingResolve_ && (CheckCinchCollision() || cinchTimer_ <= 0)) {
			ResolveCinch(centroid);
		}
		return;
	}

	Input* input = Input::GetInstance();
	if (input->TriggerKey(DIK_C) || input->TriggerKey(DIK_F)) {
		Cinch(player);
	}
}

void HookStitch::DrawThread(const Vector2& from, const Vector2& to, const Vector4& color) {
	Vector2 diff{to.x - from.x, to.y - from.y};
	float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
	if (len < 1.0f || dotSprites_.empty()) {
		return;
	}

	const float spacing = 8.0f;
	int steps = static_cast<int>(len / spacing);
	if (steps < 1) {
		steps = 1;
	}

	const int remain = static_cast<int>(dotSprites_.size()) - dotIndex_;
	if (steps > remain) {
		steps = remain;
	}

	for (int i = 1; i < steps; ++i) {
		const float t = static_cast<float>(i) / static_cast<float>(steps);
		const float x = from.x + diff.x * t;
		const float y = from.y + diff.y * t;

		Sprite* sprite = dotSprites_[dotIndex_++];
		sprite->SetRotation(0.0f);
		sprite->SetColor(color);
		sprite->SetPosition({x - 3.0f, y - 3.0f});
		sprite->SetSize({6.0f, 6.0f});
		sprite->Draw();
	}
}

void HookStitch::DrawMark(const Vector2& center, const Vector4& color) {
	if (markSprites_.empty() || markCrossSprites_.empty()) {
		return;
	}

	static int markIndex = 0;
	if (markIndex >= static_cast<int>(markSprites_.size())) {
		markIndex = 0;
	}

	Sprite* plusH = markSprites_[markIndex];
	Sprite* plusV = markCrossSprites_[markIndex];
	++markIndex;
	if (markIndex >= static_cast<int>(markSprites_.size())) {
		markIndex = 0;
	}

	const float pulse = 1.0f + 0.10f * std::sin(static_cast<float>(markAnim_) * 0.18f);
	const float size = 22.0f * pulse;

	plusH->SetRotation(0.0f);
	plusH->SetColor(color);
	plusH->SetPosition({center.x - size * 0.5f, center.y - 3.0f});
	plusH->SetSize({size, 6.0f});
	plusH->Draw();

	plusV->SetRotation(0.0f);
	plusV->SetColor(color);
	plusV->SetPosition({center.x - 3.0f, center.y - size * 0.5f});
	plusV->SetSize({6.0f, size});
	plusV->Draw();
}

void HookStitch::Draw(Player* player, const Vector2& camera) {
	(void)player;
	dotIndex_ = 0;

	Vector4 color = (cinchTimer_ > 0) ? Vector4{1.0f, 0.25f, 0.2f, 1.0f} : Vector4{1.0f, 0.92f, 0.35f, 1.0f};

	std::vector<Vector2> marks;
	marks.reserve(stitched_.size());
	for (StitchTarget* target : stitched_) {
		if (!target) {
			continue;
		}
		Vector2 c = AABBCenter(target->GetAABB());
		marks.push_back({c.x - camera.x, c.y - camera.y});
	}

	for (size_t i = 1; i < marks.size(); ++i) {
		DrawThread(marks[i - 1], marks[i], color);
	}
	for (const Vector2& mark : marks) {
		DrawMark(mark, color);
	}
}