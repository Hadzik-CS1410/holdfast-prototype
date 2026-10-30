#include "towers.h"

// --- Arrow ------------------------------------------------------------------

Look ArrowTower::look() const
{
    return {Shape::Triangle, {200, 150, 60}, 16};
}

void ArrowTower::fire(Enemy& target, EnemyList&, std::vector<Shot>& shots)
{
    target.takeHit(DAMAGE);   // virtual: a Brute's armor applies automatically
    shots.push_back({position(), target.position(), ShotKind::Arrow, 0, 0.12f});
}

// --- Cannon -----------------------------------------------------------------

Look CannonTower::look() const
{
    return {Shape::Circle, {150, 155, 175}, 15};
}

void CannonTower::fire(Enemy& target, EnemyList& enemies, std::vector<Shot>& shots)
{
    Vec2 impact{target.position()};
    for (auto& enemy : enemies) {
        if (enemy->isAlive() && distance(enemy->position(), impact) <= SPLASH_RADIUS) {
            enemy->takeHit(DAMAGE);
        }
    }
    shots.push_back({position(), impact, ShotKind::Blast, SPLASH_RADIUS, 0.25f});
}

// --- Frost ------------------------------------------------------------------

Look FrostTower::look() const
{
    return {Shape::Diamond, {110, 190, 240}, 16};
}

// Overriding chooseTarget: prefer the leading enemy that ISN'T slowed yet;
// if every enemy in range is already slowed, fall back to the base rule.
Enemy* FrostTower::chooseTarget(EnemyList& enemies) const
{
    Enemy* best{nullptr};
    for (auto& enemy : enemies) {
        if (inRange(*enemy) && !enemy->isSlowed() &&
            (best == nullptr || enemy->progress() > best->progress())) {
            best = enemy.get();
        }
    }
    return best != nullptr ? best : Tower::chooseTarget(enemies);
}

void FrostTower::fire(Enemy& target, EnemyList&, std::vector<Shot>& shots)
{
    target.takeHit(DAMAGE);
    target.applySlow(SLOW_FACTOR, SLOW_SECONDS);
    shots.push_back({position(), target.position(), ShotKind::Frost, 0, 0.2f});
}

// --- Factory ----------------------------------------------------------------

std::unique_ptr<Tower> makeTower(TowerKind kind, Cell cell)
{
    switch (kind) {
        case TowerKind::Arrow: return std::make_unique<ArrowTower>(cell);
        case TowerKind::Cannon: return std::make_unique<CannonTower>(cell);
        case TowerKind::Frost: return std::make_unique<FrostTower>(cell);
    }
    return nullptr;
}

int towerCost(TowerKind kind)
{
    switch (kind) {
        case TowerKind::Arrow: return ArrowTower::COST;
        case TowerKind::Cannon: return CannonTower::COST;
        case TowerKind::Frost: return FrostTower::COST;
    }
    return 0;
}

const char* towerName(TowerKind kind)
{
    switch (kind) {
        case TowerKind::Arrow: return "Arrow";
        case TowerKind::Cannon: return "Cannon";
        case TowerKind::Frost: return "Frost";
    }
    return "?";
}
