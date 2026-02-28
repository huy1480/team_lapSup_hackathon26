#include "character.h"
#include "raymath.h" // Gives us Vector2Distance, Vector2Normalize, etc.
#include <stddef.h>
#include <stdio.h>

void InitCharacter(Character *player, Vector2 startPos, const char* texturePath) {
    player->position = startPos;
    player->targetPosition = startPos;
    player->texture = LoadTexture(texturePath);
    player->speed = 150.0f; // Pixels per second
    player->isWalking = false;
    player->stepToggle = false;
    player->stepTimer = 0.0f;
    player->stepInterval = 0.15f;
    player->heldItem = NULL;

    player->leftStepTexture = LoadTexture("assets/left_step.png"); // Replace with actual path
    if (player->leftStepTexture.id == 0) {
        printf("ERROR: Failed to load left step texture.\n");
    }

    player->rightStepTexture = LoadTexture("assets/right_step.png"); // Replace with actual path
    if (player->rightStepTexture.id == 0) {
        printf("ERROR: Failed to load right step texture.\n");
    }
}

void UpdateCharacter(Character *player) {
    // Check how far we are from the target
    float distance = Vector2Distance(player->position, player->targetPosition);

    // If we are further than 1 pixel away, keep moving
    // (We use > 1.0f instead of > 0 to prevent the character from jittering back and forth)
    if (distance > 1.0f) {
        player->isWalking = true;

        // 1. Get the direction vector (Target - Current Position)
        Vector2 direction = Vector2Subtract(player->targetPosition, player->position);
        
        // 2. Normalize it (make its length exactly 1)
        direction = Vector2Normalize(direction);
        
        // 3. Move the player based on speed and frame time
        player->position.x += direction.x * player->speed * GetFrameTime();
        player->position.y += direction.y * player->speed * GetFrameTime();

        player->stepTimer += GetFrameTime();
        if (player->stepTimer >= player->stepInterval) {
            player->stepTimer = 0.0f;
            player->stepToggle = !player->stepToggle; // Toggle between left and right step
        }
    } else {
        player->isWalking = false;
        player->stepTimer = 0.0f;
    }
}

void DrawCharacter(Character *player) {
    Texture2D textureToDraw;

    if (player->isWalking) {
        // Use the left or right step texture based on the step toggle
        textureToDraw = player->stepToggle ? player->leftStepTexture : player->rightStepTexture;
    } else {
        // Use the standing texture when not walking
        textureToDraw = player->texture;
    }

    if (textureToDraw.id != 0) {
        // Draw the selected texture centered on the character's position
        Vector2 origin = { textureToDraw.width / 2.0f, textureToDraw.height / 2.0f };
        Rectangle source = { 0, 0, (float)textureToDraw.width, (float)textureToDraw.height };
        Rectangle dest = { player->position.x, player->position.y,
                           (float)textureToDraw.width, (float)textureToDraw.height };

        DrawTexturePro(textureToDraw, source, dest, origin, 0.0f, WHITE);
    } else {
        // Fallback: draw a red circle if the texture fails to load
        DrawCircleV(player->position, 10.0f, RED);
    }
}

void UnloadCharacter(Character *player) {
    UnloadTexture(player->texture);
    UnloadTexture(player->leftStepTexture);
    UnloadTexture(player->rightStepTexture);
}
