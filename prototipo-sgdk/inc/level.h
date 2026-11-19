#ifndef LEVEL_H
#define LEVEL_H

#include <genesis.h>

enum
{
    SCREEN_WIDTH = 256,
    SCREEN_HEIGHT = 224,
    LEVEL_COLUMNS = 160,
    LEVEL_WIDTH = LEVEL_COLUMNS * 8,
    LEVEL_EXIT = 1248,
    TARGET_COUNT = 5
};

extern const Vect2D_s16 targetPositions[TARGET_COUNT];

bool levelSolid(s16 tileX, s16 tileY);
void levelInit(void);
void levelDraw(s16 camera);

#endif
