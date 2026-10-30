// =============================================================================
// Core tests -- every rule in the game, tested with no window and no graphics.
// =============================================================================
// These double as a preview of the weekly tests: each suite below maps to the
// week that introduces it (see TOWER-DEFENSE.md).
// =============================================================================
#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>

#include "enemies.h"
#include "game.h"
#include "map.h"
#include "towers.h"
#include "vec2.h"
#include "wave.h"

namespace {

const int GRUNT_HP{Grunt{}.maxHealth()};
const int BRUTE_HP{Brute{}.maxHealth()};

// A 12x20 level with a straight path along row 5: S at col 0, G at col 19.
std::string straightLevel()
{
    std::string text;
    for (int row{0}; row < Map::ROWS; row++) {
        text += (row == 5) ? "S##################G" : "....................";
        text += '\n';
    }
    return text;
}

Map mapFrom(const std::string& text)
{
    std::istringstream in{text};
    return Map::fromStream(in);
}

std::vector<EnemyWave> wavesFrom(const std::string& text)
{
    std::istringstream in{text};
    return parseWaves(in);
}

// Put an enemy at a given distance along the straight level's path.
std::unique_ptr<Enemy> enemyAt(std::unique_ptr<Enemy> enemy, float pixelsAlong)
{
    Map map{mapFrom(straightLevel())};
    enemy->update(0, map.waypoints());   // snap to spawn
    float speed{enemy->speed()};
    enemy->update(pixelsAlong / speed, map.waypoints());
    return enemy;
}

}  // namespace

// --- Module 9: Vec2 operators ------------------------------------------------

TEST(Vec2Operators, Arithmetic)
{
    Vec2 a{3, 4};
    Vec2 b{1, 2};
    EXPECT_EQ(a + b, (Vec2{4, 6}));
    EXPECT_EQ(a - b, (Vec2{2, 2}));
    EXPECT_EQ(a * 2.0f, (Vec2{6, 8}));
    EXPECT_EQ(2.0f * a, (Vec2{6, 8}));
    a += b;
    EXPECT_EQ(a, (Vec2{4, 6}));
    EXPECT_NE(a, b);
}

TEST(Vec2Operators, LengthAndDistance)
{
    EXPECT_FLOAT_EQ(length({3, 4}), 5.0f);
    EXPECT_FLOAT_EQ(distance({1, 1}, {4, 5}), 5.0f);
}

// --- Modules 4 + 6 + 12: the map --------------------------------------------

TEST(MapLoading, ShippedLevelLoads)
{
    Map map{Map::fromFile(std::string{ASSET_DIR} + "/meadow.txt")};
    ASSERT_GE(map.waypoints().size(), 10u);
    EXPECT_EQ(map.tileAt(Map::cellAt(map.waypoints().front())), Tile::Spawn);
    EXPECT_EQ(map.tileAt(Map::cellAt(map.waypoints().back())), Tile::Goal);
}

TEST(MapLoading, OutsideTheMapIsRock)
{
    Map map{mapFrom(straightLevel())};
    EXPECT_EQ(map.tileAt({-1, 0}), Tile::Rock);
    EXPECT_EQ(map.tileAt({0, Map::COLS}), Tile::Rock);
    EXPECT_FALSE(map.isBuildable({Map::ROWS, 3}));
    EXPECT_TRUE(map.isBuildable({0, 0}));
    EXPECT_FALSE(map.isBuildable({5, 3}));   // path
}

TEST(MapLoading, BadLevelsThrow)
{
    EXPECT_THROW(mapFrom("....\n"), std::runtime_error);                   // wrong size
    std::string noPath{straightLevel()};
    noPath[5 * 21 + 10] = '.';                                             // break the path
    EXPECT_THROW(mapFrom(noPath), std::runtime_error);
    std::string badTile{straightLevel()};
    badTile[0] = 'X';
    EXPECT_THROW(mapFrom(badTile), std::runtime_error);
}

