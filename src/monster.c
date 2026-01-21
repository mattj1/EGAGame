#include "common.h"


static void Monster_Update(TEntity *self) {
    (void)self;
}

static void Monster_StateChange(TEntity *self) {
    (void)self;
}

void Monster_Register(ent_info_t *info) {

    info->mins.x = 1;
    info->mins.y = 1;
    info->maxs.x = 1 + 12;
    info->maxs.y = 1 + 14;

    info->collision = 1;

    info->frameFunc = Monster_Update;
    info->stateChangeFunc = Monster_StateChange;
}
