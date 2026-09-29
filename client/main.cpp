#include <sightline/config.h>

#include <raylib.h>

int main()
{
    const sightline::ServerConfig config;

    InitWindow(1280, 720, "Sightline");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText(TextFormat("tick rate: %d Hz (%.4f ms)", config.tick_rate,
                            sightline::tick_duration_ms(config)),
                 20, 20, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
}
