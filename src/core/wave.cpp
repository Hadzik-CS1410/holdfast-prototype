#include "wave.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "enemies.h"

std::vector<EnemyWave> parseWaves(std::istream& in)
{
    std::vector<EnemyWave> waves;
    std::string line;
    int lineNumber{0};

    while (std::getline(in, line)) {
        lineNumber++;
        std::size_t hash{line.find('#')};
        if (hash != std::string::npos) {
            line = line.substr(0, hash);   // drop the comment
        }
        std::istringstream words{line};
        std::string first;
        if (!(words >> first)) {
            continue;   // blank line
        }
        if (first == "wave") {
            waves.push_back({});
            continue;
        }

        auto fail = [&](const std::string& why) {
            throw std::runtime_error("waves line " + std::to_string(lineNumber) + ": " + why);
        };
        if (waves.empty()) {
            fail("spawn group before the first 'wave'");
        }
        SpawnGroup group{first, 0, 0};
        if (!(words >> group.count >> group.spacing) || group.count <= 0 || group.spacing < 0) {
            fail("expected '<type> <count> <seconds>', got '" + line + "'");
        }
        makeEnemy(group.enemyType);   // throws now, at load time, not mid-game
        waves.back().groups.push_back(group);
    }

    if (waves.empty()) {
        throw std::runtime_error("no waves found");
    }
    return waves;
}

std::vector<EnemyWave> loadWaves(const std::string& path)
{
    std::ifstream file{path};
    if (!file) {
        throw std::runtime_error("can't open waves file: " + path);
    }
    return parseWaves(file);
}

std::unique_ptr<Enemy> makeEnemy(const std::string& type)
{
    if (type == "grunt") return std::make_unique<Grunt>();
    if (type == "brute") return std::make_unique<Brute>();
    if (type == "splitter") return std::make_unique<Splitter>();
    if (type == "splitling") return std::make_unique<Splitling>();
    throw std::invalid_argument("unknown enemy type '" + type + "'");
}
