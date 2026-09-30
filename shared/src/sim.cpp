#include <sightline/sim.h>

#include <cmath>
#include <format>
#include <stdexcept>

namespace sightline {

namespace {

Vec2 tile_centre(TileCoord tile) {
    return {tile.x + 0.5f, tile.y + 0.5f};
}

Team other(Team team) {
    return team == Team::A ? Team::B : Team::A;
}

}

World make_world(const Map& map, int players_per_team) {
    if (players_per_team < 1 || 2 * players_per_team > kMaxPlayers) {
        throw std::runtime_error(std::format(
            "players_per_team must be 1 to {}, got {}", kMaxPlayers / 2, players_per_team));
    }

    World world;
    world.players.reserve(2 * players_per_team);
    PlayerId next_id = 0;

    for (Team team : {Team::A, Team::B}) {
        const std::vector<TileCoord>& spawns = map.spawns(team);
        if (static_cast<int>(spawns.size()) < players_per_team) {
            throw std::runtime_error(std::format(
                "team {} has {} spawns, needs {}", team == Team::A ? 'A' : 'B', spawns.size(),
                players_per_team));
        }
        const Vec2 enemy_spawn = tile_centre(map.spawns(other(team)).front());

        for (int i = 0; i < players_per_team; ++i) {
            Player player;
            player.id = next_id++;
            player.team = team;
            player.position = tile_centre(spawns[i]);
            const Vec2 to_enemy = enemy_spawn - player.position;
            player.aim = std::atan2(to_enemy.y, to_enemy.x);
            world.players.push_back(player);
        }
    }

    return world;
}

World step(const World& world, const Inputs& inputs, [[maybe_unused]] const Map& map, float dt) {
    World next = world;
    next.tick = world.tick + 1;

    for (Player& player : next.players) {
        if (!player.alive) {
            continue;
        }
        const PlayerInput& input = inputs[player.id];

        const Vec2 direction = normalized({static_cast<float>(input.move_x),
                                           static_cast<float>(input.move_y)});
        player.velocity = direction * kMoveSpeed;
        player.position = player.position + player.velocity * dt;
        player.aim = input.aim;
    }

    return next;
}

}
