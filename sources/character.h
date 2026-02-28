#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"

typedef struct Character {
    Vector2 position;
    Vector2 targetPosition; // Where the character is walking to
    Texture2D texture;
    float speed;
    bool isWalking;
    bool stepToggle;
    float stepTimer;
    float stepInterval;
    const char* heldItem;
    Texture2D leftStepTexture;  // Texture for left step
    Texture2D rightStepTexture; // Texture for right step
} Character;

// Function declarations
void InitCharacter(Character *player, Vector2 startPos, const char* texturePath);
void UpdateCharacter(Character *player);
void DrawCharacter(Character *player);
void UnloadCharacter(Character *player);

#endif
