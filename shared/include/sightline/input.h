#pragma once

#include <sightline/world.h>

#include <array>
#include <cstdint>

namespace sightline {

struct PlayerInput {
    std::uint32_t tick = 0;  // the tick this input is for
    std::int8_t move_x = 0;  // -1, 0, 1
    std::int8_t move_y = 0;  // -1, 0, 1 (down is +)
    float aim = 0;           // radians, same convention as Player::aim
    bool fire = false;
};

// one slot per possible player, indexed by PlayerId.
using Inputs = std::array<PlayerInput, kMaxPlayers>;

}
