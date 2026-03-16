#ifndef COMMON_H
#define COMMON_H
#include "neo.h"
#include "engine/src/ega.h"
#include "res/res.h"

typedef struct {
    u16 t;
    u16 flags;
} TTile;

typedef struct {
    u16 x, y;
    TTile* tile;
} TTileRef;

typedef struct {
    u16 width, height;
    TTile* tiles;
} TMap;

extern TMap *currentMap;

bool Map_Alloc(u16 width, u16 height, TMap* map);
TTile *Map_TileAt(TMap* map, u16 x, u16 y);

#define MAX_ENT 64
#define TILEFLAG_SOLID 1
#define ENTFLAG_ACTIVE      0x01
#define ENTFLAG_VISIBLE     0x02
#define ENTFLAG_HAS_TOOLTIP 0x04


#define COLLISION_SOLID        0x01    // Blocks only if targeted (default for most things)
#define COLLISION_ALWAYS_SOLID 0x02    // Blocks all the time

#define STATE_NONE 0

typedef struct {
    int16_t x, y;
} TVec2;

struct TEntity_s;
struct ent_info_s;

typedef void (*EntityInitFunc)(struct TEntity_s *data);
typedef void (*EntityFrameFunc)(struct TEntity_s *data);
typedef void (*EntityStateChangeFunc)(struct TEntity_s *data);
typedef void (*EntityTooltipFunc)(struct TEntity_s *data);
typedef void (*EntityRegisterFunc)(struct ent_info_s *info);
typedef void (*EntityDamageFunc)(struct TEntity_s *self, int amount);

typedef struct ent_info_s {
    const char *className;
    TVec2 mins, maxs;
    u16 collision;

    EntityInitFunc initFunc;
    EntityFrameFunc frameFunc;
    EntityStateChangeFunc stateChangeFunc;
    EntityTooltipFunc tooltipFunc;
    EntityDamageFunc damageFunc;
} ent_info_t;

typedef struct {
    u16 state;
    u16 nextState;
    u16 numFrames;
    u16 spriteState;

    // Callbacks...
} entity_state_t;

typedef struct TEntity_s {
    TVec2 origin;
    u16 id;
    u16 flags;
    TVec2 mins;
    TVec2 maxs;
    u16 ownerID;
    u16 classID;
    u16 stateSpeed;
    u16 moveCount;
    bool isMoving;
    u16 nudge_time;
    u16 flash_time;
    u16 state;
    u16 stateTime;
    ent_info_t* info;
    u16 collision;

    u16 action;
    u16 targetID;
    TVec2 targetPos;
    u16 moveLine;

    TVec2 nextTile;

    uint8_t movePhase;
    // Current movement path. -1 is none.
    int8_t path;

    // Node the entity is moving (brute-forcing) towards
    uint8_t nextPathNode;

    uint8_t pathUpdateTime;

    int16_t hp;
    uint8_t data[32];
} TEntity;

typedef struct
{
    int16_t time;
    uint8_t dir;
} TMonsterData;

typedef struct {
    // Params
    int collisionMask;
    int ignoreEntity;
    int ignoreEntity2;
    int targetedEntity;

    // Final position
    TVec2 result;
    TVec2 resultDelta;

    int hitType;
    u16 hitEntity;
    u16 hitTile;
} TMoveInfo;

enum {
    HIT_TYPE_NONE,
    HIT_TYPE_MAP,
    HIT_TYPE_ENTITY
};

typedef struct bounds_s {
    TVec2 min, max;
} bounds_t;

typedef struct {
    int x, y;
} LinePoint;

#define MAX_PATH_TILES 128

typedef struct TPath
{
    // Tile index of next point. -1 means end of path
    int16_t p[MAX_PATH_TILES];

    // Next path in linked list
    int8_t next;
} TPath;

typedef struct TLine {
    int dx, dy, x, y, sx, sy, err, x0, y0, x1, y1;
    bool didReturnFirst;
    bool done;
} TLine;

typedef void (*EntityOnMoveCompleteFunc)(TEntity* self, TMoveInfo move);

extern entity_state_t entity_states[];

void Entity_Init(void);
TEntity* Entity_Alloc(int entity_type);
void Entity_Free(TEntity* e);
TEntity* EntityForIndex(int idx);
TEntity* EntityForID(int id);
void Entity_SetState(TEntity* e, int stateNum);
void Entity_StandardMove(TEntity* self, int speed, EntityOnMoveCompleteFunc onMoveComplete);

void EntityBounds(TEntity *e, bounds_t *bounds);
bool Entity_GetTargetPos(TEntity* self, TVec2* targetPos);

void Entity_SetTarget(TEntity *self, u16 x, u16 y);

void RegisterEntities(void);

// trace.c
bool WorldTrace(TVec2 origin, TVec2 delta, TVec2 mins, TVec2 maxs, TMoveInfo *trace);

// util.c
bool BoundsContainsXY(bounds_t a, int x, int y);
bool BoundsIntersectsBounds(bounds_t b0, bounds_t b1);
bounds_t UnionBounds(bounds_t a, bounds_t b);
bounds_t BoundsFromSize(TVec2 origin, TVec2 mins, TVec2 maxs);
bounds_t OffsetBounds(bounds_t bounds, TVec2 offset);
TVec2 BoundsCenter(bounds_t bounds);
int16_t BoundsDistance(bounds_t b0, bounds_t b1);

void Line_InitPool(void);
void Line_Init(TLine* line, int x0, int y0, int x1, int y1);
u16 Line_Alloc(void);
void Line_Free(u16 index);
TLine* Line_Get(u16 index);
void Line_GetPoint(TLine* line, LinePoint* pt);


int8_t Path_Alloc(void);
TPath *Path_Get(int8_t idx);
void Path_Free(int8_t idx);

void R_DrawStringCentered(u16 x, u16 y, u16 color, const char *s);

#endif
