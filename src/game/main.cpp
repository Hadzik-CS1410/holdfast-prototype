// =============================================================================
// Holdfast (prototype) -- main.cpp: window, input, and the game loop
// =============================================================================
// Controls:  1/2/3 choose a tower   left-click build   right-click sell
//            SPACE next wave   F double speed   P pause   R restart   ESC cancel
//
// Hidden instructor mode:  ./build/holdfast --selftest
//   A scripted player buys towers and plays every wave as fast as possible,
//   saves screenshots, and prints the result. Used to check balance and to
//   prove the game runs headless in the dev container.
// =============================================================================
#include <raylib.h>

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "game.h"
#include "render.h"

namespace {

Game newGame()
{
    std::string dir{ASSET_DIR};
    return Game{Map::fromFile(dir + "/meadow.txt"), loadWaves(dir + "/meadow.waves")};
}

void handleInput(Game& game, UiState& ui)
{
    Vector2 mouse{GetMousePosition()};
    ui.hover = mouse.y < Map::ROWS * Map::TILE_SIZE ? Map::cellAt({mouse.x, mouse.y}) : Cell{-1, -1};

    if (IsKeyPressed(KEY_ONE)) ui.selected = TowerKind::Arrow;
    if (IsKeyPressed(KEY_TWO)) ui.selected = TowerKind::Cannon;
    if (IsKeyPressed(KEY_THREE)) ui.selected = TowerKind::Frost;
    if (IsKeyPressed(KEY_ESCAPE)) ui.selected.reset();
    if (IsKeyPressed(KEY_SPACE)) game.startNextWave();
    if (IsKeyPressed(KEY_P)) ui.paused = !ui.paused;
    if (IsKeyPressed(KEY_F)) ui.speed = ui.speed == 1 ? 2 : 1;
    if (IsKeyPressed(KEY_R)) {
        game = newGame();
        ui = UiState{};
    }

    // Clicking a HUD button selects that tower too.
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse.y >= Map::ROWS * Map::TILE_SIZE) {
        int index{static_cast<int>((mouse.x - 300) / 170)};
        if (mouse.x >= 300 && index >= 0 && index < 3) {
            ui.selected = static_cast<TowerKind>(index);
        }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui.selected && game.map().inBounds(ui.hover)) {
        game.placeTower(*ui.selected, ui.hover);
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (ui.selected) {
            ui.selected.reset();
        } else {
            game.sellTower(ui.hover);
        }
    }
}

// --- self-test: a simple scripted player ------------------------------------

// Rank every buildable cell by how many path waypoints it can reach.
std::vector<Cell> bestSpots(const Map& map, float range)
{
    std::vector<std::pair<int, Cell>> scored;
    for (int row{0}; row < Map::ROWS; row++) {
        for (int col{0}; col < Map::COLS; col++) {
            Cell cell{row, col};
            if (!map.isBuildable(cell)) continue;
            int covered{0};
            for (Vec2 w : map.waypoints()) {
                if (distance(w, Map::centerOf(cell)) <= range) covered++;
            }
            scored.push_back({covered, cell});
        }
    }
    std::stable_sort(scored.begin(), scored.end(), [](auto& a, auto& b) { return a.first > b.first; });
    std::vector<Cell> cells;
    for (auto& [score, cell] : scored) cells.push_back(cell);
    return cells;
}

int selfTest()
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Holdfast (selftest)");
    Game game{newGame()};
    UiState ui{};
    std::vector<Cell> spots{bestSpots(game.map(), 120)};
    const TowerKind buyOrder[]{TowerKind::Arrow, TowerKind::Arrow, TowerKind::Cannon, TowerKind::Frost,
                               TowerKind::Arrow, TowerKind::Cannon};
    std::size_t nextBuy{0};
    std::size_t nextSpot{0};
    const float dt{1.0f / 60};
    int frame{0};
    int shotsTaken{0};

    while (game.state() != GameState::Won && game.state() != GameState::Lost && frame < 60 * 60 * 15) {
        // Buy whenever affordable (mid-wave too, like a player would). If the
        // planned tower is too expensive but an Arrow isn't, buy the Arrow.
        while (nextSpot < spots.size()) {
            TowerKind kind{buyOrder[nextBuy % 6]};
            if (game.gold() < towerCost(kind)) {
                if (game.gold() < towerCost(TowerKind::Arrow) || game.gold() >= 60) break;   // save up a bit
                kind = TowerKind::Arrow;
            }
            if (game.placeTower(kind, spots[nextSpot])) nextBuy++;
            nextSpot++;
        }
        if (game.state() == GameState::Building) {
            if (game.startNextWave()) {
                std::cout << "  wave " << game.waveNumber() << " starts: lives=" << game.lives()
                          << " gold=" << game.gold() << " towers=" << game.towers().size() << '\n';
            }
        }
        game.update(dt);
        frame++;

        // Render every 4th frame so the test runs fast; screenshot mid-wave 4 and mid-wave 6.
        if (frame % 4 == 0) {
            BeginDrawing();
            drawGame(game, ui);
            EndDrawing();
            bool wantShot{(game.waveNumber() == 4 && shotsTaken == 0 && game.enemies().size() >= 6) ||
                          (game.waveNumber() == 6 && shotsTaken == 1 && game.enemies().size() >= 12)};
            if (wantShot) {
                TakeScreenshot(shotsTaken == 0 ? "selftest_wave4.png" : "selftest_wave6.png");
                shotsTaken++;
            }
        }
    }
    BeginDrawing();
    drawGame(game, ui);
    EndDrawing();
    TakeScreenshot("selftest_end.png");

    std::cout << "result=" << (game.state() == GameState::Won ? "WON" : game.state() == GameState::Lost ? "LOST" : "TIMEOUT")
              << " wave=" << game.waveNumber() << "/" << game.waveCount() << " lives=" << game.lives()
              << " gold=" << game.gold() << " kills=" << game.enemiesKilled() << " towers=" << game.towers().size()
              << " game_seconds=" << frame / 60 << '\n';
    CloseWindow();
    return 0;
}

}  // namespace

int main(int argc, char** argv)
{
    try {
        if (argc > 1 && std::string{argv[1]} == "--selftest") {
            return selfTest();
        }

        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Holdfast (prototype)");
        SetExitKey(KEY_NULL);   // ESC cancels placement instead of quitting
        SetTargetFPS(60);

        Game game{newGame()};
        UiState ui{};
        while (!WindowShouldClose()) {
            handleInput(game, ui);
            if (!ui.paused) {
                // Clamp dt: a hiccup (dragging the window) shouldn't teleport enemies.
                float dt{std::min(GetFrameTime(), 0.05f)};
                for (int i{0}; i < ui.speed; i++) {
                    game.update(dt);
                }
            }
            BeginDrawing();
            drawGame(game, ui);
            EndDrawing();
        }
        CloseWindow();
    } catch (const std::exception& e) {
        // Module 12: a broken level file ends up here with a readable message.
        std::cerr << "Holdfast couldn't start: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
