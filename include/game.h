// =============================================================================
// game.h -- the whole game's state and rules, with no graphics at all
// =============================================================================
// Game is composition (Module 8): it HAS a Map, HAS enemies, HAS towers,
// HAS waves. It owns them -- when the Game goes away, so do they.
//
// Notice what Game does NOT know: which kinds of enemies or towers exist.
// update() loops over Enemy and Tower and lets each one decide. Add a new
// tower class tomorrow and this file doesn't change.
//
// Because nothing here draws, every rule in the game can be unit tested
// (see tests/test_core.cpp) -- and the tests run in CI with no display.
// =============================================================================
#pragma once

#include <memory>
#include <vector>

#include "enemy.h"
#include "map.h"
#include "tower.h"
#include "towers.h"
#include "wave.h"

enum class GameState { Building, WaveRunning, Won, Lost };

class Game {
public:
    static constexpr int START_GOLD{120};
    static constexpr int START_LIVES{20};

    Game(Map map, std::vector<EnemyWave> waves);

    void update(float dt);
    bool startNextWave();                       // false if one is already running
    bool placeTower(TowerKind kind, Cell cell); // false if can't afford / can't build there
    bool sellTower(Cell cell);

    const Map& map() const { return map_; }
    const EnemyList& enemies() const { return enemies_; }
    const std::vector<std::unique_ptr<Tower>>& towers() const { return towers_; }
    const std::vector<Shot>& shots() const { return shots_; }
    const Tower* towerAt(Cell cell) const;

    int gold() const { return gold_; }
    int lives() const { return lives_; }
    GameState state() const { return state_; }
    int waveNumber() const { return waveIndex_ + 1; }   // 1-based, for display
    int waveCount() const { return static_cast<int>(waves_.size()); }
    int enemiesKilled() const { return killed_; }

private:
    void spawnEnemies(float dt);
    void removeFinishedEnemies();

    Map map_;
    std::vector<EnemyWave> waves_;
    EnemyList enemies_;
    std::vector<std::unique_ptr<Tower>> towers_;
    std::vector<Shot> shots_;

    int gold_{START_GOLD};
    int lives_{START_LIVES};
    int killed_{0};
    GameState state_{GameState::Building};

    int waveIndex_{-1};          // -1 until the first wave starts
    std::size_t groupIndex_{0};
    int spawnedInGroup_{0};
    float spawnTimer_{0};
};
