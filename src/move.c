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

            // printf("Path length: %d\n", len);
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

    // LogInfo("Generate new path for %d %d", dest.x, dest.y);

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
    self->movePhase = 1;

    // Update entity state to indicate movement
    Entity_SetState(self, STATE_PLAYER_MOVE0);
}

void PathMoveStep(TEntity* self, EntityOnMoveCompleteFunc onMoveCompleteFunc)
{
    TMoveInfo move;
    TPath *path;
    int pn, ctx, cty, tx0, ty0;
    TVec2 dest;
    bool valid, traceResult;
    TLine *line;
    LinePoint pt;
    bounds_t selfBounds, nextBounds;

    if (self->path != -1)
    {
        path = Path_Get(self->path);

        // LogInfo("Move along path... Next node: %d", self->nextPathNode);

        pn = path->p[self->nextPathNode];

        if (pn == -1)
        {
            // Are we at the end of the path?
            self->movePhase = 2;

            valid = Entity_GetTargetPos(self, &dest);
            if (!valid)
            {
                self->isMoving = false;
            } else
            {
                // Can we attempt a line-move for the rest of it?

                // Free the existing movement line
                Line_Free(self->moveLine);

                // Allocate a new movement line
                self->moveLine = Line_Alloc();
                if (self->moveLine != 0)
                {
                    line = Line_Get(self->moveLine);
                    // Initialize the movement line
                    Line_Init(line, self->origin.x, self->origin.y, dest.x, dest.y);

                    // step the line maybe?
                    Line_GetPoint(line, &pt);
                }
            }

            return;
        }

        ctx = self->origin.x >> 4;
        cty = self->origin.y >> 4;

        // Are we in the next node?
        tx0 = pn % currentMap->width;
        ty0 = pn / currentMap->width;

        nextBounds.min.x = tx0 * 16;
        nextBounds.min.y = ty0 * 16;
        nextBounds.max.x = nextBounds.min.x + 16;
        nextBounds.max.y = nextBounds.min.y + 16;

        EntityBounds(self, &selfBounds);

        // LogInfo("current tile: %d %d, target: %d %d", ctx, cty, tx0, ty0);

        if (ctx == tx0 && cty == ty0)
        {
            // LogInfo("In next");
            self->nextPathNode++;
        }
        else
        {
            // moving...

            TVec2 delta;

            delta.x = tx0 - ctx;
            delta.y = ty0 - cty;

            move.collisionMask = COLLISION_SOLID;
            move.ignoreEntity = self->id;
            move.ignoreEntity2 = 0;
            traceResult = WorldTrace(self->origin, delta, self->mins, self->maxs, &move);

            if (move.hitType == HIT_TYPE_ENTITY && move.hitEntity == self->targetID)
            {
                LogInfo("Hit the target entity!");

                self->isMoving = false;

                if (onMoveCompleteFunc)
                {
                    onMoveCompleteFunc(self, move);
                }

                return;
            }

            // LogInfo("result delta: %d %d", move.resultDelta.x, move.resultDelta.y);

            if (move.resultDelta.x == 0 && move.resultDelta.y == 0)
            {
                // LogInfo("move: hit something, delta: %d %d, cur: %d %d, result: %d %d\n", delta.x, delta.y,
                        // self->origin.x, self->origin.y, move.result.x, move.result.y);

                EntityBounds(self, &selfBounds);

                // wasn't able to move, so brute-force it
                // if bottom of next tile is below bounds, then try to  move down

                delta.x = 0;
                delta.y = 0;

                if (nextBounds.min.x < selfBounds.min.x)
                {
                    delta.x = -1;
                }

                if (nextBounds.min.x > selfBounds.min.x)
                {
                    delta.x = 1;
                }

                if (nextBounds.max.y > selfBounds.max.y)
                {
                    delta.y = 1;
                }

                traceResult = WorldTrace(self->origin, delta, self->mins, self->maxs, &move);

                if (move.resultDelta.x == 0 && move.resultDelta.y == 0)
                {
                    LogInfo("stuck?");
                }
                else
                {
                    // LogInfo("brute force from %d %d -> %d %d", self->origin.x, self->origin.y, move.result.x, move.result.y);
                    self->origin.x = move.result.x;
                    self->origin.y = move.result.y;
                }
            }
            else
            {
                self->origin.x = move.result.x;
                self->origin.y = move.result.y;
            }

            if (!traceResult)
            {
                // self->origin.x += delta.x;
                // self->origin.y += delta.y;
            }
            else
            {
                // self->isMoving = false;
            }
        }
    }
}

