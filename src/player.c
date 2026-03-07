#include "common.h"

#define MAX_OPEN 128   /* tune to your RAM budget */

typedef struct { int x, y; } Vec2;

static int heuristic(Vec2 a, Vec2 b) {
    int dx, dy;

    dx = a.x - b.x;
    dy = a.y - b.y;

    int v = (sqrtf((float) dx * (float)dx + (float)dy * (float)dy) * 100.0f);
    // TraceLog(LOG_INFO, "%d %d -> %d %d = %d", a.x, a.y, b.x, b.y, v);
    return v;
    // return dx + dy;
}

static const Vec2 dirs[9] = {
    {-1,-1},{0,-1},{1,-1},
    {-1,0},{0,0},{1,0},
    {-1,1},{0,1},{1,1},
};

/* Returns path length, or -1 if no path. Fills `path` back-to-front. */
int greedy_path(const TMap *map, Vec2 start, Vec2 goal,
                Vec2 *path, int path_max)
{
    TTile *tile;

    static bool  open[4096];
    static Vec2  parent[4096];

    static int   fScore[4096];
    static int   g_cost[4096];

    for (int i = 0; i < map->width * map->height; i++)
    {
        open[i] = false;
        fScore[i] = 0x7fff;
        g_cost[i] = 0x7fff;
    }

    open[start.y * map->width + start.x] = true;

    // parent[start.y * map->width + start.x] = start;

    g_cost[start.y * map->width + start.x] = 0;
    fScore[start.y * map->width + start.x] = heuristic(start, goal);

    int open_n = 1;
    int i, d;
    Vec2 cur, nb;

    while (open_n > 0) {
        printf("%d\n", open_n);

        int best = -1;
        // Find current open node with lowest fScore
        int f = 0x7fff;
        for (i = 0; i < 4096; i++)
        {
            if (open[i] && fScore[i] < f) {
                best = i;
                f = fScore[i];
            }
        }

        open_n --;
        cur.x = best % map->width;
        cur.y = best / map->width;

        open[cur.y * map->width + cur.x] = false;

        printf("cur: %d %d, goal: %d %d\n", cur.x, cur.y, goal.x, goal.y);
        if (cur.x == goal.x && cur.y == goal.y) {
            /* Reconstruct path */
            int len = 0;
            Vec2 n = goal;
            while ((n.x != start.x || n.y != start.y) && len < path_max) {
                path[(path_max - 1) - (len++)] = n;
                n = parent[n.y * map->width + n.x];
            }

            printf("Path length: %d\n", len);
            return len; /* path is stored goal→start; reverse if needed */
        }

        for (d = 0; d < 9; d++) {
            nb.x = cur.x + dirs[d].x;
            nb.y = cur.y + dirs[d].y;
            if (nb.x < 0 || nb.x >= map->width || nb.y < 0 || nb.y >= map->height) continue;
            tile = Map_TileAt(map, nb.x, nb.y);
            if (tile == NULL) continue;
            if (tile->flags & TILEFLAG_SOLID) continue;

            int tentative_g = g_cost[cur.y * map->width + cur.x] + heuristic(cur, nb);

            /* Skip if we already have a better route to this neighbour */
            // if (visited[nb.y * map->width + nb.x] && tentative_g >= g_cost[nb.y * map->width + nb.x]) continue; /* <-- NEW */
            if (tentative_g < g_cost[nb.y * map->width + nb.x]) {
                parent[nb.y * map->width + nb.x] = cur;
                g_cost[nb.y * map->width + nb.x] = tentative_g;
                fScore[nb.y * map->width + nb.x] = tentative_g + heuristic(nb, goal);
                if (!open[nb.y * map->width + nb.x]) {
                    open[nb.y * map->width + nb.x] = true;
                    open_n++;
                }
            }
        }
    }

    printf("no path");
    return -1;
}




