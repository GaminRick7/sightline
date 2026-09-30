#pragma once

#include <sightline/team.h>
#include <sightline/vec2.h>

#include <cstdint>
#include <vector>

namespace sightline {

using PlayerId = std::uint8_t;
inline constexpr int kMaxPlayers = 4;

struct Player {
    PlayerId id = 0;
    Team team = Team::A;
    Vec2 position;       // tiles
    Vec2 velocity;       // tiles per second
    float aim = 0;       // radians, 0 = +x; y points down, so positive = clockwise on screen
    int health = 100;
    bool alive = true;
};

struct World {
    std::uint32_t tick = 0;
    std::vector<Player> players;
};

}
