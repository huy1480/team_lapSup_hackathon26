#include "raylib.h"

// Increased the window size a bit for a better view of the landscape
#define SCREEN_WIDTH (1024)
#define SCREEN_HEIGHT (576)

#define WINDOW_TITLE "Lego Western Town Background"

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    // Load the attached image. 
    // IMPORTANT: Make sure "AI_Upscaled1772255993.jpg" is in the same directory 
    // as your executable (or update the path accordingly).
    Texture2D background = LoadTexture("assets/bg.png");

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw the background image scaled to fit the screen
        Rectangle sourceRec = { 0.0f, 0.0f, (float)background.width, (float)background.height };
        Rectangle destRec = { 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
        Vector2 origin = { 0.0f, 0.0f };
        
        // DrawTexturePro allows us to stretch/scale the image to the destRec dimensions
        DrawTexturePro(background, sourceRec, destRec, origin, 0.0f, WHITE);

        // Draw some overlay text at the top
        const char* text = "Welcome to the Lego Western Town!";
        int fontSize = 30;
        const Vector2 text_size = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
        
        // Added a slight shadow effect to make the text readable over the busy background
        DrawText(text, SCREEN_WIDTH / 2 - text_size.x / 2 + 2, 22, fontSize, BLACK);
        DrawText(text, SCREEN_WIDTH / 2 - text_size.x / 2, 20, fontSize, RAYWHITE);

        EndDrawing();
    }

    // Always unload textures when you are done with them to prevent memory leaks
    UnloadTexture(background); 
    
    CloseWindow();

    return 0;
}
