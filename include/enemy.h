// =============================================================================
// enemy.h -- the abstract base class every enemy inherits from
// =============================================================================
// Course arc -- this one class carries most of the semester:
//   Module 4  parallel arrays: enemyX[], enemyHp[], enemyType[], enemyCount
//             -- deleting a dead enemy means shifting FOUR arrays in step
//   Module 5  struct Enemy on the heap: Enemy* enemies[MAX], new/delete,
//             nullptr = empty slot. A tower holding Enemy* target after the
//             enemy is deleted is THE dangling-pointer bug of the semester.
//   Module 7  class Enemy: health is private, so takeHit() is the only way to
//             change it and health can never go negative. But armor still
//             needs  if (type_ == BRUTE)  inside takeHit().
//   Module 10 Grunt/Brute/Splitter inherit from Enemy. Brute "overrides"
//             takeHit() -- but without virtual, an Enemy* still calls the
//             BASE version. Armor silently stops working. (Planted on purpose.)
//   Module 11 virtual fixes it. Every  switch (type)  in the game is deleted.
//             Adding an enemy = one new class, zero edits anywhere else.
//   Module 12 std::vector<std::unique_ptr<Enemy>> owns them -- no delete at all
// =============================================================================
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "look.h"
#include "vec2.h"

class Enemy;
using EnemyList = std::vector<std::unique_ptr<Enemy>>;

class Enemy {
public:
    Enemy(std::string name, int maxHealth, float speed, int bounty);

    // Module 11: deleting a Brute through an Enemy* must run ~Brute().
    // Without virtual here, only ~Enemy() runs. (Undefined behavior; often a leak.)
    virtual ~Enemy() = default;

    // --- the same for every enemy: NOT virtual ---------------------------------
    void update(float dt, const std::vector<Vec2>& waypoints);
    void applySlow(float factor, float seconds);
    void placeAt(Vec2 position, std::size_t nextWaypoint, float progress);

    // --- the parts that differ: virtual --------------------------------------
    // Damage after any armor/shields. Base version: take it all.
    virtual void takeHit(int damage);
    // What appears when this enemy dies. Base version: nothing.
    virtual EnemyList onDeath() const;
    // How to draw it. Pure virtual: there is no such thing as a plain "Enemy",
    // so Enemy is abstract and can't be constructed on its own.
    virtual Look look() const = 0;

    // --- const getters: reading never changes the enemy -------------------------
    const std::string& name() const { return name_; }
    int health() const { return health_; }
    int maxHealth() const { return maxHealth_; }
    int bounty() const { return bounty_; }
    float speed() const { return speed_; }
    Vec2 position() const { return position_; }
    float progress() const { return progress_; }   // pixels traveled along the path
    std::size_t nextWaypoint() const { return nextWaypoint_; }
    bool isAlive() const { return health_ > 0; }
    bool isSlowed() const { return slowTimer_ > 0; }
    bool reachedGoal() const { return reachedGoal_; }

private:
    std::string name_;
    int maxHealth_;
    int health_;
    float speed_;
    int bounty_;

    Vec2 position_{};
    std::size_t nextWaypoint_{0};
    float progress_{0};
    bool reachedGoal_{false};
    float slowFactor_{1.0f};
    float slowTimer_{0};
};
