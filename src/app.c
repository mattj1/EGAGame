// app.c

#include "common.h"
#ifdef PLATFORM_DOS
#include <alloc.h>
#endif

static TMap map;
static TEntity *plyr[2], *monster;

void shutdown(void) {
    EGA_Close();
    printf("Shutdown\n");
    Neo_Shutdown();
}

void Update(void) {
    if(Neo_WasKeyDown(kEsc)) {
        Neo_Quit();
    }
}

struct {
    ega_sprite_t sprites[SPRITE_STATE_MAX];


#ifdef PLATFORM_DESKTOP
    Image raylib_font;
    Image raylib_tiles;
#else
    ega_tile_t tiles[64];
    unsigned char font_data[3072];
#endif
} assets;


void draw_string(u16 x, u16 y, u16 color, const char *s) {
    u16 is_shift = 0;
    int i;
    u16 ch;
    ega_sprite_t fs;
    int l = strlen(s);
    x = x & 0xfffc;

#ifdef PLATFORM_DOS
    if ((x & 7) != 0) is_shift = 1;
    fs.column_count = 1;
    fs.width = 8;
    fs.height = 8;
    fs.num_channels = 2;
    fs.num_color_channels = 1;
    fs.planes_and = 0;
    fs.planes_or = color;

    for(i = 0; i < l; i++) {
        ch = s[i] - 32;
        if(ch != 0) {
            fs.data = &assets.font_data[(1536 * is_shift) + 16 * ch];
            EGA_DrawSpriteFast(x, y, &fs);
        }

        x += 4;
        is_shift = 1 - is_shift;
    }
#else
    Image img = EGA_Raylib_GetBackBuffer();
    Rectangle srcRect, dstRect;
    srcRect.width = 4;
    srcRect.height = 8;
    dstRect.width = 4;
    dstRect.height = 8;


    for(i = 0; i < l; i++) {
        ch = s[i] - 32;

        srcRect.x = (float) ((ch & 15) * 8);
        srcRect.y = (float) ((ch & 0xf0) >> 1);

        dstRect.x = (float) x + i * 4;
        dstRect.y = y;

        ImageDraw(&img, assets.raylib_font, srcRect, dstRect, WHITE);
    }
#endif
}

#ifdef PLATFORM_DESKTOP
void Raylib_DrawTile(u16 x, u16 y, Image srcImage, u16 t) {
    Rectangle srcRect, dstRect;
    Image img = EGA_Raylib_GetBackBuffer();

    srcRect.x = (float) (t & 7) * 16;
    srcRect.y = (float) (t & 0xfff8) * 2;
    srcRect.width = 16;
    srcRect.height = 16;

    dstRect.x = (float) x * 16;
    dstRect.y = (float) y * 16;
    dstRect.width = 16;
    dstRect.height = 16;

    ImageDraw(&img, srcImage, srcRect, dstRect, WHITE);
}
#endif

u16 mouse_x, mouse_y, mouse_buttons;
void Update2(void) {
    int i;

#ifdef PLATFORM_DOS
    //    unsigned char far *dst;
//    unsigned char far *src;
#endif

    Neo_Mouse_GetStatus(&mouse_x, &mouse_y, &mouse_buttons);
    mouse_x >>= 1;
#ifdef PLATFORM_DESKTOP
    mouse_y >>= 1;

    mouse_x /= 2.5;
    mouse_y /= 2.5;
#endif

    if (mouse_buttons & 1) {
        Player_SetTarget(plyr[0], mouse_x, mouse_y);
        Player_SetTarget(plyr[1], mouse_x, mouse_y);
    }


    for(i = 1; i < MAX_ENT; i++) {
        TEntity *e = EntityForIndex(i);
        if(!e->state) {
            continue;
        }

        if(e->flash_time > 0) {
            e->flash_time--;
        }

        if(e->info->frameFunc) {
            e->info->frameFunc(e);
        }

        e->stateTime--;
        if(e->stateTime == 0) {
            Entity_SetState(e, e->state->nextState);
            if(e->info->stateChangeFunc) {
                e->info->stateChangeFunc(e);
            }
        }
    }
}

void DrawEntity(TEntity *e) {
    ega_sprite_t *sprite = &assets.sprites[e->state->spriteState];
    ega_sprite_t s = *sprite;

    if(e->flash_time > 0) {
//         White
        s.planes_and = 0;
        s.planes_or = 0x0f;

        // Red (not very interesting)
//        s.planes_and = 0x04;
//        s.planes_or = 0x00;

        // Mouse highlight
//        s.planes_and = 0x07;
//        s.planes_or = 8;
    }

    EGA_DrawSpriteSlow(e->origin.x, e->origin.y, &s);
#ifdef PLATFORM_DESKTOP
    Image img = EGA_Raylib_GetBackBuffer();

    if(e->isMoving) {
        if (e->targetID) {
            TEntity *t = EntityForID(e->targetID);
            if (t) {

                bounds_t otherBounds;
                EntityBounds(t, &otherBounds);
                TVec2 center = BoundsCenter(otherBounds);

                ImageDrawLine(&img, e->origin.x, e->origin.y, center.x, center.y, RED);
            }
        } else {
            ImageDrawLine(&img, e->origin.x, e->origin.y, e->targetPos.x, e->targetPos.y, RED);
        }
    }
#endif
}