void UpdateMoveLine(TEntity *self) {
    TVec2 dest;
    TEntity *e;
    TLine *line;

    // Get target location based on action
    switch (self->action) {
        case 1:
            // Action 1: Move to a specific position
            dest.x = self->targetPos.x;
            dest.y = self->targetPos.y;
            break;

        case 2:
            // Action 2: Move towards another entity
            e = EntityForID(self->targetID);
            if (e == NULL) {
//                printf("no entity for id %d %d\n", self->targetID, self->targetID >> 6);
                self->action = 0; // No target entity found, reset action
                return;
            } else {
                dest = e->origin; // Target entity's position
            }
            break;

        default:
            // Invalid or unknown action
            return;
    }

    if (self->action == 0) {
        return; // Exit since action is reset
    }

    // Mark the entity as moving
    self->isMoving = true;
    self->moveCount = 0;

    // Free the existing movement line
    Line_Free(self->moveLine);

    // Allocate a new movement line
    self->moveLine = Line_Alloc();
    if (self->moveLine != 0) {
        line = Line_Get(self->moveLine);
        // Initialize the movement line
        Line_Init(line,self->origin.x, self->origin.y,dest.x, dest.y);

        // Update entity state to indicate movement
        Entity_SetState(self, STATE_PLAYER_MOVE0);
    }
}

static void OnMoveComplete(TEntity *self, TMoveInfo move) {
    Entity_SetState(self, STATE_PLAYER_STAND0);
    if(self->targetID != 0 && move.hitEntity == self->targetID) {
//        printf("Player moved into target\n");
        Entity_SetState(self, STATE_PLAYER_ATTACK0);
    }
}

static void Player_Update(TEntity *self) {
    Entity_StandardMove(self, 256, OnMoveComplete);
}

static void Player_StateChange(TEntity *self) {
    (void)self;
    if(self->state->state == STATE_PLAYER_ATTACK2) {
//        printf("Damage target\n");

        if(self->targetID != 0) {
            TEntity *target = EntityForID(self->targetID);
            if(target != NULL) {
                target->flash_time = 8;
//                target->health -= 1;
            }
        }
    }
}

void Player_SetTarget(TEntity *self, u16 x, u16 y) {
    TEntity *e, *targetEntity = NULL;
    bounds_t bounds;
    int i, j, tx0, ty0, tx1, ty1, np;

    for(i = 1; i < MAX_ENT; i++) {
        e = EntityForIndex(i);
        if (e && e != self&& e->classID == ET_MONSTER) {
            EntityBounds(e, &bounds);

            if(BoundsContainsXY(bounds, x, y)) {
                targetEntity = e;
                break;
            }
        }
    }

    if(targetEntity != NULL) {
        self->targetID = targetEntity->id;
//        printf("Target ID: %d\n", self->targetID);
        self->action = 2;
    } else {

#ifndef PLATFORM_DOS

        TraceLog(LOG_INFO, "move to point");

#endif
        // printf("move to point...\n");
        self->targetID = 0;
        self->action = 1;
        self->targetPos.x = x;
        self->targetPos.y = y;
    }

    UpdateMoveLine(self);
}


void DrawMoveDebug(TEntity *e)
{
#ifdef PLATFORM_DESKTOP
    Image img = EGA_Raylib_GetBackBuffer();
    Vec2 path[128];

    if(e->isMoving) {
        TVec2 targetPos = e->targetPos;
        if (e->targetID) {
            TEntity *t = EntityForID(e->targetID);

            if (t) {
                targetPos = t->origin;
                         bounds_t otherBounds;
                EntityBounds(t, &otherBounds);
                TVec2 center = BoundsCenter(otherBounds);

                ImageDrawLine(&img, e->origin.x, e->origin.y, center.x, center.y, RED);
            }
        } else {
            ImageDrawLine(&img, e->origin.x, e->origin.y, e->targetPos.x, e->targetPos.y, RED);
        }

        int tx0 = e->origin.x >> 4;
        int ty0 = e->origin.y >> 4;
        int tx1 = targetPos.x >> 4;
        int ty1 = targetPos.y >> 4;
        int np = greedy_path(currentMap, (Vec2){tx0, ty0}, (Vec2){tx1, ty1}, &path[0], 128);

        if (np != -1)
        {
            for (int i = 0; i < np; i++)
            {
                int j = (128 - np + i);
                // TraceLog(LOG_INFO, "path[%d]: %d %d", i, path[j].x, path[j].y);
                int left = path[j].x * 16;
                int top = path[j].y * 16;
                ImageDrawRectangleLines(&img, (Rectangle){left, top, 16, 16}, 1, RED);
            }
        }
    }
#endif
}

void Player_Register(ent_info_t *info) {

    info->mins.x = -6;
    info->mins.y = -14;
    info->maxs.x = info->mins.x + 12;
    info->maxs.y = 0;

    info->collision = COLLISION_SOLID;

    info->frameFunc = Player_Update;
    info->stateChangeFunc = Player_StateChange;
}
