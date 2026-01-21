#include "common.h"

#define MAX_NEAR_COLLIDE 32

typedef struct near_collide_s {
    bounds_t bounds;

    // If NULL, then we're referring to a tile
    TEntity *e;

    int tile;
} near_collide_t;

static int nearCollideCount = 0;
static near_collide_t near_collide[MAX_NEAR_COLLIDE];

static bounds_t moveBounds;

static void CollectEntities(TMoveInfo *trace) {
    TEntity *e;
    TTile *t;
    bounds_t bounds, tileBounds;
    int i, tx0, ty0, tx1, ty1, tx, ty;
    bool check;
    nearCollideCount = 0;

    for (i = 1; i < MAX_ENT; i++) {
        e = EntityForIndex(i);
        if (!e->state
            || e->id == trace->ignoreEntity
            || e->id == trace->ignoreEntity2
            || (e->collision & trace->collisionMask) == 0) continue;

        EntityBounds(e, &bounds);
        bounds.max.x += 1;
        bounds.max.y += 1;

        if (BoundsIntersectsBounds(moveBounds, bounds)) {
            near_collide[nearCollideCount].bounds = bounds;
            near_collide[nearCollideCount].e = e;
            nearCollideCount++;
        }
    }

    tx0 = moveBounds.min.x >> 4;
    tx1 = moveBounds.max.x >> 4;
    ty0 = moveBounds.min.y >> 4;
    ty1 = moveBounds.max.y >> 4;

    for (ty = ty0; ty <= ty1; ty++) {
        for (tx = tx0; tx <= tx1; tx++) {
            t = Map_TileAt(currentMap, tx, ty);

            // TODO, Solid tiles, with collision 0x0001

            check = false;

            if(t->flags & TILEFLAG_SOLID) {
                if(trace->collisionMask & COLLISION_SOLID) {
                    check = true;
                }
            }

//            if(t & TILEFLAG_HIDDEN) {
//                if(trace->collisionMask & COLLISION_HIDDEN_TILE) {
//                    check = true;
//                }
//            }

            if(!check) {
                continue;
            }

            tileBounds.min.x = tx * 16;
            tileBounds.min.y = ty * 16;
            tileBounds.max.x = tx * 16 + 16;
            tileBounds.max.y = ty * 16 + 16;

            near_collide[nearCollideCount].bounds = tileBounds;
            near_collide[nearCollideCount].e = NULL;
            nearCollideCount++;
        }
    }
}

bool WorldTrace(TVec2 origin, TVec2 delta, TVec2 mins, TVec2 maxs, TMoveInfo *trace) {

    int i;
    near_collide_t *nc;
    bounds_t bounds, startBounds, endBounds;

    trace->hitType = 0; // 0 = none, 1 = tile, 2 = entity
    trace->hitEntity = 0;
    trace->hitTile = false;

    maxs.x += 1;
    maxs.y += 1;

    startBounds = BoundsFromSize(origin, mins, maxs);
    endBounds = OffsetBounds(startBounds, delta);

    // TraceLog(LOG_INFO, "endBounds.max.y: %d.%d", fix32ToInt(endBounds.max.y), endBounds.max.y & FIX32_FRAC_MASK);

    moveBounds = UnionBounds(startBounds, endBounds);

    // Find relevant entities inside bounds

    CollectEntities(trace);

    endBounds = startBounds;

    if (delta.x != 0) {
        endBounds.min.x += delta.x;
        endBounds.max.x += delta.x;

        for (i = 0; i < nearCollideCount; i++) {
            nc = &near_collide[i];

            bounds = nc->bounds;

            if (!BoundsIntersectsBounds(endBounds, bounds)) {
                continue;
            }

            if (delta.x < 0) {
                if (endBounds.min.x < bounds.max.x) {
                    int d = bounds.max.x - endBounds.min.x;
                    endBounds.min.x += d;
                    endBounds.max.x += d;

                    if (nc->e) {
                        trace->hitType = HIT_TYPE_ENTITY;
                        trace->hitEntity = nc->e->id;
                    } else {
                        trace->hitType = HIT_TYPE_MAP;
                    }
                }
            }

            if (delta.x > 0) {
                if (endBounds.max.x >= bounds.min.x) {
                    int d = endBounds.max.x - bounds.min.x;
                    endBounds.min.x -= d;
                    endBounds.max.x -= d;

                    if (nc->e) {
                        trace->hitType = HIT_TYPE_ENTITY;
                        trace->hitEntity = nc->e->id;
                    } else {
                        trace->hitType = HIT_TYPE_MAP;
                    }
                }
            }
        }
    }

    if (delta.y != 0) {
        endBounds.min.y += delta.y;
        endBounds.max.y += delta.y;
        for (i = 0; i < nearCollideCount; i++) {
            nc = &near_collide[i];
            bounds = near_collide[i].bounds;

            if (!BoundsIntersectsBounds(endBounds, bounds)) {
                continue;
            }

            if (delta.y < 0) {
                if (endBounds.min.y < bounds.max.y) {
                    int d = bounds.max.y - endBounds.min.y;
                    endBounds.min.y += d;
                    endBounds.max.y += d;

                    if (nc->e) {
                        trace->hitType = HIT_TYPE_ENTITY;
                        trace->hitEntity = nc->e->id;
                    } else {
                        trace->hitType = HIT_TYPE_MAP;
                    }
                }
            }

            if (delta.y > 0) {
                if (endBounds.max.y > bounds.min.y) {
                    int d = endBounds.max.y - (bounds.min.y);
                    endBounds.min.y -= d;
                    endBounds.max.y -= d;

                    if (nc->e) {
                        trace->hitType = HIT_TYPE_ENTITY;
                        trace->hitEntity = nc->e->id;
                    } else {
                        trace->hitType = HIT_TYPE_MAP;
                    }
                }
            }
        }
    }

    trace->result.x += origin.x + (endBounds.min.x - startBounds.min.x);
    trace->result.y += origin.y + (endBounds.min.y - startBounds.min.y);

    return trace->hitType != 0;
}
