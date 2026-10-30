// =============================================================================
// tower.h -- the abstract base class for every tower
// =============================================================================
// update() is written ONCE, here, and never overridden. It handles the parts
// every tower shares -- reloading, range checks -- and calls two virtual
// functions for the parts that differ:
//
//     chooseTarget()   WHICH enemy?   (default: the one closest to the goal)
//     fire()           WHAT happens?  (pure virtual -- every tower must say)
//
// That shape -- a fixed algorithm in the base class with virtual "holes" the
// derived classes fill in -- is called the Template Method pattern. It's how
// most real frameworks (including game engines) use inheritance.
// =============================================================================
#pragma once

#include <string>
#include <vector>

#include "enemy.h"
#include "look.h"
#include "map.h"

enum class ShotKind { Arrow, Blast, Frost };

// A short-lived visual: the renderer draws it, update() ages it.
struct Shot {
    Vec2 from{};
    Vec2 to{};
    ShotKind kind{ShotKind::Arrow};
    float radius{0};
    float timeLeft{0.15f};
};

class Tower {
public:
    Tower(std::string name, Cell cell, float range, float reloadSeconds, int cost);
    virtual ~Tower() = default;

    void update(float dt, EnemyList& enemies, std::vector<Shot>& shots);

    virtual Look look() const = 0;

    const std::string& name() const { return name_; }
    Cell cell() const { return cell_; }
    Vec2 position() const { return Map::centerOf(cell_); }
    float range() const { return range_; }
    int cost() const { return cost_; }
    int sellValue() const { return cost_ * 7 / 10; }
    bool inRange(const Enemy& enemy) const;

protected:
    // protected: derived classes override/call these; outside code can't.
    virtual Enemy* chooseTarget(EnemyList& enemies) const;
    virtual void fire(Enemy& target, EnemyList& enemies, std::vector<Shot>& shots) = 0;

private:
    std::string name_;
    Cell cell_;
    float range_;
    float reloadSeconds_;
    int cost_;
    float reloadTimer_{0};
};
