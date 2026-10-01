#include <sightline/raycast.h>

#include <cmath>
#include <limits>

namespace sightline {

float ray_to_wall(const Map& map, Vec2 origin, Vec2 dir) {
    constexpr float kNever = std::numeric_limits<float>::infinity();

    int tx = static_cast<int>(std::floor(origin.x));
    int ty = static_cast<int>(std::floor(origin.y));
    if (map.is_wall(tx, ty)) {
        return 0;
    }
    if (dir.x == 0 && dir.y == 0) {
        return 0;  // no direction: the loop below would never cross a boundary
    }

    // t_max: distance along the ray to the next vertical (x) / horizontal (y) grid line.
    // t_delta: distance between consecutive lines of that kind; constant for a straight ray.
    const int step_x = dir.x > 0 ? 1 : -1;
    const int step_y = dir.y > 0 ? 1 : -1;
    const float t_delta_x = dir.x != 0 ? std::abs(1 / dir.x) : kNever;
    const float t_delta_y = dir.y != 0 ? std::abs(1 / dir.y) : kNever;
    float t_max_x = dir.x > 0 ? (tx + 1 - origin.x) / dir.x
                  : dir.x < 0 ? (origin.x - tx) / -dir.x
                              : kNever;
    float t_max_y = dir.y > 0 ? (ty + 1 - origin.y) / dir.y
                  : dir.y < 0 ? (origin.y - ty) / -dir.y
                              : kNever;

    while (true) {
        float t;
        if (t_max_x < t_max_y) {
            tx += step_x;
            t = t_max_x;
            t_max_x += t_delta_x;
        } else {
            ty += step_y;
            t = t_max_y;
            t_max_y += t_delta_y;
        }
        if (map.is_wall(tx, ty)) {
            return t;
        }
    }
}

std::optional<float> ray_to_circle(Vec2 origin, Vec2 dir, Vec2 centre, float radius) {
    // Solve |origin + t*dir - centre|^2 = radius^2, i.e. t^2 + 2bt + c = 0 (dir is unit length).
    const Vec2 m = origin - centre;
    const float b = dot(m, dir);
    const float c = dot(m, m) - radius * radius;
    if (c <= 0) {
        return 0.0f;  // origin inside the circle
    }
    if (b > 0) {
        return std::nullopt;  // outside and pointing away
    }
    const float discriminant = b * b - c;
    if (discriminant < 0) {
        return std::nullopt;  // the line passes the circle by
    }
    return -b - std::sqrt(discriminant);  // the nearer of the two roots
}

}
