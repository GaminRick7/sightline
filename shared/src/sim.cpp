#include <sightline/sim.h>

#include <sightline/raycast.h>

#include <algorithm>
#include <array>
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


Vec2 push_out_of_walls(Vec2 p, const Map& map) {
    for (int pass = 0; pass < 4; ++pass) {  // an inside corner needs 2
        const int x0 = static_cast<int>(std::floor(p.x - kPlayerRadius));
        const int x1 = static_cast<int>(std::floor(p.x + kPlayerRadius));
        const int y0 = static_cast<int>(std::floor(p.y - kPlayerRadius));
        const int y1 = static_cast<int>(std::floor(p.y + kPlayerRadius));

        float deepest = 0;
        Vec2 push;
        for (int ty = y0; ty <= y1; ++ty) {
            for (int tx = x0; tx <= x1; ++tx) {
                if (!map.is_wall(tx, ty)) {
                    continue;
                }
                const Vec2 closest{std::clamp(p.x, static_cast<float>(tx), static_cast<float>(tx + 1)),
                                   std::clamp(p.y, static_cast<float>(ty), static_cast<float>(ty + 1))};
                const Vec2 away = p - closest;
                const float depth = kPlayerRadius - length(away);
                if (depth > deepest) {
                    deepest = depth;
                    push = normalized(away) * depth;
                }
            }
        }

        if (deepest == 0) {
            break;
        }
        p = p + push;
    }
    return p;
}

// puts every player back on their team's spawns at full health 
// player i of a team gets that team's spawn 
void reset_to_spawns(World& world, const Map& map) {
    std::array<int, 2> placed{};  // per team, how many players have a spawn so far
    for (Player& player : world.players) {
        const int team = static_cast<int>(player.team);
        const TileCoord spawn = map.spawns(player.team)[placed[team]++];
        const Vec2 enemy_spawn = tile_centre(map.spawns(other(player.team)).front());

        player.position = tile_centre(spawn);
        player.velocity = {};
        const Vec2 to_enemy = enemy_spawn - player.position;
        player.aim = std::atan2(to_enemy.y, to_enemy.x);
        player.health = kMaxHealth;
        player.alive = true;
        player.fire_cooldown = 0;
    }
}

void move_players(World& world, const Inputs& inputs, const Map& map, float dt) {
    for (Player& player : world.players) {
        if (!player.alive) {
            continue;
        }
        const PlayerInput& input = inputs[player.id];

        const Vec2 direction = normalized({static_cast<float>(input.move_x),
                                           static_cast<float>(input.move_y)});
        const Vec2 start = player.position;
        player.position = push_out_of_walls(start + direction * kMoveSpeed * dt, map);
        player.velocity = (player.position - start) * (1 / dt);  // what actually happened, walls included
        player.aim = input.aim;
        if (player.fire_cooldown > 0) {
            player.fire_cooldown -= dt;
        }
    }
}

// damage is collected first and applied after every shot
void shoot(World& world, const Inputs& inputs, const Map& map) {
    std::array<int, kMaxPlayers> damage{};
    for (Player& shooter : world.players) {
        if (!shooter.alive || !inputs[shooter.id].fire || shooter.fire_cooldown > 0) {
            continue;
        }
        // += keeps the leftover fraction of a tick, so holding fire averages exactly 10 shots/s.
        shooter.fire_cooldown += kFireInterval;

        const Vec2 dir{std::cos(shooter.aim), std::sin(shooter.aim)};
        float range = ray_to_wall(map, shooter.position, dir);
        const Player* target = nullptr;
        for (const Player& other : world.players) {
            if (other.team == shooter.team || !other.alive) {
                continue;
            }
            const std::optional<float> t = ray_to_circle(shooter.position, dir, other.position, kPlayerRadius);
            if (t && *t < range) {
                range = *t;
                target = &other;
            }
        }

        if (target) {
            damage[target->id] += kShotDamage;
        }
        world.shots.push_back({shooter.id, shooter.position, shooter.position + dir * range, target != nullptr});
    }

    for (Player& player : world.players) {
        if (damage[player.id] == 0) {
            continue;
        }
        player.health = std::max(player.health - damage[player.id], 0);
        if (player.health == 0) {
            player.alive = false;
        }
    }
}

// a wiped team ends the round:
void end_round_if_wiped(World& world) {
    std::array<bool, 2> any_alive{};
    for (const Player& player : world.players) {
        if (player.alive) {
            any_alive[static_cast<int>(player.team)] = true;
        }
    }
    if (any_alive[0] && any_alive[1]) {
        return;
    }
    if (any_alive[0] || any_alive[1]) {
        ++world.score[any_alive[0] ? 0 : 1];
    }
    world.phase = Phase::RoundOver;
    world.round_timer = kRoundOverSeconds;
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
        const std::size_t spawns = map.spawns(team).size();
        if (static_cast<int>(spawns) < players_per_team) {
            throw std::runtime_error(std::format(
                "team {} has {} spawns, needs {}", team == Team::A ? 'A' : 'B', spawns, players_per_team));
        }
        for (int i = 0; i < players_per_team; ++i) {
            Player player;
            player.id = next_id++;
            player.team = team;
            world.players.push_back(player);
        }
    }

    reset_to_spawns(world, map);
    return world;
}

World step(const World& world, const Inputs& inputs, const Map& map, float dt) {
    World next = world;
    next.tick = world.tick + 1;
    next.shots.clear();

    if (next.phase == Phase::RoundOver) {
        next.round_timer -= dt;
        if (next.round_timer <= 0) {
            reset_to_spawns(next, map);
            next.phase = Phase::Playing;
            next.round_timer = 0;
            ++next.round;
        }
    }

    move_players(next, inputs, map, dt);

    if (next.phase == Phase::Playing) {
        shoot(next, inputs, map);
        end_round_if_wiped(next);
    }

    return next;
}

}
