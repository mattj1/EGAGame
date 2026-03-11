#include "common.h"


static void Monster_Update(TEntity *self) {
    TVec2 delta;
    TMoveInfo move;
    TMonsterData *data = (TMonsterData *) self->data;
    bool traceResult;

    data->time --;
    if (data->time <= 0)
    {
        data->time = 60;
        data->dir = rand() % 4;
    }

    self->moveCount += 127;
    while (self->moveCount >= 256)
    {
        self->moveCount -= 256;

        move.collisionMask = COLLISION_SOLID;
        move.ignoreEntity = self->id;
        move.ignoreEntity2 = 0;
        switch (data->dir)
        {
            case 0:
                delta.x = 0; delta.y = -1; break;
        case 1:
            delta.x = 0; delta.y = 1; break;
            case 2:
            delta.x = -1; delta.y = 0; break;
            case 3:
            delta.x = 1; delta.y = 0; break;
        }
        traceResult = WorldTrace(self->origin, delta, self->mins, self->maxs, &move);

        if (!traceResult)
        {
            self->origin = move.result;
        }
    }

}

static void Monster_StateChange(TEntity *self) {
    (void)self;

}

static void Monster_Init(TEntity *self)
{
    // TMonsterData *data = (TMonsterData *) &self->data[0];
    TMonsterData *data = (TMonsterData *) self->data;
    data->dir = 2;
    data->time = 60;
}

void Monster_Register(ent_info_t *info) {

    info->mins.x = -7;
    info->mins.y = -14;
    info->maxs.x = info->mins.x + 14;
    info->maxs.y = 0;

    info->collision = 1;

    info->initFunc = Monster_Init;
    info->frameFunc = Monster_Update;
    info->stateChangeFunc = Monster_StateChange;
}
