#include "common.h"

#define MAX_OPEN 256   /* tune to your RAM budget */

typedef struct
{
    uint16_t index; // Node/tile index
    uint16_t f; // f = g + h (use uint16_t if costs fit)
} HeapNode;

typedef struct
{
    HeapNode data[MAX_OPEN];
    uint8_t in_open[MAX_OPEN];

    uint16_t size;
} MinHeap;

void heap_init(MinHeap* h)
{
    h->size = 0;
    memset(h->in_open, 0, sizeof(h->in_open));
}

static void swap(HeapNode* a, HeapNode* b)
{
    HeapNode tmp = *a;
    *a = *b;
    *b = tmp;
}

void heap_push(MinHeap* h, uint16_t index, uint16_t f)
{
    uint8_t i, parent;

    if (h->size >= MAX_OPEN)
        return; // Handle overflow!

    h->in_open[index] = true;

    i = h->size++;

    h->data[i].index = index;
    h->data[i].f = f;

    // Sift up
    while (i > 0)
    {
        parent = (i - 1) / 2;
        if (h->data[parent].f <= h->data[i].f) break;
        swap(&h->data[parent], &h->data[i]);
        i = parent;
    }
}

// Returns the node with the lowest f, removes it from the heap
HeapNode heap_pop(MinHeap* h)
{
    uint8_t i = 0;
    HeapNode top = h->data[0];

    h->in_open[top.index] = false;

    h->data[0] = h->data[--h->size];

    // Sift down
    while (1)
    {
        uint8_t left = 2 * i + 1;
        uint8_t right = 2 * i + 2;
        uint8_t smallest = i;

        if (left < h->size && h->data[left].f < h->data[smallest].f) smallest = left;
        if (right < h->size && h->data[right].f < h->data[smallest].f) smallest = right;
        if (smallest == i) break;

        swap(&h->data[i], &h->data[smallest]);
        i = smallest;
    }

    return top;
}

// Update f for a node already in the heap (re-heapify upward)
void heap_update(MinHeap* h, uint16_t index, uint16_t new_f)
{
    uint8_t i;
    for (i = 0; i < h->size; i++)
    {
        if (h->data[i].index != index) continue;
        h->data[i].f = new_f;
        // Sift up (f only ever decreases on update in A*)
        while (i > 0)
        {
            uint8_t parent = (i - 1) / 2;
            if (h->data[parent].f <= h->data[i].f) break;
            swap(&h->data[parent], &h->data[i]);
            i = parent;
        }
        return;
    }
}

typedef struct
{
    int16_t x, y;
} Vec2;

int isqrt(int n) {
    int x, x1;
    if (n < 0) return -1;
    if (n == 0) return 0;
    x = n;
    x1 = (x + 1) >> 1;
    while (x1 < x) {
        x = x1;
        x1 = (x + n / x) >> 1;
    }
    return x;
}

static int heuristic(Vec2 a, Vec2 b)
{
    int dx, dy, v;

    dx = a.x - b.x;
    dy = a.y - b.y;

    v = isqrt(dx * dx + dy * dy);
    // v = (sqrtf((float)dx * (float)dx + (float)dy * (float)dy) * 100.0f);
    // TraceLog(LOG_INFO, "%d %d -> %d %d = %d", a.x, a.y, b.x, b.y, v);
    return v;
    // return dx + dy;
}

static const Vec2 dirs[9] = {
    {-1, -1},
    {0, -1},
    {1, -1},
    {-1, 0},
    {0, 0},
    {1, 0},
    {-1, 1},
    {0, 1},
    {1, 1},
};

int16_t greedy_path(TMap* map, Vec2 start, Vec2 goal, Vec2* path, int path_max)
{
    int i, d, nb_idx, tentative_g, f;
    Vec2 cur, nb;
    static Vec2 parent[4096];
    static int fScore[4096];
    static int g_cost[4096];
    TTile* tile;

    int start_idx = start.y * map->width + start.x;
    MinHeap openHeap;
    heap_init(&openHeap);

    for (i = 0; i < map->width * map->height; i++)
    {
        fScore[i] = 0x7fff;
        g_cost[i] = 0x7fff;
    }


    // parent[start.y * map->width + start.x] = start;

    g_cost[start_idx] = 0;
    fScore[start_idx] = heuristic(start, goal);

    heap_push(&openHeap, start_idx, 0);

    while (openHeap.size > 0)
    {
        HeapNode top = heap_pop(&openHeap);

        cur.x = top.index % map->width;
        cur.y = top.index / map->width;

        // printf("cur: %d %d, goal: %d %d\n", cur.x, cur.y, goal.x, goal.y);
        if (cur.x == goal.x && cur.y == goal.y)
        {
            /* Reconstruct path */
            int len = 0;
            Vec2 n = goal;
            while ((n.x != start.x || n.y != start.y) && len < path_max)
            {
                path[(path_max - 1) - (len++)] = n;
                n = parent[n.y * map->width + n.x];
            }

            printf("Path length: %d\n", len);
            return len; /* path is stored goal→start; reverse if needed */
        }

        for (d = 1; d < 9; d+=2)
        // for (d = 0; d < 9; d++)
        {
            nb.x = cur.x + dirs[d].x;
            nb.y = cur.y + dirs[d].y;

            nb_idx = nb.y * map->width + nb.x;

            if (nb.x < 0 || nb.x >= map->width || nb.y < 0 || nb.y >= map->height) continue;
            tile = Map_TileAt(map, nb.x, nb.y);
            if (tile == NULL) continue;
            if (tile->flags & TILEFLAG_SOLID) continue;

            tentative_g = g_cost[cur.y * map->width + cur.x] + heuristic(cur, nb);

            /* Skip if we already have a better route to this neighbour */
            // if (visited[nb.y * map->width + nb.x] && tentative_g >= g_cost[nb.y * map->width + nb.x]) continue; /* <-- NEW */
            if (tentative_g < g_cost[nb.y * map->width + nb.x])
            {
                parent[nb.y * map->width + nb.x] = cur;
                g_cost[nb.y * map->width + nb.x] = tentative_g;
                f = tentative_g + heuristic(nb, goal);
                fScore[nb_idx] = f;

                heap_update(&openHeap, nb_idx, f);

                if (!openHeap.in_open[nb_idx])
                {
                    heap_push(&openHeap, nb_idx, f);
                }
            }
        }
    }

    printf("no path");
    return -1;
}

