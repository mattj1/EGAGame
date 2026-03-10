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
        entities[i].state = NULL;
    }

    LogInfo("Entity size: %u, array size: %u\n", sizeof(TEntity), sizeof(entities));
}

TEntity* EntityForIndex(int idx)
{
    return &entities[idx];
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
    e->state = &entity_states[stateNum];
    e->stateTime = e->state->numFrames;
}

TEntity* Entity_Alloc_Slot(int entity_type, int entityNo)
{
    TEntity* e;
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
    e->path = -1;

    Entity_SetState(e, STATE_NONE);

    return e;
}

TEntity* Entity_Alloc(int entity_type)
{
    int i;
    for (i = 1; i < MAX_ENT; i++)
    {
        if (entities[i].state == NULL)
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
    // TMoveInfo move;
    // TLine* line;

    if (!self->isMoving)
    {
        return;
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

void RegisterEntity(unsigned int entityType, EntityRegisterFunc registerFunc)
{
    ent_info_t* ei = &ent_info[entityType];
    registerFunc(ei);
}

void RegisterEntities(void)
{
    extern void Player_Register(ent_info_t* info);
    extern void Monster_Register(ent_info_t* info);

    RegisterEntity(ET_PLAYER, Player_Register);
    RegisterEntity(ET_MONSTER, Monster_Register);
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
