#include "character.h"
#include "raymath.h" // Gives us Vector2Distance, Vector2Normalize, etc.
#include <stddef.h>
#include <stdio.h>

void InitCharacter(Character *player, Vector2 startPos, const char* texturePath) {
    player->position = startPos;
    player->targetPosition = startPos;
    player->texture = LoadTexture(texturePath);
    //player->speed = 150.0f; // Pixels per second
    player->heldItem = NULL;
    player->isWalking = false;
    player->stepTimer = 0.0f;
    player->stepInterval = 0.2f; // Adjust as needed
    player->stepToggle = false;
    player->leftStepTexture = LoadTexture("assets/left_step.png"); // Replace with actual path
    if (player->leftStepTexture.id == 0) {
        printf("ERROR: Failed to load left step texture.\n");
    }

    player->rightStepTexture = LoadTexture("assets/right_step.png"); // Replace with actual path
    if (player->rightStepTexture.id == 0) {
        printf("ERROR: Failed to load right step texture.\n");
    }
}

void UpdateCharacter(Character* character) {
    // Check if the character is moving
    if (Vector2Distance(character->position, character->targetPosition) > 1.0f) {
        character->isWalking = true;

        // Move the character toward the target position
        Vector2 direction = Vector2Normalize(Vector2Subtract(character->targetPosition, character->position));
        character->position = Vector2Add(character->position, Vector2Scale(direction, 2.0f)); // Adjust speed as needed

        // Handle sprite switching
        character->stepTimer += GetFrameTime();
        if (character->stepTimer >= character->stepInterval) {
            character->stepTimer = 0.0f;
            character->stepToggle = !character->stepToggle; // Toggle between 0 and 1
        }
    } else {
        character->isWalking = false;
    }
}

void DrawCharacter(Character* character) {
    if (character->isWalking) {
        if (character->stepToggle) {
            if (character->leftStepTexture.id != 0) {
                DrawTexture(character->leftStepTexture, character->position.x, character->position.y, WHITE);
            } else {
                printf("ERROR: Left step texture not loaded.\n");
            }
        } else {
            if (character->rightStepTexture.id != 0) {
                DrawTexture(character->rightStepTexture, character->position.x, character->position.y, WHITE);
            } else {
                printf("ERROR: Right step texture not loaded.\n");
            }
        }
    } else {
        DrawTexture(character->texture, character->position.x, character->position.y, WHITE);
    }
}

void UnloadCharacter(Character *player) {
    UnloadTexture(player->texture);
    UnloadTexture(player->leftStepTexture);
    UnloadTexture(player->rightStepTexture);
}
