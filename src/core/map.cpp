#include "map.h"

#include <fstream>
#include <stdexcept>

Map Map::fromStream(std::istream& in)
{
    Map map;
    Cell spawn{-1, -1};
    int row{0};
    std::string line;

    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();   // a level saved on Windows
        }
        if (line.empty()) {
            continue;
        }
        if (row >= ROWS || static_cast<int>(line.size()) != COLS) {
            throw std::runtime_error("level must be " + std::to_string(ROWS) + " rows of " +
                                     std::to_string(COLS) + " characters (problem at row " +
                                     std::to_string(row + 1) + ")");
        }
        for (int col{0}; col < COLS; col++) {
            Tile tile{};
            switch (line[col]) {
                case '.': tile = Tile::Grass; break;
                case '#': tile = Tile::Path; break;
                case '^': tile = Tile::Rock; break;
                case 'S': tile = Tile::Spawn; spawn = {row, col}; break;
                case 'G': tile = Tile::Goal; break;
                default:
                    throw std::runtime_error(std::string{"unknown tile '"} + line[col] + "' at row " +
                                             std::to_string(row + 1));
            }
            map.tiles_[row][col] = tile;
        }
        row++;
    }

    if (row != ROWS) {
        throw std::runtime_error("level has " + std::to_string(row) + " rows, expected " +
                                 std::to_string(ROWS));
    }
    if (spawn.row < 0) {
        throw std::runtime_error("level has no spawn (S)");
    }

    bool visited[ROWS][COLS]{};
    std::vector<Cell> route;
    if (!map.tracePath(spawn, visited, route)) {
        throw std::runtime_error("no path (#) connects the spawn (S) to the goal (G)");
    }
    for (Cell cell : route) {
        map.waypoints_.push_back(centerOf(cell));
    }
    return map;
}

Map Map::fromFile(const std::string& path)
{
    std::ifstream file{path};
    if (!file) {
        throw std::runtime_error("can't open level file: " + path);
    }
    return fromStream(file);
}

// Module 6 -- recursion with backtracking.
//   - base cases: off the map, already visited, not walkable, or the goal
//   - recursive case: try each neighbor; the first one that reaches the goal wins
//   - a dead end pops itself back off the route, so only the real path remains
bool Map::tracePath(Cell cell, bool visited[ROWS][COLS], std::vector<Cell>& route) const
{
    if (!inBounds(cell) || visited[cell.row][cell.col]) {
        return false;
    }
    Tile tile{tiles_[cell.row][cell.col]};
    if (tile != Tile::Path && tile != Tile::Spawn && tile != Tile::Goal) {
        return false;
    }

    visited[cell.row][cell.col] = true;
    route.push_back(cell);
    if (tile == Tile::Goal) {
        return true;
    }

    const Cell neighbors[4]{{cell.row - 1, cell.col}, {cell.row + 1, cell.col},
                            {cell.row, cell.col - 1}, {cell.row, cell.col + 1}};
    for (Cell next : neighbors) {
        if (tracePath(next, visited, route)) {
            return true;
        }
    }

    route.pop_back();   // dead end -- this cell isn't on the way
    return false;
}

bool Map::inBounds(Cell cell) const
{
    return cell.row >= 0 && cell.row < ROWS && cell.col >= 0 && cell.col < COLS;
}

Tile Map::tileAt(Cell cell) const
{
    return inBounds(cell) ? tiles_[cell.row][cell.col] : Tile::Rock;
}

bool Map::isBuildable(Cell cell) const
{
    return tileAt(cell) == Tile::Grass;
}

Vec2 Map::centerOf(Cell cell)
{
    return {cell.col * TILE_SIZE + TILE_SIZE / 2.0f, cell.row * TILE_SIZE + TILE_SIZE / 2.0f};
}

Cell Map::cellAt(Vec2 pixel)
{
    // Truncation toward zero would put pixel -10 in column 0; floor puts it in -1,
    // which tileAt() then correctly reports as outside the map.
    return {static_cast<int>(std::floor(pixel.y / TILE_SIZE)),
            static_cast<int>(std::floor(pixel.x / TILE_SIZE))};
}
