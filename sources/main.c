#include "raylib.h"
#include "character.h" // Include our new header!

#define SCREEN_WIDTH (1024)
#define SCREEN_HEIGHT (576)
#define WINDOW_TITLE "Lego Western Town - Point & Click"

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    Texture2D background = LoadTexture("assets/bg.png");

    // Initialize our character in the center of the screen
    Character player;
    InitCharacter(&player, (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }, "assets/character.png");

    Camera2D camera = { 0 };
    camera.zoom = 3.0f; 
    camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.target = player.position;

    while (!WindowShouldClose())
    {
        // --- Input Logic ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            // CRITICAL: Translate the screen click to world coordinates
            player.targetPosition = GetScreenToWorld2D(GetMousePosition(), camera);
        }

        // --- Update Logic ---
        UpdateCharacter(&player);

        // --- Camera Logic ---
        Vector2 desiredCameraTarget = player.position;

        // Boundary Clamping (keeps camera from showing out-of-bounds void)
        float minX = (SCREEN_WIDTH / 2.0f) / camera.zoom;
        float maxX = SCREEN_WIDTH - minX;
        float minY = (SCREEN_HEIGHT / 2.0f) / camera.zoom;
        float maxY = SCREEN_HEIGHT - minY;

        if (desiredCameraTarget.x < minX) desiredCameraTarget.x = minX;
        if (desiredCameraTarget.x > maxX) desiredCameraTarget.x = maxX;
        if (desiredCameraTarget.y < minY) desiredCameraTarget.y = minY;
        if (desiredCameraTarget.y > maxY) desiredCameraTarget.y = maxY;

        // Smoothly glide camera to the clamped target
        camera.target.x += (desiredCameraTarget.x - camera.target.x) * 0.05f;
        camera.target.y += (desiredCameraTarget.y - camera.target.y) * 0.05f;

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);

        if (background.id != 0) 
        {
            Rectangle sourceRec = { 0.0f, 0.0f, (float)background.width, (float)background.height };
            Rectangle destRec = { 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
            DrawTexturePro(background, sourceRec, destRec, (Vector2){0,0}, 0.0f, WHITE);
        }

        // Draw the player on top of the background
        DrawCharacter(&player);

        EndMode2D();

        // UI Layer
        DrawRectangle(10, 10, 310, 30, Fade(BLACK, 0.7f));
        DrawText("Click to move the character!", 20, 15, 18, RAYWHITE);

        EndDrawing();
    }

    // Unload assets
    UnloadCharacter(&player);
    UnloadTexture(background); 
    CloseWindow();

    return 0;
}
