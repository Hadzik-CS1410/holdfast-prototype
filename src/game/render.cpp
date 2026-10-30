#include "render.h"

#include <raylib.h>

namespace {

constexpr int MAP_HEIGHT{Map::ROWS * Map::TILE_SIZE};

Color toColor(Rgb c, unsigned char alpha = 255)
{
    return {c.r, c.g, c.b, alpha};
}

Vector2 toVector(Vec2 v)
{
    return {v.x, v.y};
}

// Every enemy and tower is drawn by this one function, from the Look it
// reports about itself. The renderer never asks "what kind are you?"
void drawLook(const Look& look, Vec2 at, Color tint)
{
    Vector2 p{toVector(at)};
    switch (look.shape) {
        case Shape::Circle:
            DrawCircleV(p, look.size, tint);
            break;
        case Shape::Square:
            DrawRectangleV({p.x - look.size, p.y - look.size}, {look.size * 2, look.size * 2}, tint);
            break;
        case Shape::Diamond:
            DrawPoly(p, 4, look.size * 1.25f, 0, tint);
            break;
        case Shape::Triangle:
            DrawPoly(p, 3, look.size * 1.25f, -90, tint);
            break;
    }
}

void drawMap(const Map& map)
{
    for (int row{0}; row < Map::ROWS; row++) {
        for (int col{0}; col < Map::COLS; col++) {
            int x{col * Map::TILE_SIZE};
            int y{row * Map::TILE_SIZE};
            bool checker{(row + col) % 2 == 0};
            Color color{};
            switch (map.tileAt({row, col})) {
                case Tile::Grass: color = checker ? Color{118, 170, 92, 255} : Color{110, 162, 86, 255}; break;
                case Tile::Path: color = Color{206, 184, 140, 255}; break;
                case Tile::Rock: color = Color{120, 112, 104, 255}; break;
                case Tile::Spawn: color = Color{150, 90, 160, 255}; break;
                case Tile::Goal: color = Color{70, 120, 190, 255}; break;
            }
            DrawRectangle(x, y, Map::TILE_SIZE, Map::TILE_SIZE, color);
            if (map.tileAt({row, col}) == Tile::Rock) {
                DrawCircle(x + 16, y + 20, 9, Color{96, 90, 84, 255});
                DrawCircle(x + 31, y + 30, 7, Color{96, 90, 84, 255});
            }
        }
    }
}

void drawHealthBar(const Enemy& enemy)
{
    if (enemy.health() == enemy.maxHealth()) {
        return;
    }
    Vec2 p{enemy.position()};
    float width{28};
    float fraction{static_cast<float>(enemy.health()) / enemy.maxHealth()};
    float top{p.y - enemy.look().size - 9};
    DrawRectangleV({p.x - width / 2, top}, {width, 4}, Color{60, 20, 20, 220});
    DrawRectangleV({p.x - width / 2, top}, {width * fraction, 4}, Color{90, 220, 90, 255});
}

void drawRange(Vec2 center, float range, Color color)
{
    DrawCircleV(toVector(center), range, Fade(color, 0.12f));
    DrawCircleLinesV(toVector(center), range, Fade(color, 0.7f));
}

void drawShots(const std::vector<Shot>& shots)
{
    for (const Shot& shot : shots) {
        switch (shot.kind) {
            case ShotKind::Arrow:
                DrawLineEx(toVector(shot.from), toVector(shot.to), 2, Color{250, 230, 150, 255});
                break;
            case ShotKind::Frost:
                DrawLineEx(toVector(shot.from), toVector(shot.to), 3, Color{170, 225, 255, 255});
                break;
            case ShotKind::Blast:
                DrawCircleV(toVector(shot.to), shot.radius * (1.2f - shot.timeLeft * 2),
                            Color{255, 150, 60, 110});
                break;
        }
    }
}

void drawTowerButton(int index, TowerKind kind, const Game& game, const UiState& ui)
{
    int x{300 + index * 170};
    int y{MAP_HEIGHT + 14};
    bool affordable{game.gold() >= towerCost(kind)};
    bool selected{ui.selected && *ui.selected == kind};

    DrawRectangleRounded({float(x), float(y), 160, 44}, 0.25f, 6,
                         selected ? Color{80, 80, 110, 255} : Color{50, 50, 60, 255});
    if (selected) {
        DrawRectangleRoundedLines({float(x), float(y), 160, 44}, 0.25f, 6, Color{240, 220, 120, 255});
    }
    auto preview{makeTower(kind, {0, 0})};
    Look look{preview->look()};
    look.size = 11;
    drawLook(look, {float(x + 24), float(y + 22)}, toColor(look.color, affordable ? 255 : 90));
    Color text{affordable ? RAYWHITE : GRAY};
    DrawText(TextFormat("[%d] %s", index + 1, towerName(kind)), x + 44, y + 6, 18, text);
    DrawText(TextFormat("%d gold", towerCost(kind)), x + 44, y + 25, 14, affordable ? GOLD : GRAY);
}

void drawHud(const Game& game, const UiState& ui)
{
    DrawRectangle(0, MAP_HEIGHT, SCREEN_WIDTH, HUD_HEIGHT, Color{32, 32, 40, 255});
    DrawText(TextFormat("Gold  %d", game.gold()), 16, MAP_HEIGHT + 10, 22, GOLD);
    DrawText(TextFormat("Lives %d", game.lives()), 16, MAP_HEIGHT + 38, 22, Color{240, 110, 110, 255});
    DrawText(TextFormat("Wave %d/%d", game.waveNumber(), game.waveCount()), 160, MAP_HEIGHT + 10, 22, RAYWHITE);
    DrawText(TextFormat("Kills %d", game.enemiesKilled()), 160, MAP_HEIGHT + 38, 18, LIGHTGRAY);

    const TowerKind kinds[]{TowerKind::Arrow, TowerKind::Cannon, TowerKind::Frost};
    for (int i{0}; i < 3; i++) {
        drawTowerButton(i, kinds[i], game, ui);
    }
    if (ui.speed > 1) {
        DrawText("x2", SCREEN_WIDTH - 40, MAP_HEIGHT + 26, 22, YELLOW);
    }
}

void drawBanner(const char* title, const char* subtitle, Color color)
{
    int titleWidth{MeasureText(title, 44)};
    int subWidth{MeasureText(subtitle, 20)};
    DrawRectangle(0, MAP_HEIGHT / 2 - 56, SCREEN_WIDTH, 112, Fade(BLACK, 0.6f));
    DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, MAP_HEIGHT / 2 - 40, 44, color);
    DrawText(subtitle, (SCREEN_WIDTH - subWidth) / 2, MAP_HEIGHT / 2 + 14, 20, RAYWHITE);
}

}  // namespace

