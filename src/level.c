#include "common.h"

TMap *currentMap = NULL;

const u16 mapData[240] = {
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
        2, 0, 0, 5, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 2,
        2, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
        2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 5, 2,
        2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 6, 0,
        2, 0, 0, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 0, 2,
        2, 0, 8, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 8, 0, 0, 0, 0, 0, 2,
        2, 0, 0, 0, 0, 0, 0, 0, 0, 5, 2, 0, 0, 0, 0, 4, 0, 0, 0, 2,
        2, 0, 0, 0, 3, 7, 0, 0, 0, 0, 2, 4, 0, 0, 0, 0, 0, 0, 0, 2,
        2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10
};

bool Map_Alloc(u16 width, u16 height, TMap* map) {
    int i;

    map->width = width;
    map->height = height;
    map->tiles = (TTile*)malloc(sizeof(TTile) * width * height);

    if (map->tiles == NULL) {
        return false;
    }

    memset(map->tiles, 0, sizeof(TTile) * width * height);
    currentMap = map;

    for (i = 0; i < (map->width * map->height); i++) {
        map->tiles[i].t = mapData[i];
        map->tiles[i].flags = 0;
        if (mapData[i] == 2) {
            map->tiles[i].flags = 1;
        }
    }

    return true;
}

TTile *Map_TileAt(TMap* map, u16 x, u16 y) {
    return &map->tiles[y * map->width + x];
}
