#include <sightline/map.h>

#include <cstddef>
#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace sightline {

Map::Map(int width, int height, std::vector<Tile> tiles,
         std::vector<TileCoord> spawns_a, std::vector<TileCoord> spawns_b)
    : width_(width),
      height_(height),
      tiles_(std::move(tiles)),
      spawns_a_(std::move(spawns_a)),
      spawns_b_(std::move(spawns_b)) {}

int Map::width() const {
    return width_;
}

int Map::height() const {
    return height_;
}

bool Map::is_wall(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return true;
    }
    return tiles_[y * width_ + x] == Tile::Wall;
}

const std::vector<TileCoord>& Map::spawns(Team team) const {
    return team == Team::A ? spawns_a_ : spawns_b_;
}

Map parse_map(std::string_view text) {
    // Split into lines. A trailing '\r' (CRLF files) is dropped, and a trailing
    // newline ends the loop instead of producing an empty last line.
    std::vector<std::string_view> lines;
    while (!text.empty()) {
        const std::size_t end = text.find('\n');
        std::string_view line = text.substr(0, end);
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }
        lines.push_back(line);
        if (end == std::string_view::npos) {
            break;
        }
        text.remove_prefix(end + 1);
    }

    if (lines.empty() || lines.front().empty()) {
        throw std::runtime_error("map is empty");
    }

    const int width = static_cast<int>(lines.front().size());
    const int height = static_cast<int>(lines.size());

    std::vector<Tile> tiles;
    tiles.reserve(static_cast<std::size_t>(width) * height);
    std::vector<TileCoord> spawns_a;
    std::vector<TileCoord> spawns_b;

    for (int y = 0; y < height; ++y) {
        const std::string_view line = lines[y];
        if (static_cast<int>(line.size()) != width) {
            throw std::runtime_error(std::format(
                "map line {}: expected {} chars, got {}", y + 1, width, line.size()));
        }
        for (int x = 0; x < width; ++x) {
            switch (line[x]) {
                case '#':
                    tiles.push_back(Tile::Wall);
                    break;
                case '.':
                    tiles.push_back(Tile::Floor);
                    break;
                case 'A':
                    tiles.push_back(Tile::Floor);
                    spawns_a.push_back({x, y});
                    break;
                case 'B':
                    tiles.push_back(Tile::Floor);
                    spawns_b.push_back({x, y});
                    break;
                default:
                    throw std::runtime_error(std::format(
                        "map line {}, column {}: unknown char '{}'", y + 1, x + 1, line[x]));
            }
        }
    }

    if (spawns_a.empty()) {
        throw std::runtime_error("map has no A spawn");
    }
    if (spawns_b.empty()) {
        throw std::runtime_error("map has no B spawn");
    }

    return Map(width, height, std::move(tiles), std::move(spawns_a), std::move(spawns_b));
}

Map load_map(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error(std::format("can't open map file {}", path.string()));
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return parse_map(contents.str());
}

}