void Entity_MoveLine(TEntity* e, int lineNum, TMoveInfo* move)
{
    LinePoint pt;
    TVec2 delta;
    bool traceResult;
    TLine* line = Line_Get(lineNum);

    if (!line)
    {
        printf("Entity_MoveLine: line not found\n");
        return;
    }

    Line_GetPoint(line, &pt);

    delta.x = pt.x - e->origin.x;
    delta.y = pt.y - e->origin.y;

    traceResult = WorldTrace(e->origin, delta, e->mins, e->maxs, move);

    if (!traceResult)
    {
        e->origin.x = pt.x;
        e->origin.y = pt.y;
    }
    else
    {
        printf("moveLine: hit something\n");
        e->isMoving = false;
    }
}

void LineMoveStep(TEntity* self, EntityOnMoveCompleteFunc onMoveComplete)
{
    TMoveInfo move;
    TLine *line;

    if (self->moveLine != 0) {
        move.collisionMask = COLLISION_SOLID;
        move.ignoreEntity = self->id;
        move.ignoreEntity2 = 0;
        Entity_MoveLine(self, self->moveLine, &move);

        line = Line_Get(self->moveLine);
        if (line->done || move.hitType != 0) {
            // Free line resource equivalent
            printf("line move done, hittype: %d\n", move.hitType);
            self->isMoving = false;

            if (onMoveComplete) {
                onMoveComplete(self, move);
            }
        }
    }
}

void Entity_StandardMove(TEntity* self, int speed, EntityOnMoveCompleteFunc onMoveComplete)
{
    int bd;
    // We should ensure the target entity exists here so we don't have to check during the rest of the move logic

    // If we're targeting an entity, check to see if it has moved out of range, and start moving towards it again

    if (self->action == 2 && !self->isMoving)
    {
        TEntity *other = EntityForID(self->targetID);
        if (other)
        {
            bounds_t selfBounds, otherBounds;
            EntityBounds(self, &selfBounds);
            EntityBounds(other, &otherBounds);

            bd = BoundsDistance(selfBounds, otherBounds);

            if (bd >= 8)
            {
                LogInfo("Check distance to see if we should start moving again: %d", bd);
                // LogInfo("Retarget...")
                // Re-target the enemy
                UpdateMoveLine(self);
            }

        } else
        {
            //
        }
    }
    if (!self->isMoving)
    {
        return;
    }

    if (self->action == 2)
    {
        // TODO: Path should only update if the target moved by more than some small amount

        // Could also check if the target point moved
        if (++self->pathUpdateTime > 12)
        {
            self->pathUpdateTime = 0;

            UpdateMoveLine(self);

            if (self->action == 0)
            {
                // TODO: Check if we're still targeting the entity (it may no longer exist)
                LogInfo("Couldn't update path");
            }
        }
    }

    self->moveCount += speed;

    while (self->moveCount >= 256)
    {
        self->moveCount -= 256;
        switch (self->movePhase)
        {
        case 0:
            // No move phase
            break;
        case 1:
            PathMoveStep(self, onMoveComplete);
            if (self->isMoving && self->movePhase == 2)
            {
                LineMoveStep(self, onMoveComplete);
            }
            break;
        case 2:
            LineMoveStep(self, onMoveComplete);
            break;
        default:
            break;
        }



    }
}
