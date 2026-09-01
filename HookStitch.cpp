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
		Sprite* sprite = Sprite::Create(textureHandle, {0.0f, 0.0f});
		threadSprites_.push_back(sprite);
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

void HookStitch::Cinch(const std::vector<StitchTarget*>& /*allTargets*/) {
	if (cinchTimer_ > 0) {
		return;
	}
	if (stitched_.size() < 2) {
		return;
	}
	cinchTimer_ = kCinchDuration;
	pendingResolve_ = true;
}

void HookStitch::Update(Player* player, const std::vector<StitchTarget*>& targets) {
	(void)player;

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
		if (cinchTimer_ <= 0 && pendingResolve_) {
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
				if (!target || target->GetKind() != StitchTarget::Kind::kBoss) {
					continue;
				}
				target->OnCinchHit(damage);

				AABB2 aabb = target->GetAABB();
				Vector2 center = AABBCenter(aabb);
				Boss* boss = dynamic_cast<Boss*>(target);
				if (boss) {
					Vector2 kb{(center.x - centroid.x) * 0.08f, -8.0f};
					boss->ApplyKnockback(kb);
				}
			}

			Clear();
		}
		return;
	}

	Input* input = Input::GetInstance();
	if (input->TriggerKey(DIK_C) || input->TriggerKey(DIK_F)) {
		Cinch(targets);
	}
}

void HookStitch::DrawThread(const Vector2& from, const Vector2& to, const Vector4& color) {
	if (threadSprites_.empty()) {
		return;
	}

	static int spriteIndex = 0;
	if (spriteIndex >= static_cast<int>(threadSprites_.size())) {
		spriteIndex = 0;
	}
	Sprite* sprite = threadSprites_[spriteIndex++];
	if (spriteIndex >= static_cast<int>(threadSprites_.size())) {
		spriteIndex = 0;
	}

	Vector2 diff{to.x - from.x, to.y - from.y};
	float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
	if (len < 1.0f) {
		return;
	}
	float angle = std::atan2(diff.y, diff.x);

	sprite->SetColor(color);
	sprite->SetPosition({(from.x + to.x) * 0.5f, (from.y + to.y) * 0.5f});
	sprite->SetSize({len, 6.0f});
	sprite->SetRotation(angle);
	sprite->Draw();
}

void HookStitch::Draw(Player* player) {
	if (!player) {
		return;
	}

	Vector2 prev = player->GetCenter();
	Vector4 color = (cinchTimer_ > 0) ? Vector4{1.0f, 0.2f, 0.2f, 1.0f} : Vector4{1.0f, 1.0f, 1.0f, 1.0f};

	for (StitchTarget* target : stitched_) {
		if (!target) {
			continue;
		}
		Vector2 next = AABBCenter(target->GetAABB());
		DrawThread(prev, next, color);
		prev = next;
	}
}