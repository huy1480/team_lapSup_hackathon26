#include "raylib.h"

#define SCREEN_WIDTH (1024)
#define SCREEN_HEIGHT (576)
#define WINDOW_TITLE "Lego Western Town - Auto-Follow Camera"

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    Texture2D background = LoadTexture("assets/bg.png");

    Camera2D camera = { 0 };
    camera.zoom = 5.0f; // Permanently zoomed in 2x
    
    // Anchor the camera to the exact center of your screen
    camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.target = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

    while (!WindowShouldClose())
    {
        // --- Auto-Follow Logic ---
        // Get where the mouse is on the screen
        Vector2 desiredTarget = GetMousePosition();
        
        // --- Boundary Clamping ---
        // Calculate the minimum and maximum coordinates the camera can look at 
        // without showing the background behind the image.
        float minX = (SCREEN_WIDTH / 2.0f) / camera.zoom;
        float maxX = SCREEN_WIDTH - minX;
        float minY = (SCREEN_HEIGHT / 2.0f) / camera.zoom;
        float maxY = SCREEN_HEIGHT - minY;

        // Force the desired target to stay within these boundaries
        if (desiredTarget.x < minX) desiredTarget.x = minX;
        if (desiredTarget.x > maxX) desiredTarget.x = maxX;
        if (desiredTarget.y < minY) desiredTarget.y = minY;
        if (desiredTarget.y > maxY) desiredTarget.y = maxY;

        // Smoothly glide the camera toward the mouse position
        // The 0.05f dictates the speed. Lower = "floatier" and smoother, Higher = faster and snappier.
        camera.target.x += (desiredTarget.x - camera.target.x) * 0.05f;
        camera.target.y += (desiredTarget.y - camera.target.y) * 0.05f;

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);

        if (background.id != 0) 
        {
            Rectangle sourceRec = { 0.0f, 0.0f, (float)background.width, (float)background.height };
            // We scale the image to the screen size. 
            // The camera zoom takes care of magnifying it!
            Rectangle destRec = { 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
            DrawTexturePro(background, sourceRec, destRec, (Vector2){0,0}, 0.0f, WHITE);
        }
        else
        {
            DrawText("IMAGE FAILED TO LOAD! Check file path.", 100, 100, 30, RED);
        }

        EndMode2D();

        // UI Layer
        DrawRectangle(10, 10, 310, 30, Fade(BLACK, 0.7f));
        DrawText("Move mouse to explore the town!", 20, 15, 18, RAYWHITE);

        EndDrawing();
    }

    UnloadTexture(background); 
    CloseWindow();

    return 0;
}
