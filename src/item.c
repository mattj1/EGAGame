#include "common.h"

static void Gold_Init(TEntity *self)
{
    self->flags |= ENTFLAG_HAS_TOOLTIP;
}

static void Gold_Tooltip(TEntity *self)
{
    R_DrawStringCentered(self->origin.x + 8, self->origin.y - 8, 15, "Gold");
}

void Gold_Register(ent_info_t *info) {

    info->mins.x = 0;
    info->mins.y = 0;
    info->maxs.x = 16;
    info->maxs.y = 16;

    info->collision = COLLISION_SOLID;

    info->initFunc = Gold_Init;
    info->tooltipFunc = Gold_Tooltip;

    // info->frameFunc = Monster_Update;
    // info->stateChangeFunc = Monster_StateChange;
}
