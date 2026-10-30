#include "enemies.h"

#include <algorithm>

Look Grunt::look() const
{
    return {Shape::Circle, {214, 160, 90}, 11};
}

// The override adds armor, then lets the base class do the real work -- so
// the "health never goes negative" rule still lives in exactly one place.
void Brute::takeHit(int damage)
{
    Enemy::takeHit(std::max(1, damage - ARMOR));
}

Look Brute::look() const
{
    return {Shape::Square, {90, 90, 105}, 15};
}

Look Splitling::look() const
{
    return {Shape::Diamond, {140, 220, 120}, 7};
}

EnemyList Splitter::onDeath() const
{
    EnemyList children;
    for (int i{0}; i < 2; i++) {
        auto child = std::make_unique<Splitling>();
        // Stagger them slightly so they don't overlap perfectly.
        child->placeAt(position() + Vec2{i * 8.0f - 4.0f, 0}, nextWaypoint(), progress() - i * 10.0f);
        children.push_back(std::move(child));
    }
    return children;
}

Look Splitter::look() const
{
    return {Shape::Diamond, {70, 170, 80}, 12};
}
