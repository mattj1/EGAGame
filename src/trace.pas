unit trace;

interface

uses fixed, common;

const
	HIT_TYPE_MAP = 1;
	HIT_TYPE_ENTITY = 2;
	


type

	TMoveInfo = record 
    { params }
    collisionMask: word;
    ignoreEntity: PEntity;
    ignoreEntity2: PEntity;

    { Final position }
    result: TVec2;

    hitEntity: PEntity;
    hitTile: boolean;
    hitType: word;
	end;

  TNearCollide = record
    bounds: TBounds;
    { If nil, then we're referring to a tile }
    e: PEntity;
    tile: Integer;
  end;


function WorldTrace(origin, delta, mins, maxs: TVec2; var trace: TMoveInfo): Boolean;

implementation

uses entity, level;

const
  MAX_NEAR_COLLIDE = 32;

var
  nearCollideCount: Integer;
  near_collide: array[0..MAX_NEAR_COLLIDE-1] of TNearCollide;
  moveBounds: TBounds;

procedure CollectEntities(var trace: TMoveInfo);
var
  i, tx0, tx1, ty0, ty1, tx, ty: Integer;
  e: PEntity;
  bounds, tileBounds: TBounds;
  t: PTile;
  check: Boolean;
begin
  nearCollideCount := 0;

  for i := 0 to MAX_ENT - 1 do
  begin
    e := EntityForIndex(i);
    if (e^.state = nil) or 
       (e = trace.ignoreEntity) or 
       (e = trace.ignoreEntity2) or
       ((e^.collision and trace.collisionMask) = 0) then
      Continue;

    BoundsFromSize(e^.origin, e^.mins, e^.maxs, bounds);

    bounds.max.x := bounds.max.x + 1; { this was intToFix(1) ... }
    bounds.max.y := bounds.max.y + 1;

    if BoundsIntersectsBounds(moveBounds, bounds) then
    begin
      near_collide[nearCollideCount].bounds := bounds;
      near_collide[nearCollideCount].e := e;
      Inc(nearCollideCount);
    end;
  end;

  tx0 := moveBounds.min.x shr 4;
  tx1 := moveBounds.max.x shr 4;
  ty0 := moveBounds.min.y shr 4;
  ty1 := moveBounds.max.y shr 4;

  for ty := ty0 to ty1 do
  begin
    for tx := tx0 to tx1 do
    begin
      t := Map_TileAt(currentMap^, tx, ty);

      check := False;

      if (t^.flags and TILEFLAG_SOLID) <> 0 then
      begin
        if (trace.collisionMask and COLLISION_SOLID) <> 0 then
          check := True;
      end;
{
      if (t and TILEFLAG_HIDDEN) <> 0 then
      begin
        if (trace.collisionMask and COLLISION_HIDDEN_TILE) <> 0 then
          check := True;
      end;
}
      if not check then
        Continue;

      tileBounds.min.x := tx shl 4;
      tileBounds.min.y := ty shl 4;
      tileBounds.max.x := (tx + 1) shl 4;
      tileBounds.max.y := (ty + 1) shl 4;

      near_collide[nearCollideCount].bounds := tileBounds;
      near_collide[nearCollideCount].e := nil;
      Inc(nearCollideCount);
    end;
  end;
end;

function WorldTrace(origin, delta, mins, maxs: TVec2; var trace: TMoveInfo): Boolean;
var
  didHit: Boolean;
  startBounds, endBounds: TBounds;
  i: Integer;
  nc: ^TNearCollide;
  bounds: TBounds;
  d: fix16;
begin
  didHit := False;

  trace.hitType := 0; { 0 = none, 1 = tile, 2 = entity }
  trace.hitEntity := nil;
  trace.hitTile := False;

  maxs.x := maxs.x + 1; { was IntToFix...}
  maxs.y := maxs.y + 1;

  BoundsFromSize(origin, mins, maxs, startBounds);
  OffsetBounds(startBounds, delta, endBounds);

  UnionBounds(moveBounds, startBounds, endBounds);

  { Find relevant entities inside bounds }
  CollectEntities(trace);

  endBounds := startBounds;

  if delta.x <> 0 then
  begin
    endBounds.min.x := endBounds.min.x + delta.x;
    endBounds.max.x := endBounds.max.x + delta.x;

    for i := 0 to nearCollideCount - 1 do
    begin
      nc := @near_collide[i];
      bounds := nc^.bounds;

      if not BoundsIntersectsBounds(endBounds, bounds) then
        Continue;

      if delta.x < 0 then
      begin
        if endBounds.min.x < bounds.max.x then
        begin
          d := bounds.max.x - endBounds.min.x;
          endBounds.min.x := endBounds.min.x + d;
          endBounds.max.x := endBounds.max.x + d;

          didHit := True;

          if nc^.e <> nil then
          begin
            trace.hitType := HIT_TYPE_ENTITY;
            trace.hitEntity := nc^.e;
          end
          else
          begin
            trace.hitType := HIT_TYPE_MAP;
          end;
        end;
      end;

      if delta.x > 0 then
      begin
        if endBounds.max.x >= bounds.min.x then
        begin
          d := endBounds.max.x - bounds.min.x;
          endBounds.min.x := endBounds.min.x - d;
          endBounds.max.x := endBounds.max.x - d;

          didHit := True;
          if nc^.e <> nil then
          begin
            trace.hitType := HIT_TYPE_ENTITY;
            trace.hitEntity := nc^.e;
          end
          else
          begin
            trace.hitType := HIT_TYPE_MAP;
          end;
        end;
      end;
    end;
  end;

  if delta.y <> 0 then
  begin
    endBounds.min.y := endBounds.min.y + delta.y;
    endBounds.max.y := endBounds.max.y + delta.y;

    for i := 0 to nearCollideCount - 1 do
    begin
      nc := @near_collide[i];
      bounds := near_collide[i].bounds;

      if not BoundsIntersectsBounds(endBounds, bounds) then
        Continue;

      if delta.y < 0 then
      begin
        if endBounds.min.y < bounds.max.y then
        begin
          d := bounds.max.y - endBounds.min.y;
          endBounds.min.y := endBounds.min.y + d;
          endBounds.max.y := endBounds.max.y + d;

          didHit := True;
          if nc^.e <> nil then
          begin
            trace.hitType := HIT_TYPE_ENTITY;
            trace.hitEntity := nc^.e;
          end
          else
          begin
            trace.hitType := HIT_TYPE_MAP;
          end;
        end;
      end;

      if delta.y > 0 then
      begin
        if endBounds.max.y > bounds.min.y then
        begin
          d := endBounds.max.y - bounds.min.y;
          endBounds.min.y := endBounds.min.y - d;
          endBounds.max.y := endBounds.max.y - d;

          didHit := True;
          if nc^.e <> nil then
          begin
            trace.hitType := HIT_TYPE_ENTITY;
            trace.hitEntity := nc^.e;
          end
          else
          begin
            trace.hitType := HIT_TYPE_MAP;
          end;
        end;
      end;
    end;
  end;

  trace.result.x := trace.result.x + origin.x + (endBounds.min.x - startBounds.min.x);
  trace.result.y := trace.result.y + origin.y + (endBounds.min.y - startBounds.min.y);

  WorldTrace := trace.hitType <> 0;
end;

begin
end.