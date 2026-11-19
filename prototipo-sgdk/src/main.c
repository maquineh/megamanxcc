#include "art.h"
#include "level.h"

enum
{
    SUBPIXEL = 256,
    WALK_SPEED = 376,
    WALK_ACCEL = 128,
    JUMP_SPEED = 1364,
    GRAVITY = 64,
    MAX_FALL = 1536,
    PLAYER_HALF_WIDTH = 6,
    PLAYER_HEIGHT = 24,
    CAMERA_MARGIN = 28,
    SHOT_SPEED = 4,
    SHOT_LIMIT = 5,
    SHOT_COOLDOWN = 12
};

typedef struct
{
    s32 x, feet;
    s16 vx, vy;
    bool grounded, left;
} Player;

typedef struct
{
    Sprite* sprite;
    s16 x, y, dx;
    u16 lifetime;
} Shot;

static Player player;
static Sprite* playerSprite;
static Shot shots[SHOT_LIMIT];
static Sprite* targetSprites[TARGET_COUNT];
static u16 hitTargets, shotCooldown, frame;
static s16 camera;
static bool finished;

static s16 pixel(s32 position)
{
    return (s16)(position >> 8);
}

static void drawHud(void)
{
    char label[] = "ALVOS 0/5";
    label[6] += hitTargets;
    VDP_drawText(label, 22, 0);
    VDP_drawText(finished ? "BALIZA ALCANCADA   START: VOLTA" : "D-PAD    B: SALTO    A/C: TIRO ", 1, 1);
}

static void restart(void)
{
    player = (Player) { 40 * SUBPIXEL, 184 * SUBPIXEL, 0, 0, TRUE, FALSE };
    camera = 0;
    frame = 0;
    for (u16 i = 0; i < TARGET_COUNT; i++) SPR_setVisibility(targetSprites[i], AUTO_FAST);
    for (u16 i = 0; i < SHOT_LIMIT; i++) shots[i].lifetime = 0;
    hitTargets = 0;
    shotCooldown = 0;
    finished = FALSE;
    drawHud();
}

static void moveHorizontal(s16 direction)
{
    s16 speed = direction * WALK_SPEED;
    player.vx += clamp(speed - player.vx, -WALK_ACCEL, WALK_ACCEL);
    if (direction) player.left = direction < 0;

    s32 next = player.x + player.vx;
    s16 center = pixel(next);
    if (center < PLAYER_HALF_WIDTH || center > LEVEL_WIDTH - PLAYER_HALF_WIDTH)
    {
        player.x = clamp(center, PLAYER_HALF_WIDTH, LEVEL_WIDTH - PLAYER_HALF_WIDTH) * (s32)SUBPIXEL;
        player.vx = 0;
        return;
    }
    if (player.vx == 0) return;

    s16 side = player.vx > 0 ? center + PLAYER_HALF_WIDTH - 1 : center - PLAYER_HALF_WIDTH;
    s16 column = side >> 3;
    s16 feet = pixel(player.feet);
    for (s16 row = (feet - PLAYER_HEIGHT) >> 3; row <= (feet - 1) >> 3; row++)
    {
        if (!levelSolid(column, row)) continue;
        s16 edge = player.vx > 0 ? column * 8 - PLAYER_HALF_WIDTH : (column + 1) * 8 + PLAYER_HALF_WIDTH;
        player.x = edge * (s32)SUBPIXEL;
        player.vx = 0;
        return;
    }
    player.x = next;
}

static void moveVertical(u16 held, u16 pressed)
{
    if ((pressed & BUTTON_B) && player.grounded) player.vy = -JUMP_SPEED;
    if (!(held & BUTTON_B) && player.vy < -2 * SUBPIXEL) player.vy = -2 * SUBPIXEL;
    player.vy = clamp(player.vy + GRAVITY, -JUMP_SPEED, MAX_FALL);

    s32 next = player.feet + player.vy;
    bool falling = player.vy >= 0;
    s16 edge = pixel(next) - (falling ? 0 : PLAYER_HEIGHT);
    s16 row = edge >> 3;
    s16 center = pixel(player.x);
    player.grounded = FALSE;
    for (s16 column = (center - PLAYER_HALF_WIDTH) >> 3; column <= (center + PLAYER_HALF_WIDTH - 1) >> 3; column++)
    {
        if (!levelSolid(column, row)) continue;
        player.feet = (falling ? row * 8 : (row + 1) * 8 + PLAYER_HEIGHT) * (s32)SUBPIXEL;
        player.vy = 0;
        player.grounded = falling;
        return;
    }
    player.feet = next;
}

static void fire(void)
{
    for (u16 i = 0; i < SHOT_LIMIT; i++)
    {
        Shot* shot = &shots[i];
        if (shot->lifetime) continue;
        shot->dx = player.left ? -SHOT_SPEED : SHOT_SPEED;
        shot->x = pixel(player.x) + (player.left ? -12 : 12);
        shot->y = pixel(player.feet) - 12;
        shot->lifetime = 60;
        shotCooldown = SHOT_COOLDOWN;
        return;
    }
}

