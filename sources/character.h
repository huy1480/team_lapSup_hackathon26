#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"

typedef struct Character {
    Vector2 position;
    Vector2 targetPosition;
    Texture2D texture; // Standing texture
    Texture2D leftStepTexture; // Texture for left step
    Texture2D rightStepTexture; // Texture for right step
    bool isWalking; // Indicates if the character is walking
    int stepToggle; // 0 for left step, 1 for right step
    float stepTimer; // Timer for step animation
    float stepInterval; // Interval between step animations
    const char* heldItem;
} Character;

// Function declarations
extern void InitCharacter(Character *player, Vector2 startPos, const char* texturePath);
extern void UpdateCharacter(Character *player);
extern void DrawCharacter(Character *player);
extern void UnloadCharacter(Character *player);

#endif
