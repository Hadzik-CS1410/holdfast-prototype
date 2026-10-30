// =============================================================================
// wave.h -- what attacks, how many, and how fast
// =============================================================================
// Course arc:
//   Module 3  parse one line:  "grunt 8 0.9"  with find/substr/stoi/stof
//   Module 8  class EnemyWave owns a heap array of SpawnGroups -> Rule of Three
//   Module 12 std::vector replaces the heap array (Rule of Zero), waves come
//             from a file, and a bad line throws with its line number
//
// Named EnemyWave, not Wave, because raylib already defines a struct Wave (an
// audio clip). Two libraries, one name: a real collision, and the reason big
// projects put their code in a namespace.
//
// File format (levels/*.waves):
//     # comment
//     wave
//     grunt 8 0.9        <- type, count, seconds between spawns
//     brute 2 2.0
//     wave
//     ...
// =============================================================================
#pragma once

#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "enemy.h"

struct SpawnGroup {
    std::string enemyType;
    int count{};
    float spacing{};
};

struct EnemyWave {
    std::vector<SpawnGroup> groups;
};

std::vector<EnemyWave> parseWaves(std::istream& in);       // throws std::runtime_error
std::vector<EnemyWave> loadWaves(const std::string& path);

// The enemy factory: a type name from the file becomes an object.
// Throws std::invalid_argument for a name it doesn't know.
std::unique_ptr<Enemy> makeEnemy(const std::string& type);
