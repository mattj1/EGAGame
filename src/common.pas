unit common;
interface
uses fixed;
const
	MAX_ENT = 64;
	COLLISION_SOLID = 1;

	ENTFLAG_ACTIVE = 1;
	ENTFLAG_VISIBLE = 2;

type
	PEntityState = ^TEntityState;
	TEntityState = record

    state, nextState: word;
    numFrames:        word;
    spriteState: 	  word;
    {
    thinkFunc think;

     Run every frame
    frameFunc frame;

     Sprite ID that we're using
     }
  end;

	PEntityInfo = ^TEntityInfo;
	TEntityInfo = record
		mins: 			TVec2;
		maxs: 			TVec2;
		collision: 		word;
	end;
	PEntity = ^TEntity;

	TEntity = record 
		origin: 		TVec2;
		id: 			word;
		flags: 			word;
		ownerID:		word;
		mins: 			TVec2;
		maxs: 			TVec2;
		
		stateTime:		word;
		state: 			PEntityState;
		collision: 		word;
		info: 			PEntityInfo;

		nudge_time:		byte;
		flash_time:		byte;
		stateSpeed:		byte;


		{ 0 - none, 1 - move to, 2 - attack target }
		action: 		word;

		targetID:		word;
		targetPos:		TVec2;

		moveCount: word;

		isMoving:		boolean;
	end;


implementation
begin
end.