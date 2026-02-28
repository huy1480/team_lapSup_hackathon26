#ifndef ITEM_H
#define ITEM_H

#include "raylib.h"
#include "scene_manager.h"

typedef struct Item {
    Vector2 position;
    const char* name;
    bool active;     // If true, it's on the ground. If false, it's been collected.
    SceneType scene; // Which scene this item lives in
} Item;

#endif
