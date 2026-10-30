// =============================================================================
// enemies.h -- every kind of enemy
// =============================================================================
// Read these four classes side by side. Each one is SHORT because it only
// says how it differs from Enemy. That's the payoff of inheritance:
//   - Grunt     differs in nothing but stats and looks
//   - Brute     overrides takeHit()  (armor)
//   - Splitter  overrides onDeath()  (splits into two Splitlings)
//
// Want a new enemy? Add a class here and one line in makeEnemy() (wave.cpp).
// Nothing in Game, Tower, or the renderer changes. That's polymorphism.
// =============================================================================
#pragma once

#include "enemy.h"

class Grunt : public Enemy {
public:
    Grunt() : Enemy{"Grunt", 45, 75.0f, 4} {}   // Module 10: constructor chaining
    Look look() const override;
};

class Brute : public Enemy {
public:
    static constexpr int ARMOR{5};
    Brute() : Enemy{"Brute", 165, 45.0f, 10} {}
    void takeHit(int damage) override;   // override: compiler error if the base isn't virtual
    Look look() const override;
};

class Splitling : public Enemy {
public:
    Splitling() : Enemy{"Splitling", 18, 100.0f, 1} {}
    Look look() const override;
};

class Splitter : public Enemy {
public:
    Splitter() : Enemy{"Splitter", 80, 60.0f, 5} {}
    EnemyList onDeath() const override;
    Look look() const override;
};
