#pragma once

#include <sightline/team.h>
#include <sightline/vec2.h>

#include <array>
#include <cstdint>
#include <vector>

namespace sightline {

using PlayerId = std::uint8_t;
inline constexpr int kMaxPlayers = 4;
inline constexpr int kMaxHealth = 100;

struct Player {
    PlayerId id = 0;
    Team team = Team::A;
    Vec2 position;       // tiles
    Vec2 velocity;       // tiles per second
    float aim = 0;       // radians, 0 = +x; y points down, so positive = clockwise on screen
    int health = kMaxHealth;
    bool alive = true;
    float fire_cooldown = 0;  // seconds until this player can fire again
};

// one shot fired this tick, kept so the client can draw a tracer.
struct Shot {
    PlayerId shooter = 0;
    Vec2 from;
    Vec2 to;           // where the ray stopped: a wall face or the target's edge
    bool hit = false;  // true if it stopped at a player
};

enum class Phase : std::uint8_t {
    Playing,    // shooting allowed; ends when a team is wiped
    RoundOver,  // short pause: players can move but not shoot, then everyone respawns
};

struct World {
    std::uint32_t tick = 0;
    std::vector<Player> players;
    std::vector<Shot> shots;  // only this tick's; cleared at the start of every step()

    Phase phase = Phase::Playing;
    float round_timer = 0;       // seconds left in RoundOver
    std::array<int, 2> score{};  // rounds won, indexed by Team
    int round = 1;
};

}
