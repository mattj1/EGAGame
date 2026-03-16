#include "common.h"

static TEntity entities[MAX_ENT];
static ent_info_t ent_info[MAX_ENT];
static int gen = 0;

TEntity* Entity_Alloc_Slot(int entity_type, int entityNo);
void Entity_MoveLine(TEntity* e, int lineNum, TMoveInfo* move);

// Implementation

void Entity_Init(void)
{
    int i;
    memset(entities, 0, sizeof(entities));
    for (i = 0; i < MAX_ENT; i++)
    {
        entities[i].id = 0;
        entities[i].state = 0;
    }

    LogInfo("Entity size: %u, array size: %u\n", sizeof(TEntity), sizeof(entities));
}

TEntity* EntityForIndex(int idx)
{
    TEntity *e = &entities[idx];
    if (idx < 0 || idx >= MAX_ENT)
    {
        return NULL;
    }

    if (e->classID == ET_NONE)
    {
        return NULL;
    }

    return e;
}

TEntity* EntityForID(int id)
{
    TEntity* e = EntityForIndex(id >> 6); // Equivalent of "id shr 6" in Pascal
    if (!e) return NULL;
    if (e->id != id) return NULL;
    return e;
}

void Entity_SetState(TEntity* e, int stateNum)
{
    entity_state_t *es = &entity_states[stateNum];
    e->state = stateNum;
    e->stateTime = es->numFrames;

    if (e->state == STATE_NONE)
    {
        e->classID = ET_NONE;
        e->flags = 0;
    }
}

TEntity* Entity_Alloc_Slot(int entity_type, int entityNo)
{
    TEntity* e;
    gen = (gen + 1) & 63;

    e = &entities[entityNo];
    memset(e, 0, sizeof(TEntity));

    e->info = &ent_info[entity_type];
    e->flags = ENTFLAG_ACTIVE | ENTFLAG_VISIBLE;
    e->state = STATE_NONE;

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
    e->path = -1;

    if (e->info->initFunc)
    {
        e->info->initFunc(e);
    }

    return e;
}

TEntity* Entity_Alloc(int entity_type)
{
    int i;
    for (i = 1; i < MAX_ENT; i++)
    {
        if (entities[i].classID == ET_NONE)
        {
            return Entity_Alloc_Slot(entity_type, i);
        }
    }
    printf("fatal: couldn't allocate entity\n");
    return NULL;
}

void Entity_Free(TEntity* e)
{
    e->state = NULL;
    e->flags = 0;
    // Free line or other associated resources if applicable
    // Line_Free(e->moveLine);
}



void Entity_TargetMove(TEntity *self, EntityOnMoveCompleteFunc onMoveComplete)
{

}

void Entity_SetTarget(TEntity* self, u16 x, u16 y)
{
    TEntity *e, *targetEntity = NULL;
    bounds_t bounds;
    int i, j, tx0, ty0, tx1, ty1, np;

    for (i = 1; i < MAX_ENT; i++)
    {
        e = EntityForIndex(i);
        if (e && e != self)
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
        self->pathUpdateTime = 0;
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


void RegisterEntity(unsigned int entityType, EntityRegisterFunc registerFunc)
{
    ent_info_t* ei = &ent_info[entityType];
    registerFunc(ei);
}

void RegisterEntities(void)
{
    extern void Player_Register(ent_info_t* info);
    extern void Monster_Register(ent_info_t* info);
    extern void Gold_Register(ent_info_t* info);

    RegisterEntity(ET_PLAYER, Player_Register);
    RegisterEntity(ET_MONSTER, Monster_Register);
    RegisterEntity(ET_GOLD, Gold_Register);
}

void EntityBounds(TEntity* e, bounds_t* bounds)
{
    bounds->min.x = e->origin.x + e->mins.x;
    bounds->min.y = e->origin.y + e->mins.y;
    bounds->max.x = e->origin.x + e->maxs.x;
    bounds->max.y = e->origin.y + e->maxs.y;
}


bool Entity_GetTargetPos(TEntity* self, TVec2* targetPos)
{
    TEntity* e;
    // Get target location based on action
    switch (self->action)
    {
    case 1:
        // Action 1: Move to a specific position
        targetPos->x = self->targetPos.x;
        targetPos->y = self->targetPos.y;
        return true;

    case 2:
        // Action 2: Move towards another entity
        e = EntityForID(self->targetID);
        if (e == NULL)
        {
            //                printf("no entity for id %d %d\n", self->targetID, self->targetID >> 6);
            self->action = 0; // No target entity found, reset action
            return false;
        }
        *targetPos = e->origin; // Target entity's position
        return true;

    default:
        // Invalid or unknown action
        return false;
    }
}