void UpdateMoveLine(TEntity* self)
{
    TVec2 dest;
    int16_t np, i, j;
    Vec2 path[128], start, end;
    TPath *p;

    bool valid = Entity_GetTargetPos(self, &dest);

    if (!valid)
    {
        self->action = 0;
        return; // Exit since action is reset
    }

    LogInfo("Generate new path for %d %d", dest.x, dest.y);

    start.x = self->origin.x >> 4;
    start.y = self->origin.y >> 4;
    end.x = dest.x >> 4;
    end.y = dest.y >> 4;

    np = greedy_path(currentMap, start, end, &path[0], 128);

    if (np != -1)
    {
        int8_t pathNum = Path_Alloc();
        if (pathNum == -1)
        {
            LogInfo("couldn't allocate path!");
            exit(0);
        }

        if (self->path != -1)
        {
            Path_Free(self->path);
        }

        self->path = pathNum;
        self->nextPathNode = 0;

       p = Path_Get(pathNum);

        for (i = 0; i < np; i++)
        {
            j = (128 - np + i);
            // TraceLog(LOG_INFO, "path[%d]: %d %d", i, path[j].x, path[j].y);
            p->p[i] = path[j].y * currentMap->width + path[j].x;
        }
    }

    // Mark the entity as moving
    self->isMoving = true;
    self->moveCount = 0;

    // Update entity state to indicate movement
    Entity_SetState(self, STATE_PLAYER_MOVE0);
}

static void OnMoveComplete(TEntity* self, TMoveInfo move)
{
    Entity_SetState(self, STATE_PLAYER_STAND0);
    if (self->targetID != 0 && move.hitEntity == self->targetID)
    {
        //        printf("Player moved into target\n");
        Entity_SetState(self, STATE_PLAYER_ATTACK0);
    }
}

static void Player_Update(TEntity* self)
{
    Entity_StandardMove(self, 256, OnMoveComplete);
}

static void Player_StateChange(TEntity* self)
{
    (void)self;
    if (self->state->state == STATE_PLAYER_ATTACK2)
    {
        //        printf("Damage target\n");

        if (self->targetID != 0)
        {
            TEntity* target = EntityForID(self->targetID);
            if (target != NULL)
            {
                target->flash_time = 8;
                //                target->health -= 1;
            }
        }
    }
}

void Player_SetTarget(TEntity* self, u16 x, u16 y)
{
    TEntity *e, *targetEntity = NULL;
    bounds_t bounds;
    int i, j, tx0, ty0, tx1, ty1, np;

    for (i = 1; i < MAX_ENT; i++)
    {
        e = EntityForIndex(i);
        if (e && e != self && e->classID == ET_MONSTER)
        {
            EntityBounds(e, &bounds);

            if (BoundsContainsXY(bounds, x, y))
            {
                targetEntity = e;
                break;
            }
        }
    }

    if (targetEntity != NULL)
    {
        self->targetID = targetEntity->id;
        //        printf("Target ID: %d\n", self->targetID);
        self->action = 2;
    }
    else
    {
#ifndef PLATFORM_DOS

        TraceLog(LOG_INFO, "move to point");

#endif
        // printf("move to point...\n");
        self->targetID = 0;
        self->action = 1;
        self->targetPos.x = x;
        self->targetPos.y = y;
    }

    self->movePhase = 1;

    UpdateMoveLine(self);
}


void DrawMoveDebug(TEntity* e)
{
#ifdef PLATFORM_DESKTOP
    Image img = EGA_Raylib_GetBackBuffer();

    if (e->isMoving)
    {
        TVec2 targetPos = e->targetPos;
        if (e->targetID)
        {
            TEntity* t = EntityForID(e->targetID);

            if (t)
            {
                targetPos = t->origin;
                bounds_t otherBounds;
                EntityBounds(t, &otherBounds);
                TVec2 center = BoundsCenter(otherBounds);

                ImageDrawLine(&img, e->origin.x, e->origin.y, center.x, center.y, RED);
            }
        }
        else
        {
            ImageDrawLine(&img, e->origin.x, e->origin.y, e->targetPos.x, e->targetPos.y, RED);
        }

        if (e->path != -1)
        {
            TPath* path = Path_Get(e->path);

            for (int i = 0; i < 128; i++)
            {
                if (path->p[i] == -1) break;
                int idx = path->p[i];
                int px = idx % currentMap->width;
                int py = idx / currentMap->width;
                int left = px * 16;
                int top = py * 16;
                ImageDrawRectangleLines(&img, (Rectangle){left, top, 16, 16}, 1, RED);
            }
        }
    }
#endif
}

void Player_Register(ent_info_t* info)
{
    info->mins.x = -6;
    info->mins.y = -14;
    info->maxs.x = info->mins.x + 12;
    info->maxs.y = 0;

    info->collision = COLLISION_SOLID;

    info->frameFunc = Player_Update;
    info->stateChangeFunc = Player_StateChange;
}