static int page = 0;
void Draw(void) {
    int x, y;
    Update2();

    page = 1 - page;
    EGA_SetDrawPage(page);

#ifdef PLATFORM_DESKTOP
    EGA_ClearScreen();
#endif

#ifdef PLATFORM_DOS
    EGA_BeginLatchedCopy();
#endif
    for(y = 0; y < map.height; y++) {
        for(x = 0; x < map.width; x++) {
#ifdef PLATFORM_DOS
            EGA_DrawTileFast(x, y, Map_TileAt(&map, x, y)->t * 128);
#else
            Raylib_DrawTile(x, y, assets.raylib_tiles, Map_TileAt(&map, x, y)->t);
#endif
        }
    }
#ifdef PLATFORM_DOS
//        memcpy(dst, src, numTiles * 32);
//        src = MK_FP(0xA400, 128);
        /*for(row = 0; row < 16; row ++) {
            dst = MK_FP(0xA000, row * 40);
            *dst++ = *src++;
            *dst = *src++;

        }*/
#endif

#ifdef PLATFORM_DOS
    EGA_EndLatchedCopy();
#endif
    draw_string(16, 16, 15, "Hello, world");

    DrawEntity(plyr[0]);
    DrawEntity(plyr[1]);
    DrawEntity(monster);

    EGA_DrawSpriteSlow(mouse_x - 8, mouse_y - 8, &assets.sprites[SPRITE_STATE_CURSOR0]);

    EGA_ShowPage(page);
    EGA_WaitVerticalRetrace();
}

bool LoadFile(const char *path, void *dest, int sz) {
    size_t res;

    FILE *f = fopen(path, "rb");

    if(!path) {
        exit(0);
        return false;
    }

    res = fread(dest, sz, 1, f);
    if(res != 1) {
        exit(0);
    }
//    printf("LoadFile: Loaded %s\n", path);
    return true;
}

#ifdef PLATFORM_DOS
void EGA_LoadTilesToVRAM(ega_tile_t *tiles, int numTiles, int vramStartIndex) {
    // Load numTiles into VRAM starting at the nth index (which is n * 32 bytes - accounting for planes);
    // Tile size (per plane) is 16x16
    // 16x16 1 plane is 256 bits = 32 bytes

    int plane, j = 0;
    unsigned char far *dst;
    unsigned char *src;
    for(plane = 0; plane < 4; plane ++) {
        EGA_SetPlanes(1 << plane);
        src = (unsigned char *) tiles;
        dst = MK_FP(0xA400, vramStartIndex * 32);
        for(j = 0; j < 32 * numTiles; j++) {
            *dst++ = src[plane * 32 + j];
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    neo_config_t config = {
            "NEO Engine",
            Update,
            Draw
    };

    (void) argc;
    (void) argv;

    atexit(shutdown);
	Neo_Init(config);

    assets.sprites[SPRITE_STATE_PLAYER_STAND0] = EGA_LoadSprite("player");
    assets.sprites[SPRITE_STATE_PLAYER_MOVE0] = EGA_LoadSprite("player2");
    assets.sprites[SPRITE_STATE_PLAYER_MOVE1] = EGA_LoadSprite("player3");
    assets.sprites[SPRITE_STATE_PLAYER_ATTACK0] = EGA_LoadSprite("player4");
    assets.sprites[SPRITE_STATE_PLAYER_ATTACK1] = EGA_LoadSprite("player5");
    assets.sprites[SPRITE_STATE_PLAYER_ATTACK2] = EGA_LoadSprite("player6");
    assets.sprites[SPRITE_STATE_MONSTER_STAND0] = EGA_LoadSprite("monster");
    assets.sprites[SPRITE_STATE_CURSOR0] = EGA_LoadSprite("cursor");
#ifdef PLATFORM_DOS
    LoadFile("data/tile.ega", &assets.tiles, 8192);
    LoadFile("data/font.ega", assets.font_data, 3072);
#else
    assets.raylib_font = LoadImage("dev/font.png");
    assets.raylib_tiles = LoadImage("dev/tile.png");

    if(!IsImageValid(assets.raylib_tiles)) {
        return 0;
    }
#endif

    Line_InitPool();
    RegisterEntities();
    Entity_Init();
    Map_Alloc(20, 12, &map);

    // Spawn Player and Monster

    plyr[0] = Entity_Alloc(ET_PLAYER);
    plyr[0]->origin.x = 200;
    plyr[0]->origin.y = 100;
    Entity_SetState(plyr[0], STATE_PLAYER_STAND0);

    plyr[1] = Entity_Alloc(ET_PLAYER);
    plyr[1]->origin.x = 200;
    plyr[1]->origin.y = 150;
    Entity_SetState(plyr[1], STATE_PLAYER_STAND0);

    monster = Entity_Alloc(ET_MONSTER);
    monster->origin.x = 280;
    monster->origin.y = 100;
    Entity_SetState(monster, STATE_MONSTER_STAND0);
    // Copy default tiles
    EGA_Init();


#ifdef PLATFORM_DOS
    EGA_LoadTilesToVRAM(assets.tiles, 64, 0);
#endif
    Neo_Timer_Init();
    Neo_Keyboard_Init();
    Neo_Run();

	return 0;
}
