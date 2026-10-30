// =============================================================================
// look.h -- how something should be drawn, WITHOUT knowing how to draw
// =============================================================================
// The game's core (map, enemies, towers, waves) never includes raylib.
// Each object describes itself with a Look; only src/game/render.cpp turns a
// Look into pixels.
//
// Why bother?
//   - the core compiles and tests with no window, no graphics, no display
//   - swapping raylib for another library touches ONE file
//   - this is the "separate what changes from what doesn't" idea the whole
//     course is about, applied to the program's architecture
// =============================================================================
#pragma once

enum class Shape { Circle, Square, Diamond, Triangle };

struct Rgb {
    unsigned char r{};
    unsigned char g{};
    unsigned char b{};
};

struct Look {
    Shape shape{Shape::Circle};
    Rgb color{};
    float size{10.0f};   // radius in pixels
};
