
Unit player;


Interface

Uses common, engine, fixed, entity, res;

Procedure Player_SetTarget(Var self: TEntity; x, y: word);
Procedure Player_Update(Var self: TEntity);

Implementation

Var moveLine: TLine;

Procedure UpdateMoveLine(Var self: TEntity);

Var dest: TVec2;
  e: PEntity;
Begin

 { Get target location based on action }

  Case self.action Of 
    1:
       Begin
         dest.x := self.targetPos.x;
         dest.y := self.targetPos.y;
       End;
    2:
       Begin
         e := EntityForID(self.targetID);
         If Not assigned(e) Then
           Begin
             writeln('no entity for id ', self.targetID, ' ', self.targetID shr 6);
             self.action := 0;
           End
         Else
           Begin
             dest := e^.origin;
           End;
       End;
  End;

  If self.action = 0 Then Exit;
 { if dest.x = self.x and dest.y = self.y then Exit; }



 { If target entity doesn't exist, then go back to action=0}

  self.isMoving := True;
  self.moveCount := 0;
  Line_Init(moveLine, self.origin.x, self.origin.y, dest.x, dest.y);
  Entity_SetState(self, STATE_PLAYER_MOVE0);


End;

Procedure Player_Update(Var self: TEntity);
Begin
  With self 
    Do
    Begin
      If self.isMoving Then
        Begin
          inc(self.moveCount, 256);

          While (self.moveCount >= 256) 
            Do
            Begin
              Entity_MoveLine(self, moveLine);
              Dec(self.moveCount, 256);

              If (self.origin.x = moveLine.x1) And (self.origin.y = moveLine.y1) Then
                Begin
                  self.isMoving := false;
                  Entity_SetState(self, STATE_PLAYER_STAND0);
                  break;
                End;
            End;
        End;
    End;
End;

Procedure Player_SetTarget(Var self: TEntity; x, y: word);

Var i: word;
  e: PEntity;
  bounds: TBounds;
  targetEntity: PEntity;
Begin

  targetEntity := Nil;

{	dest_x := cursor_x;
    dest_y := cursor_y;}

  { Find out if something is targetable }

  For i := 0 To MAX_ENT-1 
    Do
    Begin
      e := EntityForIndex(i);
      If Assigned(e^.state) And (e <> @self) Then
        Begin
          BoundsFromSize(e^.origin, e^.mins, e^.maxs, bounds);

          If BoundsContainsXY(bounds, x, y) Then
            Begin
              targetEntity := e;
              break;
            End;
        End;
    End;

  If Assigned(targetEntity) Then
    Begin
      Console_Print('target entity');
      self.targetID := targetEntity^.id;
      self.action := 2;
    End
  Else
    Begin
      Console_Print('move');
      self.targetID := 0;
      self.action := 1;
      self.targetPos.x := x;
      self.targetPos.y := y;
    End;

  UpdateMoveLine(self);

End;
Begin
End.
