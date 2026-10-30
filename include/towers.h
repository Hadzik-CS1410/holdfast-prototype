// =============================================================================
// towers.h -- every kind of tower
// =============================================================================
//   ArrowTower   cheap, fast, single target           overrides fire()
//   CannonTower  slow, splash damage around the target overrides fire()
//   FrostTower   barely hurts, slows enemies down     overrides fire() AND
//                                                     chooseTarget() (skips
//                                                     enemies already slowed)
//
// Tower costs live on the classes as static constants so the shop can show a
// price before any tower exists.
// =============================================================================
#pragma once

#include <memory>

#include "tower.h"

enum class TowerKind { Arrow, Cannon, Frost };

class ArrowTower : public Tower {
public:
    static constexpr int COST{50};
    static constexpr int DAMAGE{10};
    explicit ArrowTower(Cell cell) : Tower{"Arrow", cell, 140.0f, 0.6f, COST} {}
    Look look() const override;

protected:
    void fire(Enemy& target, EnemyList& enemies, std::vector<Shot>& shots) override;
};

class CannonTower : public Tower {
public:
    static constexpr int COST{90};
    static constexpr int DAMAGE{26};
    static constexpr float SPLASH_RADIUS{55.0f};
    explicit CannonTower(Cell cell) : Tower{"Cannon", cell, 120.0f, 1.6f, COST} {}
    Look look() const override;

protected:
    void fire(Enemy& target, EnemyList& enemies, std::vector<Shot>& shots) override;
};

class FrostTower : public Tower {
public:
    static constexpr int COST{70};
    static constexpr int DAMAGE{3};
    static constexpr float SLOW_FACTOR{0.45f};
    static constexpr float SLOW_SECONDS{2.0f};
    explicit FrostTower(Cell cell) : Tower{"Frost", cell, 110.0f, 0.9f, COST} {}
    Look look() const override;

protected:
    Enemy* chooseTarget(EnemyList& enemies) const override;
    void fire(Enemy& target, EnemyList& enemies, std::vector<Shot>& shots) override;
};

// The factory: these three functions are the ONLY code in the game that
// switches on a TowerKind. Everything else holds a Tower and asks it what to do.
// (Compare Module 5's version, where every update/draw/fire had its own switch.)
std::unique_ptr<Tower> makeTower(TowerKind kind, Cell cell);
int towerCost(TowerKind kind);
const char* towerName(TowerKind kind);
