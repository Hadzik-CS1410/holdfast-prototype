// =============================================================================
// map.h -- the grid the game is played on
// =============================================================================
// Course arc:
//   Module 3  enum class Tile, struct Cell; the map is an array of strings
//   Module 4  Tile tiles[ROWS][COLS] -- row-major indexing, bounds checks
//   Module 6  tracePath() finds the enemy route recursively (and backtracks)
//   Module 7  everything wrapped in class Map: the grid is private, so nobody
//             can index out of bounds -- tileAt() is the only door in
//   Module 12 fromFile() reads a level from a text file; bad files throw
// =============================================================================
#pragma once

#include <istream>
#include <string>
#include <vector>

#include "vec2.h"

enum class Tile { Grass, Path, Rock, Spawn, Goal };

struct Cell {
    int row{};
    int col{};
    bool operator==(const Cell&) const = default;
};

class Map {
public:
    static constexpr int ROWS{12};
    static constexpr int COLS{20};
    static constexpr int TILE_SIZE{48};

    // Level text, one char per tile:  . grass   # path   ^ rock   S spawn   G goal
    // Throws std::runtime_error if the level is the wrong size, has no
    // spawn/goal, or has no path connecting them.
    static Map fromStream(std::istream& in);
    static Map fromFile(const std::string& path);

    // Outside the map counts as solid rock. Callers never need their own bounds
    // check, and an off-by-one can never read memory that isn't the grid.
    Tile tileAt(Cell cell) const;
    bool inBounds(Cell cell) const;
    bool isBuildable(Cell cell) const;

    // Pixel centers of the path tiles, spawn first, goal last.
    const std::vector<Vec2>& waypoints() const { return waypoints_; }

    static Vec2 centerOf(Cell cell);
    static Cell cellAt(Vec2 pixel);

private:
    Map() = default;   // only fromStream can build one -- so every Map is valid

    bool tracePath(Cell cell, bool visited[ROWS][COLS], std::vector<Cell>& route) const;

    Tile tiles_[ROWS][COLS]{};
    std::vector<Vec2> waypoints_;
};
