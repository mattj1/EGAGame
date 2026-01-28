#include "common.h"


static void Monster_Update(TEntity *self) {
    (void)self;
}

static void Monster_StateChange(TEntity *self) {
    (void)self;
}

void Monster_Register(ent_info_t *info) {

    info->mins.x = -7;
    info->mins.y = -14;
    info->maxs.x = info->mins.x + 14;
    info->maxs.y = 0;

    info->collision = 1;

    info->frameFunc = Monster_Update;
    info->stateChangeFunc = Monster_StateChange;
}