void drawGame(const Game& game, const UiState& ui)
{
    ClearBackground(BLACK);
    drawMap(game.map());

    // Placement preview / hovered tower range.
    const Map& map{game.map()};
    if (map.inBounds(ui.hover)) {
        if (ui.selected) {
            bool ok{map.isBuildable(ui.hover) && game.towerAt(ui.hover) == nullptr &&
                    game.gold() >= towerCost(*ui.selected)};
            auto preview{makeTower(*ui.selected, ui.hover)};
            drawRange(preview->position(), preview->range(), ok ? WHITE : RED);
        } else if (const Tower* tower{game.towerAt(ui.hover)}) {
            drawRange(tower->position(), tower->range(), WHITE);
        }
    }

    for (const auto& tower : game.towers()) {
        Vec2 p{tower->position()};
        DrawRectangleRounded({p.x - 19, p.y - 19, 38, 38}, 0.3f, 6, Color{60, 56, 52, 255});
        Look look{tower->look()};
        drawLook(look, p, toColor(look.color));
    }

    for (const auto& enemy : game.enemies()) {
        Look look{enemy->look()};
        if (enemy->isSlowed()) {
            DrawCircleV(toVector(enemy->position()), look.size + 4, Color{170, 225, 255, 120});
        }
        drawLook(look, enemy->position(), toColor(look.color));
        drawHealthBar(*enemy);
    }

    drawShots(game.shots());
    drawHud(game, ui);

    // Status line just above the HUD.
    const char* hint{nullptr};
    if (const Tower* tower{map.inBounds(ui.hover) ? game.towerAt(ui.hover) : nullptr}; tower && !ui.selected) {
        hint = TextFormat("%s tower  -  right-click to sell for %d", tower->name().c_str(), tower->sellValue());
    } else if (game.state() == GameState::Building) {
        hint = TextFormat("Press SPACE to send wave %d     1/2/3 pick a tower, click to build     F speed  P pause",
                          game.waveNumber() + 1);
    }
    if (hint) {
        DrawText(hint, 10, MAP_HEIGHT - 24, 18, Fade(BLACK, 0.75f));
    }

    if (game.state() == GameState::Won) {
        drawBanner("THE GATE HOLDS", TextFormat("All %d waves stopped. R to play again.", game.waveCount()), GOLD);
    } else if (game.state() == GameState::Lost) {
        drawBanner("THE GATE FALLS", TextFormat("You reached wave %d. R to try again.", game.waveNumber()),
                   Color{240, 110, 110, 255});
    } else if (ui.paused) {
        drawBanner("PAUSED", "P to resume", RAYWHITE);
    }
}
