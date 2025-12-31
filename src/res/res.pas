
unit res;
interface
uses common;

const
	STATE_NONE = 0;
	STATE_PLAYER_STAND0 = 1;
	STATE_PLAYER_MOVE0 = 2;
	STATE_PLAYER_MOVE1 = 3;
	STATE_PLAYER_MOVE2 = 4;
	STATE_PLAYER_MOVE3 = 5;
	STATE_PLAYER_ATTACK0 = 6;
	STATE_PLAYER_ATTACK1 = 7;
	STATE_PLAYER_ATTACK2 = 8;
	STATE_CURSOR0 = 9;
	STATE_MAX = 10;

	SPRITE_STATE_NONE = 0;
	SPRITE_STATE_PLAYER_STAND0 = 1;
	SPRITE_STATE_PLAYER_MOVE0 = 2;
	SPRITE_STATE_PLAYER_MOVE1 = 3;
	SPRITE_STATE_PLAYER_ATTACK0 = 4;
	SPRITE_STATE_PLAYER_ATTACK1 = 5;
	SPRITE_STATE_PLAYER_ATTACK2 = 6;
	SPRITE_STATE_CURSOR0 = 7;
	SPRITE_STATE_MAX = 8;

	ET_NONE = 0;
	ET_PLAYER = 1;
	ET_MONSTER = 2;

	entity_states: array[0..STATE_MAX-1] of TEntityState = (
		(state: STATE_NONE; nextState: STATE_NONE; numFrames: 60; spriteState: SPRITE_STATE_NONE),
		(state: STATE_PLAYER_STAND0; nextState: STATE_PLAYER_STAND0; numFrames: 30; spriteState: SPRITE_STATE_PLAYER_STAND0),
		(state: STATE_PLAYER_MOVE0; nextState: STATE_PLAYER_MOVE1; numFrames: 20; spriteState: SPRITE_STATE_PLAYER_MOVE0),
		(state: STATE_PLAYER_MOVE1; nextState: STATE_PLAYER_MOVE2; numFrames: 20; spriteState: SPRITE_STATE_PLAYER_STAND0),
		(state: STATE_PLAYER_MOVE2; nextState: STATE_PLAYER_MOVE3; numFrames: 20; spriteState: SPRITE_STATE_PLAYER_MOVE1),
		(state: STATE_PLAYER_MOVE3; nextState: STATE_PLAYER_MOVE0; numFrames: 20; spriteState: SPRITE_STATE_PLAYER_STAND0),
		(state: STATE_PLAYER_ATTACK0; nextState: STATE_PLAYER_ATTACK1; numFrames: 20; spriteState: SPRITE_STATE_PLAYER_ATTACK0),
		(state: STATE_PLAYER_ATTACK1; nextState: STATE_PLAYER_ATTACK2; numFrames: 20; spriteState: SPRITE_STATE_PLAYER_ATTACK1),
		(state: STATE_PLAYER_ATTACK2; nextState: STATE_PLAYER_ATTACK0; numFrames: 60; spriteState: SPRITE_STATE_PLAYER_ATTACK2),
		(state: STATE_CURSOR0; nextState: STATE_CURSOR0; numFrames: 60; spriteState: SPRITE_STATE_CURSOR0)
	);

    ent_info: array[0..2] of TEntityInfo = (
		(mins: (x:0; y:0); maxs: (x:0; y:0); collision: 0 ),
		(mins: (x:-1; y:-1); maxs: (x:11; y:13); collision: 1 ),
		(mins: (x:-1; y:-1); maxs: (x:11; y:13); collision: 1 )
	);
implementation
begin
end.
