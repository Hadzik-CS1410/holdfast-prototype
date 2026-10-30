#include "tower.h"

Tower::Tower(std::string name, Cell cell, float range, float reloadSeconds, int cost)
    : name_{std::move(name)}, cell_{cell}, range_{range}, reloadSeconds_{reloadSeconds}, cost_{cost}
{
}

void Tower::update(float dt, EnemyList& enemies, std::vector<Shot>& shots)
{
    if (reloadTimer_ > 0) {
        reloadTimer_ -= dt;
        return;
    }
    // Virtual calls: which version runs depends on what this tower really is,
    // even though this code only knows it has "a Tower".
    Enemy* target{chooseTarget(enemies)};
    if (target == nullptr) {
        return;   // nothing in range -- stay loaded and keep watching
    }
    fire(*target, enemies, shots);
    reloadTimer_ = reloadSeconds_;
}

bool Tower::inRange(const Enemy& enemy) const
{
    return enemy.isAlive() && distance(position(), enemy.position()) <= range_;
}

// Default targeting: the enemy that has traveled furthest -- the biggest threat.
// The pointer is only used during this frame; nobody stores it. (Storing it is
// exactly how the Module 5 dangling-pointer bug happens.)
Enemy* Tower::chooseTarget(EnemyList& enemies) const
{
    Enemy* best{nullptr};
    for (auto& enemy : enemies) {
        if (inRange(*enemy) && (best == nullptr || enemy->progress() > best->progress())) {
            best = enemy.get();
        }
    }
    return best;
}
