unit player;


interface

uses common, engine, fixed, entity, res;

procedure Player_SetTarget(var self: TEntity; x, y: word);
procedure Player_Update(var self: TEntity);

procedure Player_Register(var info: TEntityInfo);

implementation

procedure UpdateMoveLine(var self: TEntity);
var
  dest: TVec2;
  e: PEntity;
begin

  { Get target location based on action }

  case self.action of
    1:
    begin
      dest.x := self.targetPos.x;
      dest.y := self.targetPos.y;
    end;
    2:
    begin
      e := EntityForID(self.targetID);
      if not assigned(e) then
      begin
        writeln('no entity for id ', self.targetID, ' ', self.targetID shr 6);
        self.action := 0;
      end
      else
      begin
        dest := e^.origin;
      end;
    end;
  end;

  if self.action = 0 then Exit;
  { if dest.x = self.x and dest.y = self.y then Exit; }



  { If target entity doesn't exist, then go back to action=0}

  self.isMoving := True;
  self.moveCount := 0;
  Line_Free(self.moveLine);
  self.moveLine := Line_Alloc;
  if self.moveLine <> 0 then
  begin
    Line_Init(Line_Get(self.moveLine)^, self.origin.x, self.origin.y, dest.x, dest.y);
    Entity_SetState(self, STATE_PLAYER_MOVE0);
  end;
end;


procedure OnMoveComplete(var self: TEntity; move: TMoveInfo);
begin
  Entity_SetState(self, STATE_PLAYER_STAND0);
  if (self.targetID <> 0) and (move.hitEntity = self.targetID) then
  begin
    Console_Print('Player moved into target');
  end;
end;

procedure Player_Update(var self: TEntity);
begin
  Entity_StandardMove(self, 256, OnMoveComplete);
end;

procedure Player_SetTarget(var self: TEntity; x, y: word);
var
  i: word;
  e: PEntity;
  bounds: TBounds;
  targetEntity: PEntity;
begin

  targetEntity := nil;

{  dest_x := cursor_x;
    dest_y := cursor_y;}

  { Find out if something is targetable }

  for i := 0 to MAX_ENT - 1 do
  begin
    e := EntityForIndex(i);
    if Assigned(e^.state) and (e <> @self) then
    begin
      BoundsFromSize(e^.origin, e^.mins, e^.maxs, bounds);

      if BoundsContainsXY(bounds, x, y) then
      begin
        targetEntity := e;
        break;
      end;
    end;
  end;

  if Assigned(targetEntity) then
  begin
    Console_Print('target entity');
    self.targetID := targetEntity^.id;
    self.action := 2;
  end
  else
  begin
    Console_Print('move');
    self.targetID := 0;
    self.action := 1;
    self.targetPos.x := x;
    self.targetPos.y := y;
  end;

  UpdateMoveLine(self);

end;

procedure Player_StateChange(var self: TEntity);
begin
end;

procedure Player_Register(var info: TEntityInfo);
begin
  { info.maxs := (x: 10; y: 10); }
  info.mins.x := 1;
  info.mins.y := 1;
  info.maxs.x := 1 + 12;
  info.maxs.y := 1 + 14;

  info.frameFunc := Player_Update;
  info.stateChangeFunc := Player_StateChange;
  Console_Print('Player_Register');
end;

(*codegen*)
(*codegen-end*)
begin
end.
