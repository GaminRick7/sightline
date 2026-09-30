#pragma once

#include <sightline/input.h>
#include <sightline/map.h>
#include <sightline/world.h>

namespace sightline {

inline constexpr float kMoveSpeed = 6.75f;  // tiles per second

// Tick 0 of a match: players_per_team players on each team, on spawn tile centres,
// facing the enemy team's first spawn. Ids are A's first, then B's.
// Throws std::runtime_error if the count doesn't fit kMaxPlayers or a team lacks spawns.
World make_world(const Map& map, int players_per_team);

// Advances the world by one tick of length dt seconds, using inputs[player.id] for each player.
// The only function that changes game state during a match.
World step(const World& world, const Inputs& inputs, const Map& map, float dt);

}
