#pragma once

#include <sightline/input.h>
#include <sightline/map.h>
#include <sightline/world.h>

namespace sightline {

inline constexpr float kMoveSpeed = 6.75f;  // tiles per second

// Must stay above kMoveSpeed * dt (0.21 tiles at 32 Hz), or a player can pass through a wall
// corner in one tick.
inline constexpr float kPlayerRadius = 0.35f;  // tiles

inline constexpr float kFireInterval = 0.1f;  // seconds between shots while fire is held (10/s)
inline constexpr int kShotDamage = 25;        // 4 hits from full health
inline constexpr float kRoundOverSeconds = 3.0f;

// Tick 0 of a match: players_per_team players on each team, on spawn tile centres,
// facing the enemy team's first spawn. Ids are A's first, then B's.
// Throws std::runtime_error if the count doesn't fit kMaxPlayers or a team lacks spawns.
World make_world(const Map& map, int players_per_team);

// Advances the world by one tick of length dt seconds, using inputs[player.id] for each player.
// The only function that changes game state during a match.
World step(const World& world, const Inputs& inputs, const Map& map, float dt);

}
