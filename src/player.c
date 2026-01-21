#include "common.h"

void UpdateMoveLine(TEntity *self) {
    TVec2 dest;
    TEntity *e;
    TLine *line;

    // Get target location based on action
    switch (self->action) {
        case 1:
            // Action 1: Move to a specific position
            dest.x = self->targetPos.x;
            dest.y = self->targetPos.y;
            break;

        case 2:
            // Action 2: Move towards another entity
            e = EntityForID(self->targetID);
            if (e == NULL) {
//                printf("no entity for id %d %d\n", self->targetID, self->targetID >> 6);
                self->action = 0; // No target entity found, reset action
                return;
            } else {
                dest = e->origin; // Target entity's position
            }
            break;

        default:
            // Invalid or unknown action
            return;
    }

    if (self->action == 0) {
        return; // Exit since action is reset
    }

    // Mark the entity as moving
    self->isMoving = true;
    self->moveCount = 0;

    // Free the existing movement line
    Line_Free(self->moveLine);

    // Allocate a new movement line
    self->moveLine = Line_Alloc();
    if (self->moveLine != 0) {
        line = Line_Get(self->moveLine);
        // Initialize the movement line
        Line_Init(line,self->origin.x, self->origin.y,dest.x, dest.y);

        // Update entity state to indicate movement
        Entity_SetState(self, STATE_PLAYER_MOVE0);
    }
}

static void OnMoveComplete(TEntity *self, TMoveInfo move) {
    Entity_SetState(self, STATE_PLAYER_STAND0);
    if(self->targetID != 0 && move.hitEntity == self->targetID) {
//        printf("Player moved into target\n");
        Entity_SetState(self, STATE_PLAYER_ATTACK0);
    }
}

static void Player_Update(TEntity *self) {
    Entity_StandardMove(self, 256, OnMoveComplete);
}

static void Player_StateChange(TEntity *self) {
    (void)self;
    if(self->state->state == STATE_PLAYER_ATTACK2) {
//        printf("Damage target\n");

        if(self->targetID != 0) {
            TEntity *target = EntityForID(self->targetID);
            if(target != NULL) {
                target->flash_time = 8;
//                target->health -= 1;
            }
        }
    }
}

void Player_SetTarget(TEntity *self, u16 x, u16 y) {
    TEntity *e, *targetEntity = NULL;
    bounds_t bounds;
    int i;

    for(i = 1; i < MAX_ENT; i++) {
        e = EntityForIndex(i);
        if (e && e != self&& e->classID == ET_MONSTER) {
            EntityBounds(e, &bounds);

            if(BoundsContainsXY(bounds, x, y)) {
                targetEntity = e;
                break;
            }
        }
    }

    if(targetEntity != NULL) {
        self->targetID = targetEntity->id;
//        printf("Target ID: %d\n", self->targetID);
        self->action = 2;
    } else {
//        printf("move to point...\n");
        self->targetID = 0;
        self->action = 1;
        self->targetPos.x = x;
        self->targetPos.y = y;
    }

    UpdateMoveLine(self);
}

void Player_Register(ent_info_t *info) {

    info->mins.x = 1;
    info->mins.y = 1;
    info->maxs.x = 1 + 12;
    info->maxs.y = 1 + 14;

    info->collision = COLLISION_SOLID;

    info->frameFunc = Player_Update;
    info->stateChangeFunc = Player_StateChange;
}