TEST(MapPath, RecursionBacktracksOutOfDeadEnds)
{
    // A dead-end spur goes up from col 5. The traced path must skip it.
    std::string text{straightLevel()};
    text[4 * 21 + 5] = '#';
    text[3 * 21 + 5] = '#';
    Map map{mapFrom(text)};
    EXPECT_EQ(map.waypoints().size(), 20u);   // exactly the 20 tiles of row 5
    for (Vec2 w : map.waypoints()) {
        EXPECT_EQ(Map::cellAt(w).row, 5);
    }
}

// --- Modules 7 + 10 + 11: enemies -------------------------------------------

TEST(EnemyBasics, HealthNeverGoesNegative)
{
    Grunt grunt;
    grunt.takeHit(1000);
    EXPECT_EQ(grunt.health(), 0);
    EXPECT_FALSE(grunt.isAlive());
}

TEST(EnemyBasics, WalksThePathAndReachesTheGoal)
{
    Map map{mapFrom(straightLevel())};
    Grunt grunt;
    grunt.update(1.0f, map.waypoints());
    EXPECT_NEAR(grunt.progress(), grunt.speed(), 0.01f);
    EXPECT_FALSE(grunt.reachedGoal());
    for (int i{0}; i < 100; i++) {
        grunt.update(1.0f, map.waypoints());
    }
    EXPECT_TRUE(grunt.reachedGoal());
}

TEST(EnemyPolymorphism, BruteArmorWorksThroughABasePointer)
{
    // The whole point of virtual: this code only knows it has an Enemy.
    // Without virtual on takeHit, armor would be skipped here.
    std::unique_ptr<Enemy> enemy{std::make_unique<Brute>()};
    enemy->takeHit(10);
    EXPECT_EQ(enemy->health(), BRUTE_HP - (10 - Brute::ARMOR));
    enemy->takeHit(1);   // armor can't reduce a hit below 1
    EXPECT_EQ(enemy->health(), BRUTE_HP - (10 - Brute::ARMOR) - 1);
}

TEST(EnemyPolymorphism, SplitterSplitsOnDeath)
{
    std::unique_ptr<Enemy> splitter{enemyAt(std::make_unique<Splitter>(), 200)};
    EnemyList children{splitter->onDeath()};
    ASSERT_EQ(children.size(), 2u);
    for (auto& child : children) {
        EXPECT_EQ(child->name(), "Splitling");
        EXPECT_LT(distance(child->position(), splitter->position()), 10.0f);
    }
    EXPECT_TRUE(Grunt{}.onDeath().empty());
}

// --- Module 11: towers --------------------------------------------------------

TEST(TowerTargeting, ArrowHitsTheEnemyFurthestAlong)
{
    EnemyList enemies;
    enemies.push_back(enemyAt(std::make_unique<Grunt>(), 200));
    enemies.push_back(enemyAt(std::make_unique<Grunt>(), 260));   // further along
    ArrowTower tower{{4, 5}};                                     // just above the path
    std::vector<Shot> shots;
    tower.update(0.016f, enemies, shots);
    EXPECT_EQ(enemies[0]->health(), GRUNT_HP);
    EXPECT_EQ(enemies[1]->health(), GRUNT_HP - ArrowTower::DAMAGE);
    EXPECT_EQ(shots.size(), 1u);
}

TEST(TowerTargeting, NothingInRangeMeansNoShot)
{
    EnemyList enemies;
    enemies.push_back(enemyAt(std::make_unique<Grunt>(), 10));
    ArrowTower tower{{0, 19}};   // far corner
    std::vector<Shot> shots;
    tower.update(0.016f, enemies, shots);
    EXPECT_EQ(enemies[0]->health(), GRUNT_HP);
    EXPECT_TRUE(shots.empty());
}

TEST(TowerTargeting, CannonSplashesNearbyEnemies)
{
    EnemyList enemies;
    enemies.push_back(enemyAt(std::make_unique<Grunt>(), 240));
    enemies.push_back(enemyAt(std::make_unique<Grunt>(), 260));
    enemies.push_back(enemyAt(std::make_unique<Grunt>(), 500));   // out of splash
    CannonTower tower{{4, 5}};
    std::vector<Shot> shots;
    tower.update(0.016f, enemies, shots);
    EXPECT_EQ(enemies[0]->health(), GRUNT_HP - CannonTower::DAMAGE);
    EXPECT_EQ(enemies[1]->health(), GRUNT_HP - CannonTower::DAMAGE);
    EXPECT_EQ(enemies[2]->health(), GRUNT_HP);
}

