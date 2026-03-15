#include "common.h"


static void OnMoveComplete(TEntity* self, TMoveInfo move)
{
    Entity_SetState(self, STATE_PLAYER_STAND0);
    if (self->targetID != 0 && move.hitEntity == self->targetID)
    {
        //        printf("Player moved into target\n");
        Entity_SetState(self, STATE_PLAYER_ATTACK0);
    }
}

static void Player_Update(TEntity* self)
{
    // Entity_StandardMove(self, 256, OnMoveComplete);
    Entity_StandardMove(self, 256 + 128, OnMoveComplete);
}

static void Player_StateChange(TEntity* self)
{
    TEntity* target, *fx;
    if (self->state == STATE_PLAYER_ATTACK2)
    {
        //        printf("Damage target\n");

        if (self->targetID != 0)
        {
            target = EntityForID(self->targetID);
            if (target != NULL)
            {
                target->flash_time = 8;
                //                target->health -= 1;
                fx = Entity_Alloc(ET_EFFECT);
                if (fx != NULL)
                {
                    fx->origin = target->origin;
                    fx->origin.x -= 6;
                    fx->origin.y -= 12;
                    Entity_SetState(fx, STATE_SWIPE0);
                }
            }
        }
    }
}



void DrawMoveDebug(TEntity* e)
{
#ifdef PLATFORM_DESKTOP
    Image img = EGA_Raylib_GetBackBuffer();

    if (e->isMoving)
    {
        TVec2 targetPos = e->targetPos;
        if (e->targetID)
        {
            TEntity* t = EntityForID(e->targetID);

            if (t)
            {
                targetPos = t->origin;
                bounds_t otherBounds;
                EntityBounds(t, &otherBounds);
                TVec2 center = BoundsCenter(otherBounds);

                ImageDrawLine(&img, e->origin.x, e->origin.y, center.x, center.y, RED);
            }
        }
        else
        {
            ImageDrawLine(&img, e->origin.x, e->origin.y, e->targetPos.x, e->targetPos.y, RED);
        }

        if (e->path != -1)
        {
            TPath* path = Path_Get(e->path);

            for (int i = 0; i < 128; i++)
            {
                if (path->p[i] == -1) break;
                int idx = path->p[i];
                int px = idx % currentMap->width;
                int py = idx / currentMap->width;
                int left = px * 16;
                int top = py * 16;
                ImageDrawRectangleLines(&img, (Rectangle){left, top, 16, 16}, 1, RED);
            }
        }
    }
#endif
}

void Player_Register(ent_info_t* info)
{
    info->mins.x = -6;
    info->mins.y = -14;
    info->maxs.x = info->mins.x + 12;
    info->maxs.y = 0;

    info->collision = COLLISION_SOLID;

    info->frameFunc = Player_Update;
    info->stateChangeFunc = Player_StateChange;
}
