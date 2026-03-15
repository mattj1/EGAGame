#include "common.h"

#define MAX_LINES 64
#define MAX_PATHS 32

static struct
{
    // int8_t firstActivePath;
    int8_t firstFreePath;
    TPath *paths;
} state;

static TLine moveLines[MAX_LINES];
static int lineUsed[MAX_LINES];

int8_t Path_Alloc(void)
{
    int8_t idx;
    int i;
    TPath *p;

    if (state.firstFreePath == -1)
    {
        return -1;
    }

    idx = state.firstFreePath;
    p = &state.paths[idx];

    for (i = 0; i < MAX_PATH_TILES; i++)
    {
        p->p[i] = -1;
    }

    state.firstFreePath = p->next;

    // p->next = state.firstActivePath;
    // state.firstActivePath = idx;
    return idx;
}

TPath *Path_Get(int8_t idx)
{
    if (idx >= 0 && idx < MAX_PATHS)
    {
        return &state.paths[idx];
    }

    return NULL;
}

void Path_Free(int8_t idx)
{
    if (idx >= 0 && idx < MAX_PATHS)
    {
        state.paths[idx].next = state.firstFreePath;
        state.firstFreePath = idx;
    }
}

bool BoundsContainsXY(bounds_t a, int x, int y) {
    if ((x > a.max.x) || (x < a.min.x) || (y > a.max.y) || (y < a.min.y))
        return false;
    return true;
}

bool BoundsIntersectsBounds(bounds_t b0, bounds_t b1) {
    if ((b0.max.x <= b1.min.x) || (b0.max.y <= b1.min.y) ||
        (b0.min.x >= b1.max.x) || (b0.min.y >= b1.max.y)) {
        return 0;
    }
    return 1;
}

// Returns max separation distance
int16_t BoundsDistance(bounds_t b0, bounds_t b1)
{
    int16_t maxX = -32768;

    int16_t i = b1.min.x - b0.max.x;
    if (i > maxX)
    {
        maxX = i;
    }

    i = b0.min.x - b1.max.x;
    if (i > maxX)
    {
        maxX = i;
    }


    i = b1.min.y - b0.max.y;
    if (i > maxX)
    {
        maxX = i;
    }

    i = b0.min.y - b1.max.y;
    if (i > maxX)
    {
        maxX = i;
    }

    // if (maxX < 0)
    // {
    //     maxX = 0;
    // }

    return maxX;
}

bounds_t UnionBounds(bounds_t a, bounds_t b) {
    bounds_t result;

    result.min.x = (a.min.x < b.min.x) ? a.min.x : b.min.x;
    result.min.y = (a.min.y < b.min.y) ? a.min.y : b.min.y;
    result.max.x = (a.max.x > b.max.x) ? a.max.x : b.max.x;
    result.max.y = (a.max.y > b.max.y) ? a.max.y : b.max.y;

    return result;
}

bounds_t BoundsFromSize(TVec2 origin, TVec2 mins, TVec2 maxs) {
    bounds_t bounds;
    bounds.min.x = origin.x + mins.x;
    bounds.min.y = origin.y + mins.y;
    bounds.max.x = origin.x + maxs.x;
    bounds.max.y = origin.y + maxs.y;
    return bounds;
}

bounds_t OffsetBounds(bounds_t bounds, TVec2 offset) {
    bounds_t out;
    out.min.x = bounds.min.x + offset.x;
    out.min.y = bounds.min.y + offset.y;
    out.max.x = bounds.max.x + offset.x;
    out.max.y = bounds.max.y + offset.y;
    return out;
}

TVec2 BoundsCenter(bounds_t bounds) {
    TVec2 out;
    out.x = (bounds.min.x + bounds.max.x) / 2;
    out.y = (bounds.min.y + bounds.max.y) / 2;
    return out;
}

/* Line Pool Implementation */
void Line_InitPool(void) {
    int i;
    for (i = 0; i < MAX_LINES; i++) {
        lineUsed[i] = 0;
        memset(&moveLines[i], 0, sizeof(TLine));
    }

    LogInfo("Size of path: %d, paths: %d", sizeof(TPath), sizeof(TPath) * MAX_PATHS);
    LogInfo("Size of lines: %d", sizeof(moveLines));

    state.paths = malloc(sizeof(TPath) * MAX_PATHS);
    memset(state.paths, 0, sizeof(TPath) * MAX_PATHS);
    state.firstFreePath = 0;
    // state.firstActivePath = -1;

    for (i = 0; i < MAX_PATHS; i++)
    {
        state.paths[i].next = i < (MAX_PATHS-1) ? i + 1 : -1;

        // TraceLog(LOG_INFO, "%d: %d", i, paths[i].next);
    }
}

void Line_Init(TLine* line, int x0, int y0, int x1, int y1) {
    line->dx = abs(x1 - x0);
    line->dy = abs(y1 - y0);
    line->x = x0;
    line->y = y0;
    line->x0 = x0;
    line->y0 = y0;
    line->x1 = x1;
    line->y1 = y1;
    line->sx = -1;
    line->done = 0;
    if (x0 <= x1) line->sx = 1;
    line->sy = -1;
    if (y0 <= y1) line->sy = 1;
    line->err = 0;
    line->didReturnFirst = 0;
}

void Line_GetPoint(TLine* line, LinePoint* pt) {
    if(!line) {
        return;
    }
    if (!line->didReturnFirst) {
        pt->x = line->x0;
        pt->y = line->y0;
        line->didReturnFirst = 1;
        return;
    }

    if (line->done) {
        pt->x = line->x1;
        pt->y = line->y1;
        return;
    }

    if (line->dx > line->dy) {
        line->err = line->err - line->dy;
        if (line->err < 0) {
            line->y = line->y + line->sy;
            line->err = line->err + line->dx;
        }
        line->x = line->x + line->sx;
    } else {
        line->err = line->err - line->dx;
        if (line->err < 0) {
            line->x = line->x + line->sx;
            line->err = line->err + line->dy;
        }
        line->y = line->y + line->sy;
    }

    pt->x = line->x;
    pt->y = line->y;

    if ((pt->x == line->x1) && (pt->y == line->y1)) {
        line->done = 1;
    }
}


u16 Line_Alloc(void) {
    u16 i;
    for (i = 1; i < MAX_LINES; i++) {
        if (!lineUsed[i]) {
            lineUsed[i] = 1;
            return i;
        }
    }

    return 0; /* Failure */
}

void Line_Free(u16 index) {
    if ((index > 0) && (index < MAX_LINES)) {
        lineUsed[index] = 0;
    }
}

TLine* Line_Get(u16 index) {
    if ((index > 0) && (index < MAX_LINES)) {
        return &moveLines[index];
    }
    return NULL;
}