TEST(TowerTargeting, FrostPrefersEnemiesNotYetSlowed)
{
    EnemyList enemies;
    enemies.push_back(enemyAt(std::make_unique<Grunt>(), 200));
    enemies.push_back(enemyAt(std::make_unique<Grunt>(), 260));
    enemies[1]->applySlow(0.5f, 5.0f);   // the leader is already slowed
    FrostTower tower{{4, 5}};
    std::vector<Shot> shots;
    tower.update(0.016f, enemies, shots);
    EXPECT_TRUE(enemies[0]->isSlowed());
    EXPECT_EQ(enemies[0]->health(), GRUNT_HP - FrostTower::DAMAGE);
    EXPECT_EQ(enemies[1]->health(), GRUNT_HP);
}

// --- Modules 3 + 12: wave files -----------------------------------------------

TEST(WaveFiles, ParsesGroupsAndComments)
{
    auto waves{wavesFrom("# hi\nwave\ngrunt 3 0.5\n\nwave\nbrute 2 1.0  # tough\nsplitter 1 0\n")};
    ASSERT_EQ(waves.size(), 2u);
    ASSERT_EQ(waves[1].groups.size(), 2u);
    EXPECT_EQ(waves[1].groups[0].enemyType, "brute");
    EXPECT_EQ(waves[1].groups[0].count, 2);
    EXPECT_FLOAT_EQ(waves[1].groups[0].spacing, 1.0f);
}

TEST(WaveFiles, BadLinesThrowWithLineNumbers)
{
    try {
        wavesFrom("wave\ngrunt 3 0.5\ngrunt lots\n");
        FAIL() << "expected an exception";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string{e.what()}.find("line 3"), std::string::npos) << e.what();
    }
    EXPECT_THROW(wavesFrom("wave\ndragon 1 1\n"), std::invalid_argument);
    EXPECT_THROW(wavesFrom("grunt 1 1\n"), std::runtime_error);
    EXPECT_NO_THROW(loadWaves(std::string{ASSET_DIR} + "/meadow.waves"));
}

// --- Module 8: the Game composes everything -------------------------------------

TEST(GameRules, BuildingCostsGoldAndNeedsGrass)
{
    Game game{mapFrom(straightLevel()), wavesFrom("wave\ngrunt 1 1\n")};
    EXPECT_TRUE(game.placeTower(TowerKind::Arrow, {4, 5}));
    EXPECT_EQ(game.gold(), Game::START_GOLD - ArrowTower::COST);
    EXPECT_FALSE(game.placeTower(TowerKind::Arrow, {4, 5}));   // occupied
    EXPECT_FALSE(game.placeTower(TowerKind::Arrow, {5, 5}));   // path
    EXPECT_TRUE(game.sellTower({4, 5}));
    EXPECT_EQ(game.gold(), Game::START_GOLD - ArrowTower::COST + ArrowTower::COST * 7 / 10);
}

TEST(GameRules, LeakedEnemyCostsALife)
{
    Game game{mapFrom(straightLevel()), wavesFrom("wave\ngrunt 1 1\nwave\ngrunt 1 1\n")};
    ASSERT_TRUE(game.startNextWave());
    for (int i{0}; i < 60 * 30; i++) {
        game.update(1.0f / 60);
    }
    EXPECT_EQ(game.lives(), Game::START_LIVES - 1);
    EXPECT_EQ(game.state(), GameState::Building);
}

TEST(GameRules, KillingEverythingWins)
{
    Game game{mapFrom(straightLevel()), wavesFrom("wave\ngrunt 2 1\n")};
    for (int col : {2, 6, 10, 14}) {
        game.placeTower(TowerKind::Arrow, {4, col});
    }
    game.startNextWave();
    for (int i{0}; i < 60 * 30 && game.state() == GameState::WaveRunning; i++) {
        game.update(1.0f / 60);
    }
    EXPECT_EQ(game.state(), GameState::Won);
    EXPECT_EQ(game.enemiesKilled(), 2);
    EXPECT_EQ(game.lives(), Game::START_LIVES);
}
