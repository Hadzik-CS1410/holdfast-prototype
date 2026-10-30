#include "game.h"

#include <algorithm>

Game::Game(Map map, std::vector<EnemyWave> waves) : map_{std::move(map)}, waves_{std::move(waves)} {}

bool Game::startNextWave()
{
    if (state_ != GameState::Building || waveIndex_ + 1 >= waveCount()) {
        return false;
    }
    waveIndex_++;
    groupIndex_ = 0;
    spawnedInGroup_ = 0;
    spawnTimer_ = 0;
    state_ = GameState::WaveRunning;
    return true;
}

bool Game::placeTower(TowerKind kind, Cell cell)
{
    if (state_ == GameState::Won || state_ == GameState::Lost) {
        return false;
    }
    if (!map_.isBuildable(cell) || towerAt(cell) != nullptr || gold_ < towerCost(kind)) {
        return false;
    }
    gold_ -= towerCost(kind);
    towers_.push_back(makeTower(kind, cell));
    return true;
}

bool Game::sellTower(Cell cell)
{
    for (auto it = towers_.begin(); it != towers_.end(); ++it) {
        if ((*it)->cell() == cell) {
            gold_ += (*it)->sellValue();
            towers_.erase(it);   // unique_ptr deletes the tower -- virtual ~Tower() runs
            return true;
        }
    }
    return false;
}

const Tower* Game::towerAt(Cell cell) const
{
    for (const auto& tower : towers_) {
        if (tower->cell() == cell) {
            return tower.get();
        }
    }
    return nullptr;
}

void Game::update(float dt)
{
    if (state_ == GameState::Won || state_ == GameState::Lost) {
        return;
    }

    if (state_ == GameState::WaveRunning) {
        spawnEnemies(dt);
    }

    // The heart of polymorphism: one loop, every kind of enemy, no switch.
    for (auto& enemy : enemies_) {
        enemy->update(dt, map_.waypoints());
    }
    for (auto& tower : towers_) {
        tower->update(dt, enemies_, shots_);
    }
    removeFinishedEnemies();

    for (Shot& shot : shots_) {
        shot.timeLeft -= dt;
    }
    std::erase_if(shots_, [](const Shot& s) { return s.timeLeft <= 0; });

    if (lives_ <= 0) {
        lives_ = 0;
        state_ = GameState::Lost;
        return;
    }

    bool allSpawned{state_ == GameState::WaveRunning && groupIndex_ >= waves_[waveIndex_].groups.size()};
    if (allSpawned && enemies_.empty()) {
        state_ = (waveIndex_ + 1 >= waveCount()) ? GameState::Won : GameState::Building;
    }
}

void Game::spawnEnemies(float dt)
{
    const EnemyWave& wave{waves_[waveIndex_]};
    spawnTimer_ -= dt;
    while (groupIndex_ < wave.groups.size() && spawnTimer_ <= 0) {
        const SpawnGroup& group{wave.groups[groupIndex_]};
        enemies_.push_back(makeEnemy(group.enemyType));
        spawnedInGroup_++;
        spawnTimer_ += group.spacing;
        if (spawnedInGroup_ >= group.count) {
            groupIndex_++;
            spawnedInGroup_ = 0;
        }
    }
}

void Game::removeFinishedEnemies()
{
    EnemyList stillWalking;
    EnemyList newlySpawned;
    for (auto& enemy : enemies_) {
        if (!enemy->isAlive()) {
            gold_ += enemy->bounty();
            killed_++;
            // Virtual: a Splitter hands back two Splitlings; everyone else, nothing.
            for (auto& child : enemy->onDeath()) {
                newlySpawned.push_back(std::move(child));
            }
        } else if (enemy->reachedGoal()) {
            lives_--;
        } else {
            stillWalking.push_back(std::move(enemy));
        }
    }
    for (auto& child : newlySpawned) {
        stillWalking.push_back(std::move(child));
    }
    enemies_ = std::move(stillWalking);   // the dead are destroyed right here
}
