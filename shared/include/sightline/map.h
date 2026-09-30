#pragma once

#include <sightline/team.h>

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

namespace sightline {

enum class Tile : std::uint8_t { Floor, Wall };

struct TileCoord {
    int x;
    int y;
};

// Tile (x, y) covers [x, x+1) x [y, y+1) in world units. Row 0 = first line, y points down.
class Map {
public:
    Map(int width, int height, std::vector<Tile> tiles,
        std::vector<TileCoord> spawns_a, std::vector<TileCoord> spawns_b);

    int width() const;
    int height() const;

    // Out of bounds counts as a wall.
    bool is_wall(int x, int y) const;

    const std::vector<TileCoord>& spawns(Team team) const;

private:
    int width_;
    int height_;
    std::vector<Tile> tiles_;  // index = y * width_ + x
    std::vector<TileCoord> spawns_a_;
    std::vector<TileCoord> spawns_b_;
};

// '#' wall, '.' floor, 'A'/'B' team spawn (floor).
// Throws std::runtime_error on empty input, ragged rows, unknown chars, or a missing team spawn.
// Accepts a trailing newline and \r\n line endings.
Map parse_map(std::string_view text);

// Reads the whole file, then parse_map. Throws std::runtime_error (with the path) if it can't open it.
Map load_map(const std::filesystem::path& path);

}