static void updateShots(u16 held)
{
    if (shotCooldown) shotCooldown--;
    if ((held & (BUTTON_A | BUTTON_C)) && !shotCooldown) fire();

    for (u16 i = 0; i < SHOT_LIMIT; i++)
    {
        Shot* shot = &shots[i];
        if (!shot->lifetime) continue;
        shot->lifetime--;
        shot->x += shot->dx;
        if (shot->x < 0 || shot->x >= LEVEL_WIDTH || levelSolid(shot->x >> 3, shot->y >> 3))
        {
            shot->lifetime = 0;
            continue;
        }
        for (u16 target = 0; target < TARGET_COUNT; target++)
        {
            if (SPR_getVisibility(targetSprites[target]) == HIDDEN) continue;
            s16 dx = shot->x - targetPositions[target].x;
            s16 dy = shot->y - targetPositions[target].y;
            if (dx < -9 || dx > 9 || dy < -8 || dy > 8) continue;
            SPR_setVisibility(targetSprites[target], HIDDEN);
            hitTargets++;
            shot->lifetime = 0;
            drawHud();
            break;
        }
    }
}

static void drawFrame(void)
{
    s16 x = pixel(player.x);
    s16 screenX = x - camera;
    s16 leftEdge = SCREEN_WIDTH / 2 - CAMERA_MARGIN;
    s16 rightEdge = SCREEN_WIDTH / 2 + CAMERA_MARGIN;
    if (screenX < leftEdge) camera = x - leftEdge;
    if (screenX > rightEdge) camera = x - rightEdge;
    camera = clamp(camera, 0, LEVEL_WIDTH - SCREEN_WIDTH);

    levelDraw(camera);
    VDP_setHorizontalScrollVSync(BG_A, -camera);
    VDP_setHorizontalScrollVSync(BG_B, -(camera >> 2));

    u16 pose = 0;
    if (!player.grounded) pose = 4;
    else if (player.vx) pose = 1 + ((frame >> 3) % 3);
    SPR_setFrame(playerSprite, pose);
    SPR_setHFlip(playerSprite, player.left);
    SPR_setPosition(playerSprite, x - camera - 12, pixel(player.feet) - 29);

    for (u16 i = 0; i < TARGET_COUNT; i++)
        SPR_setPosition(targetSprites[i], targetPositions[i].x - camera - 8, targetPositions[i].y - 8);

    for (u16 i = 0; i < SHOT_LIMIT; i++)
    {
        Shot* shot = &shots[i];
        SPR_setPosition(shot->sprite, shot->x - camera - 4, shot->y - 4);
        SPR_setVisibility(shot->sprite, shot->lifetime ? AUTO_FAST : HIDDEN);
    }
    SPR_update();
}

int main(bool hardReset)
{
    const u16 spriteAttr = TILE_ATTR(PAL2, FALSE, FALSE, FALSE);
    const u16 hudPalette[16] = { [1] = RGB24_TO_VDPCOLOR(0x182134), [15] = RGB24_TO_VDPCOLOR(0xffead0) };

    VDP_setEnable(FALSE);
    VDP_setScreenWidth256();
    levelInit();
    PAL_setPalette(PAL2, runner.palette->data, CPU);
    PAL_setPalette(PAL3, hudPalette, CPU);
    VDP_setWindowVPos(FALSE, 2);
    VDP_setTextPlane(WINDOW);
    VDP_setTextPalette(PAL3);
    VDP_fillTileMapRect(WINDOW, TILE_ATTR_FULL(PAL3, TRUE, FALSE, FALSE, TILE_SYSTEM_INDEX + 1), 0, 0, 32, 2);
    VDP_drawText("MARE DE COBRE", 1, 0);

    SPR_initEx(128);
    playerSprite = SPR_addSprite(&runner, 0, 0, spriteAttr);
    for (u16 i = 0; i < TARGET_COUNT; i++) targetSprites[i] = SPR_addSprite(&targetArt, 0, 0, spriteAttr);
    for (u16 i = 0; i < SHOT_LIMIT; i++) shots[i].sprite = SPR_addSprite(&pulse, 0, 0, spriteAttr);

    restart();
    drawFrame();
    SYS_doVBlankProcess();
    VDP_setEnable(TRUE);

    u16 previousInput = 0;
    while (TRUE)
    {
        u16 held = JOY_readJoypad(JOY_1);
        u16 pressed = held & ~previousInput;
        previousInput = held;
        if (pressed & BUTTON_START) restart();
        else
        {
            frame++;
            moveHorizontal(!!(held & BUTTON_RIGHT) - !!(held & BUTTON_LEFT));
            moveVertical(held, pressed);
            updateShots(held);
            if (pixel(player.feet) > SCREEN_HEIGHT + PLAYER_HEIGHT) restart();
            else if (!finished && pixel(player.x) >= LEVEL_EXIT)
            {
                finished = TRUE;
                drawHud();
            }
        }
        drawFrame();
        SYS_doVBlankProcess();
    }
}
