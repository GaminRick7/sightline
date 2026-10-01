#pragma once

#include <sightline/map.h>
#include <sightline/vec2.h>

#include <optional>

namespace sightline {

// Distance along a unit-length dir from origin to the first wall face, walking the grid one tile
// boundary at a time (DDA). Out of bounds counts as wall, so it always ends. Returns 0 if origin
// is inside a wall.
float ray_to_wall(const Map& map, Vec2 origin, Vec2 dir);

// Distance along a unit-length dir from origin to where the ray enters the circle, or nullopt if
// it misses. 0 if origin is already inside the circle.
std::optional<float> ray_to_circle(Vec2 origin, Vec2 dir, Vec2 centre, float radius);

}
