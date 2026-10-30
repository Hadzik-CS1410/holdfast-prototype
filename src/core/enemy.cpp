#include "enemy.h"

#include <algorithm>

// Module 7: the member initializer list. Members are set up in the order they
// are DECLARED in the class, not the order written here -- keep them matched.
Enemy::Enemy(std::string name, int maxHealth, float speed, int bounty)
    : name_{std::move(name)}, maxHealth_{maxHealth}, health_{maxHealth}, speed_{speed}, bounty_{bounty}
{
}

void Enemy::update(float dt, const std::vector<Vec2>& waypoints)
{
    if (reachedGoal_ || waypoints.empty()) {
        return;
    }
    if (nextWaypoint_ == 0 && progress_ == 0) {
        position_ = waypoints.front();   // first frame: appear at the spawn
    }

    if (slowTimer_ > 0) {
        slowTimer_ -= dt;
        if (slowTimer_ <= 0) {
            slowFactor_ = 1.0f;
        }
    }

    // Walk toward the next waypoint. A fast enemy (or a big dt) can pass
    // several waypoints in one frame, so keep going until the step is used up.
    float step{speed_ * slowFactor_ * dt};
    while (step > 0 && nextWaypoint_ < waypoints.size()) {
        Vec2 toTarget{waypoints[nextWaypoint_] - position_};
        float remaining{length(toTarget)};
        if (remaining <= step) {
            position_ = waypoints[nextWaypoint_];
            progress_ += remaining;
            step -= remaining;
            nextWaypoint_++;
        } else {
            position_ += toTarget * (step / remaining);
            progress_ += step;
            step = 0;
        }
    }
    if (nextWaypoint_ >= waypoints.size()) {
        reachedGoal_ = true;
    }
}

void Enemy::applySlow(float factor, float seconds)
{
    slowFactor_ = std::min(slowFactor_, factor);
    slowTimer_ = std::max(slowTimer_, seconds);
}

void Enemy::placeAt(Vec2 position, std::size_t nextWaypoint, float progress)
{
    position_ = position;
    nextWaypoint_ = nextWaypoint;
    progress_ = progress;
}

// The class guarantees health stays in [0, maxHealth]. No caller can break that,
// because no caller can touch health_ directly. That guarantee is encapsulation.
void Enemy::takeHit(int damage)
{
    health_ = std::max(0, health_ - std::max(0, damage));
}

EnemyList Enemy::onDeath() const
{
    return {};
}
