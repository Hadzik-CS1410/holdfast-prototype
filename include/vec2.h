// =============================================================================
// vec2.h -- a 2D vector (position, velocity, direction)
// =============================================================================
// Course arc:
//   Module 3  struct Vec2 { float x{}; float y{}; };  + free functions add(a, b)
//   Module 9  the operators below replace those functions:
//               pos = add(pos, scale(vel, dt));   ->   pos += vel * dt;
//
// Small value types like this are the clearest case for operator overloading:
// the math on paper and the math in code look the same.
// =============================================================================
#pragma once

#include <cmath>
#include <ostream>

struct Vec2 {
    float x{};
    float y{};

    // Member operators: they change *this, so they return a reference to it.
    Vec2& operator+=(Vec2 other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(Vec2 other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2& operator*=(float s)
    {
        x *= s;
        y *= s;
        return *this;
    }

    // C++20: == written once gives you != for free.
    bool operator==(const Vec2&) const = default;
};

// Non-member operators: they make a NEW value, built from the member versions.
inline Vec2 operator+(Vec2 a, Vec2 b) { return a += b; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return a -= b; }
inline Vec2 operator*(Vec2 v, float s) { return v *= s; }
inline Vec2 operator*(float s, Vec2 v) { return v *= s; }   // so 2.0f * v works too

inline float length(Vec2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }
inline float distance(Vec2 a, Vec2 b) { return length(a - b); }

// Prints (x, y) -- handy in the debugger console and in test failure messages.
inline std::ostream& operator<<(std::ostream& out, Vec2 v)
{
    return out << '(' << v.x << ", " << v.y << ')';
}
