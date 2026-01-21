#include "common.h"

static TEntity entities[MAX_ENT];
static ent_info_t ent_info[MAX_ENT];
static int gen = 0;

TEntity* Entity_Alloc_Slot(int entity_type, int entityNo);
void Entity_MoveLine(TEntity* e, int lineNum, TMoveInfo* move);

// Implementation

void Entity_Init(void) {
    int i;
    memset(entities, 0, sizeof(entities));
    for (i = 0; i < MAX_ENT; i++) {
        entities[i].id = 0;
        entities[i].state = NULL;
    }

    printf("Entity size: %zu, array size: %zu\n", sizeof(TEntity), sizeof(entities));
}

TEntity* EntityForIndex(int idx) {
    return &entities[idx];
}

TEntity* EntityForID(int id) {
    TEntity* e = EntityForIndex(id >> 6); // Equivalent of "id shr 6" in Pascal
    if (!e) return NULL;
    if (e->id != id) return NULL;
    return e;
}

void Entity_SetState(TEntity* e, int stateNum) {
    e->state = &entity_states[stateNum];
    e->stateTime = e->state->numFrames;
}

TEntity* Entity_Alloc_Slot(int entity_type, int entityNo) {
    TEntity *e;
    gen = (gen + 1) & 63;

    e = &entities[entityNo];
    memset(e, 0, sizeof(TEntity));

    e->info = &ent_info[entity_type];
    e->flags = ENTFLAG_ACTIVE | ENTFLAG_VISIBLE;
    e->nudge_time = 0;
    e->flash_time = 0;
    e->stateSpeed = 1;
    e->id = (entityNo << 6) | gen;
    e->ownerID = e->id;
    e->classID = entity_type;
    e->collision = e->info->collision;
    e->mins = e->info->mins;
    e->maxs = e->info->maxs;
    e->isMoving = false;
    e->targetID = 0;
    e->moveLine = 0;

    Entity_SetState(e, STATE_NONE);

    return e;
}

TEntity* Entity_Alloc(int entity_type) {
    int i;
    for (i = 1; i < MAX_ENT; i++) {
        if (entities[i].state == NULL) {
            return Entity_Alloc_Slot(entity_type, i);
        }
    }
    printf("fatal: couldn't allocate entity\n");
    return NULL;
}

void Entity_Free(TEntity* e) {
    e->state = NULL;
    e->flags = 0;
    // Free line or other associated resources if applicable
    // Line_Free(e->moveLine);
}

void Entity_MoveLine(TEntity* e, int lineNum, TMoveInfo* move) {
    LinePoint pt;
    TVec2 delta;
    bool traceResult;
    TLine* line = Line_Get(lineNum);

    if (!line) {
        printf("Entity_MoveLine: line not found\n");
        return;
    }

    Line_GetPoint(line, &pt);

    delta.x = pt.x - e->origin.x;
    delta.y = pt.y - e->origin.y;

    traceResult = WorldTrace(e->origin, delta, e->mins, e->maxs, move);

    if (!traceResult) {
        e->origin.x = pt.x;
        e->origin.y = pt.y;
    } else {
        printf("moveLine: hit something\n");
        e->isMoving = false;
    }
}

void Entity_StandardMove(TEntity* self, int speed, EntityOnMoveCompleteFunc onMoveComplete) {
    TMoveInfo move;
    TLine* line;

    if(!self->isMoving) {
        return;
    }

    self->moveCount += speed;

    while (self->moveCount >= 256) {
        self->moveCount -= 256;

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
                break;
            }
        }
    }
}

void RegisterEntity(unsigned int entityType, EntityRegisterFunc registerFunc) {
    ent_info_t* ei = &ent_info[entityType];
    registerFunc(ei);
}

void RegisterEntities(void) {
    extern void Player_Register(ent_info_t *info);
    extern void Monster_Register(ent_info_t *info);

    RegisterEntity(ET_PLAYER, Player_Register);
    RegisterEntity(ET_MONSTER, Monster_Register);
}

void EntityBounds(TEntity *e, bounds_t *bounds) {
    bounds->min.x = e->origin.x + e->mins.x;
    bounds->min.y = e->origin.y + e->mins.y;
    bounds->max.x = e->origin.x + e->maxs.x;
    bounds->max.y = e->origin.y + e->maxs.y;
}

