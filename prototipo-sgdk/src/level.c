#include "art.h"
#include "level.h"

enum { PLANE_COLUMNS = 64, VISIBLE_COLUMNS = SCREEN_WIDTH / 8 + 1, VISIBLE_ROWS = SCREEN_HEIGHT / 8 };

typedef struct
{
    s16 left, right, top, bottom;
} Platform;

// Coordinates in tiles, with exclusive right and bottom edges.
static const Platform platforms[] = {
    { 0, 27, 23, 28 }, { 17, 22, 19, 20 },
    { 32, 59, 23, 28 }, { 41, 46, 20, 23 },
    { 62, 88, 21, 28 }, { 72, 77, 17, 18 },
    { 93, 114, 23, 28 }, { 108, 114, 20, 23 },
    { 118, 138, 20, 28 }, { 143, LEVEL_COLUMNS, 23, 28 }
};

const Vect2D_s16 targetPositions[TARGET_COUNT] = {
    { 104, 171 }, { 280, 171 }, { 544, 155 }, { 824, 171 }, { 1016, 147 }
};

static u16 metalIndex;
static s16 previousColumn = -VISIBLE_COLUMNS;

static const Platform* platformAt(s16 x, s16 y)
{
    for (u16 i = 0; i < sizeof(platforms) / sizeof(platforms[0]); i++)
    {
        const Platform* platform = &platforms[i];
        if (x >= platform->left && x < platform->right && y >= platform->top && y < platform->bottom)
            return platform;
    }
    return NULL;
}

bool levelSolid(s16 tileX, s16 tileY)
{
    return platformAt(tileX, tileY) != NULL;
}

static u16 metalTile(const Platform* platform, s16 x, s16 y)
{
    if (y == platform->top)
    {
        if (x >= LEVEL_EXIT / 8) return 4;
        if (x == platform->left) return 1;
        if (x == platform->right - 1) return 2;
        return (x & 7) == 4 ? 3 : 0;
    }
    if (x == platform->left) return 5;
    if (x == platform->right - 1) return 6;
    if (y == platform->top + 1) return 8;
    if ((x & 7) == 4) return 10;
    if ((x & 7) == 5) return 11;
    return (y & 1) ? 9 : 7;
}

static void drawColumn(s16 x)
{
    u16 column[VISIBLE_ROWS];
    for (s16 y = 0; y < VISIBLE_ROWS; y++)
    {
        const Platform* platform = platformAt(x, y);
        column[y] = platform ? TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, metalIndex + metalTile(platform, x, y)) : 0;
    }
    VDP_setTileMapDataColumnFast(BG_A, column, x & (PLANE_COLUMNS - 1), 0, VISIBLE_ROWS, DMA_QUEUE_COPY);
}

void levelInit(void)
{
    VDP_setPlaneSize(PLANE_COLUMNS, 32, TRUE);
    VDP_drawImageEx(BG_B, &estuary, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX), 0, 0, TRUE, TRUE);
    metalIndex = TILE_USER_INDEX + estuary.tileset->numTile;
    PAL_setPalette(PAL1, metalPalette.data, CPU);
    VDP_loadTileSet(&metalTiles, metalIndex, DMA);
}

void levelDraw(s16 camera)
{
    s16 first = camera >> 3;
    if (first == previousColumn) return;

    // Repaint the new edge of the visible tilemap. A restart can jump across the entire map.
    s16 left = first;
    s16 right = first + VISIBLE_COLUMNS;
    if (first > previousColumn && previousColumn + VISIBLE_COLUMNS > left)
        left = previousColumn + VISIBLE_COLUMNS;
    if (first < previousColumn && previousColumn < right)
        right = previousColumn;
    for (s16 x = left; x < right; x++) drawColumn(x);
    previousColumn = first;
}
