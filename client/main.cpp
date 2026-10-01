#include <sightline/config.h>
#include <sightline/map.h>
#include <sightline/sim.h>

#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <optional>
#include <vector>

namespace {

constexpr int kPixelsPerTile = 32;
constexpr float kMaxFrameTime = 0.25f;   // after a stall, don't try to catch up seconds of ticks
constexpr float kTracerSeconds = 0.08f;

// make_world gives ids in order, so a player's id is also its index in World::players.
constexpr sightline::PlayerId kLocalPlayer = 0;
constexpr sightline::PlayerId kDummy = 1;

constexpr Color kFloor{236, 234, 228, 255};
constexpr Color kWall{58, 60, 66, 255};
constexpr Color kTeamColor[2]{{59, 130, 246, 255}, {232, 72, 85, 255}};

// A shot kept on screen for a few frames. Client-only: World::shots holds just the latest tick's
// shots, and one frame can run several ticks, so without this most tracers would never be drawn.
struct Tracer {
    sightline::Shot shot;
    float seconds_left;
};

Vector2 to_screen(sightline::Vec2 v) {
    return {v.x * kPixelsPerTile, v.y * kPixelsPerTile};
}

sightline::PlayerInput read_input(const sightline::Player& me) {
    sightline::PlayerInput input;
    input.move_x = static_cast<std::int8_t>(IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
    input.move_y = static_cast<std::int8_t>(IsKeyDown(KEY_S) - IsKeyDown(KEY_W));
    const Vector2 mouse = GetMousePosition();
    input.aim = std::atan2(mouse.y / kPixelsPerTile - me.position.y,
                           mouse.x / kPixelsPerTile - me.position.x);
    return input;
}

void draw_map(const sightline::Map& map) {
    for (sightline::Team team : {sightline::Team::A, sightline::Team::B}) {
        for (sightline::TileCoord spawn : map.spawns(team)) {
            DrawRectangle(spawn.x * kPixelsPerTile, spawn.y * kPixelsPerTile, kPixelsPerTile,
                          kPixelsPerTile, Fade(kTeamColor[static_cast<int>(team)], 0.15f));
        }
    }
    for (int y = 0; y < map.height(); ++y) {
        for (int x = 0; x < map.width(); ++x) {
            if (map.is_wall(x, y)) {
                DrawRectangle(x * kPixelsPerTile, y * kPixelsPerTile, kPixelsPerTile, kPixelsPerTile, kWall);
            }
        }
    }
}

void draw_tracers(const std::vector<Tracer>& tracers) {
    for (const Tracer& tracer : tracers) {
        const float alpha = tracer.seconds_left / kTracerSeconds;
        const Color color = tracer.shot.hit ? ORANGE : GRAY;
        DrawLineEx(to_screen(tracer.shot.from), to_screen(tracer.shot.to), 2, Fade(color, alpha));
        if (tracer.shot.hit) {
            DrawCircleV(to_screen(tracer.shot.to), 3, Fade(color, alpha));
        }
    }
}

void draw_players(const sightline::World& world) {
    const float radius = sightline::kPlayerRadius * kPixelsPerTile;
    for (const sightline::Player& player : world.players) {
        const Vector2 centre = to_screen(player.position);
        const Color color = kTeamColor[static_cast<int>(player.team)];
        if (!player.alive) {
            DrawCircleV(centre, radius, Fade(color, 0.25f));
            continue;
        }

        DrawCircleV(centre, radius, color);
        const sightline::Vec2 aim{std::cos(player.aim), std::sin(player.aim)};
        DrawLineEx(centre, to_screen(player.position + aim * 0.8f), 2, color);

        const float bar_width = 0.8f * kPixelsPerTile;
        const float bar_x = centre.x - bar_width / 2;
        const float bar_y = centre.y - radius - 8;
        DrawRectangleV({bar_x, bar_y}, {bar_width, 4}, Fade(BLACK, 0.3f));
        DrawRectangleV({bar_x, bar_y},
                       {bar_width * player.health / sightline::kMaxHealth, 4}, LIME);
    }
}

// Drawn over the top wall row.
void draw_hud(const sightline::World& world) {
    const sightline::Player& me = world.players[kLocalPlayer];
    DrawText(TextFormat("Round %d    A %d - %d B    HP %d    %d FPS", world.round, world.score[0],
                        world.score[1], me.health, GetFPS()),
             8, 8, 16, RAYWHITE);

    if (world.phase == sightline::Phase::RoundOver) {
        const char* text = TextFormat("Round over - next round in %.1f s", world.round_timer);
        const int width = MeasureText(text, 24);
        const int x = (GetScreenWidth() - width) / 2;
        const int y = GetScreenHeight() / 2 - 12;
        DrawRectangle(x - 12, y - 8, width + 24, 40, Fade(BLACK, 0.6f));
        DrawText(text, x, y, 24, RAYWHITE);
    }
}

}

int main(int argc, char** argv)
{
    const sightline::ServerConfig config;
    const float dt = static_cast<float>(sightline::tick_duration_ms(config) / 1000.0);
    const char* map_path = argc > 1 ? argv[1] : SIGHTLINE_MAPS_DIR "/arena.txt";

    std::optional<sightline::Map> loaded;
    try {
        loaded = sightline::load_map(map_path);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
    const sightline::Map& map = *loaded;

    sightline::World world = sightline::make_world(map, 1);
    std::vector<Tracer> tracers;
    float accumulator = 0;
    bool fire_queued = false;  // a click seen on a frame that ran no tick still fires on the next one

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(map.width() * kPixelsPerTile, map.height() * kPixelsPerTile, "Sightline");

    while (!WindowShouldClose()) {
        const float frame_time = std::min(GetFrameTime(), kMaxFrameTime);
        accumulator += frame_time;
        fire_queued = fire_queued || IsMouseButtonDown(MOUSE_BUTTON_LEFT);

        // Fixed timestep: step() always advances exactly dt, however long the frame took.
        while (accumulator >= dt) {
            sightline::Inputs inputs{};
            inputs[kLocalPlayer] = read_input(world.players[kLocalPlayer]);
            inputs[kLocalPlayer].fire = fire_queued;
            inputs[kLocalPlayer].tick = world.tick;
            inputs[kDummy].aim = world.players[kDummy].aim;  // stands still, keeps facing the same way

            world = sightline::step(world, inputs, map, dt);
            for (const sightline::Shot& shot : world.shots) {
                tracers.push_back({shot, kTracerSeconds});
            }
            fire_queued = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
            accumulator -= dt;
        }

        for (Tracer& tracer : tracers) {
            tracer.seconds_left -= frame_time;
        }
        std::erase_if(tracers, [](const Tracer& tracer) { return tracer.seconds_left <= 0; });

        BeginDrawing();
        ClearBackground(kFloor);
        draw_map(map);
        draw_tracers(tracers);
        draw_players(world);
        draw_hud(world);
        EndDrawing();
    }

    CloseWindow();
}
