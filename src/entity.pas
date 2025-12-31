unit entity;

interface

uses fixed, common;


procedure Entity_Init;
function Entity_Alloc(entity_type: Integer): PEntity;
procedure Entity_MoveLine(var e: TEntity; var line: TLine);

function EntityForIndex(idx: word): PEntity;
function EntityForID(id: word): PEntity;
procedure Entity_SetState(var e: TEntity; stateNum: word);

implementation

uses trace, res;

var
	entities: array[0..MAX_ENT-1] of TEntity;
	gen: word;

procedure Entity_Init;
var i: word;
begin
	FillChar(entities, sizeof(entities), 0);
	for i := 0 to MAX_ENT-1 do begin
		entities[i].id := 0;
		entities[i].state := nil;
	end;

	writeln('Entity size:', sizeof(TEntity), ', array size: ', sizeof(entities));
end;

function EntityForIndex(idx: word): PEntity;
begin
	EntityForIndex := @entities[idx];
end;

function EntityForID(id: word): PEntity;
var e: PEntity;
begin
  EntityForID := nil;
  e := EntityForIndex(id shr 6);
  if not assigned(e) then Exit;

  if e^.id <> id Then Exit;

  EntityForID := e;
end;

procedure Entity_SetState(var e: TEntity; stateNum: word);
begin
  e.state := @entity_states[stateNum];
  e.stateTime := e.state^.numFrames;
end;


function Entity_Alloc_Slot(entity_type, entityNo: Integer): PEntity;
var
  e: PEntity;
begin
	gen := (gen + 1) and 63;

  e := @entities[entityNo];
  FillChar(e^, SizeOf(TEntity), 0);
  
  with e^ do begin
    info := @ent_info[entity_type];
    flags := ENTFLAG_ACTIVE or ENTFLAG_VISIBLE;
    nudge_time := 0;
    flash_time := 0;
    stateSpeed := 1;
    id := (entityNo shl 6) or gen;
    ownerID := id;
    collision := info^.collision;
    
    mins := e^.info^.mins;
    maxs := e^.info^.maxs;
    isMoving := False;
    targetID := 0;
  end;
  
  Entity_SetState(e^, STATE_NONE);

  Entity_Alloc_Slot := @entities[entityNo];
end;

function Entity_Alloc(entity_type: Integer): PEntity;
var
  i: Integer;
begin
  for i := 1 to MAX_ENT - 1 do
  begin
    if entities[i].state = nil then
    begin
      Entity_Alloc := entity_alloc_slot(entity_type, i);
      Exit;
    end;
  end;
  WriteLn('fatal: couldn''t alloc entity');
  Entity_Alloc := nil;
end;


procedure Entity_Free(e: PEntity);
begin
  e^.state := nil;
  e^.flags := 0;
  {if e^.item <> 0 then
  begin
    item_free(@items[e^.item]);
    e^.item := 0;
  end;
	}
end;


procedure Entity_MoveLine(var e: TEntity; var line: TLine);
var 
	pt: LinePoint;
	delta: TVec2;
	move: TMoveInfo;
begin	
  getPoint(line, pt);   

  delta.x := pt.x - e.origin.x;
  delta.y := pt.y - e.origin.y;

  move.collisionMask := COLLISION_SOLID;
  move.ignoreEntity := @e;
  move.ignoreEntity2 := nil;
  
  if not WorldTrace(e.origin, delta, e.mins, e.maxs, move) then begin
  	e.origin.x := pt.x;
  	e.origin.y := pt.y;
  end else begin
  	e.isMoving := false;
  end;

  {e.origin.x := move.result.x;
  e.origin.y := move.result.y;}	
end;

begin
end.
