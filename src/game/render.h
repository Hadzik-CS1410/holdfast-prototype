// =============================================================================
// render.h -- the ONLY code in the project that knows raylib exists
// =============================================================================
#pragma once

#include <optional>

#include "game.h"

// What the player is doing right now -- not part of the game's rules,
// so it lives out here instead of in Game.
struct UiState {
    std::optional<TowerKind> selected;   // tower being placed, if any
    Cell hover{-1, -1};
    bool paused{false};
    int speed{1};
};

constexpr int HUD_HEIGHT{72};
constexpr int SCREEN_WIDTH{Map::COLS * Map::TILE_SIZE};
constexpr int SCREEN_HEIGHT{Map::ROWS * Map::TILE_SIZE + HUD_HEIGHT};

void drawGame(const Game& game, const UiState& ui);
