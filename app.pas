{$X+}
Program app;

Uses engine, text, ega, crt, level, fixed, gamedata;


Type TPageInfo = Record
  dirty_tiles: array[0..240] Of byte;
End;


Var

  pages: array[0..1] Of TPageInfo;
  font_data: array[0..3071] of byte;
  tile: array[0..8192] Of byte;
  f: file;
  pg: word;
  x, y, i: word;
  player_x: integer;
  player_y: integer;
  dest_x, dest_y: fix16;
  cursor_x, cursor_y: integer;
  map: TMap;
  isMoving: boolean;
  moveLine: TLine;
  moveCount: word;


const mapData: array[0 .. 239] of word = (
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  
  2,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,2,  
  2,0,0,5,0,0,0,0,8,0,0,0,0,0,9,0,0,0,0,2,  
  2,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,2,  
  2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,5,2,  
  2,0,0,0,0,0,0,0,0,0,0,0,9,0,0,0,0,0,6,0,  
  2,0,0,0,0,0,7,0,0,0,2,0,0,0,5,0,0,0,0,2,  
  2,0,8,0,0,0,0,0,0,0,2,0,0,8,0,0,0,0,0,2,  
  2,0,0,0,0,0,0,0,0,5,2,0,0,0,0,4,0,0,0,2,  
  2,0,0,0,3,7,0,0,0,0,2,4,0,0,0,0,0,0,0,2,  
  2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,  
  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10
);

Procedure mark_dirty_tiles_around_bb(left, top, right, bottom: integer);

Var 
  x, y, x0, x1, y0, y1: word;

Begin
  If (bottom < 0) Or (right < 0) Then Exit;

  If left < 0 Then left := 0;
  If top < 0 Then top := 0;
  if right > 319 then right := 319;
  if bottom > 191 then bottom := 191;


  x0 := left shr 4;
  x1 := right shr 4;
  y0 := top shr 4;
  y1 := bottom shr 4;


  For y := y0 To y1 Do
    Begin
      For x := x0 To x1 Do
        Begin
          pages[0].dirty_tiles[x + y * 20] := 1;
          pages[1].dirty_tiles[x + y * 20] := 1;
        End;
    End;

End;

Var player_input: word;


Procedure Update(deltaTime: integer);
Begin
End;

procedure draw_string(x, y, color: word; s: string);
var
  i, l, column, is_shift: word;
  fs: EGASprite;
begin
  is_shift := 0;
  x := x and $fffc;
  if x and 7 <> 0 then is_shift := 1;

  fs.column_count := 1;
  fs.width := 8;
  fs.height := 8;
  fs.num_channels := 2;
  fs.num_color_channels := 1;
  fs.planes_and := 0;
  fs.planes_or := color;
  
  l := length(s) - 1;

  for i := 0 to l do begin
    fs.data := @font_data[(1536 * is_shift) + 16 * (ord(s[i + 1]) - 32)];
EGA_DrawSpriteFast(x shr 3, y, fs);

    inc(x, 4);
    is_shift := 1 - is_shift;
  end;
end;

Procedure Draw;

Var i: integer;
var x, y: integer;
  dx, dy, l: fix16;
  mouse_buttons: word;
  pt: LinePoint;
Begin

  player_input := 0;

  If I_IsKeyDown(kUp) Then player_input := player_input Or 1;
  If I_IsKeyDown(kDn) Then player_input := player_input Or 2;
  If I_IsKeyDown(kLf) Then player_input := player_input Or 4;
  If I_IsKeyDown(kRt) Then player_input := player_input Or 8;

  If I_IsKeyDown(kEsc) Then Loop_Cancel;

  mark_dirty_tiles_around_bb(player_x - 16, player_y - 16, player_x + 40, player_y + 32);
  mark_dirty_tiles_around_bb(cursor_x, cursor_y, cursor_x + 16, cursor_y + 16);

  If (player_input And 1) <> 0 Then
    Begin
      inc(player_y, 1);
    End;
  If (player_input And 2) <> 0 Then
    Begin
      inc(player_y, 1);
    End;
  If (player_input And 4) <> 0 Then
    Begin
      inc(player_x, 1);
    End;
  If (player_input And 8) <> 0 Then
    Begin
      inc(player_x, 1);
    End;

  asm
    mov ax, 3
    int $33
    mov mouse_buttons, bx
    mov cursor_x, cx
    mov cursor_y, dx
end;
  { TODO: Detect when the mouse actually goes down }

  cursor_x := cursor_x div 2; 

  if (mouse_buttons and 1) <> 0 then begin
    dest_x := cursor_x;
    dest_y := cursor_y;

    if (player_x <> dest_x) or (player_y <> dest_y) then begin
    isMoving := True;
    Line_Init(moveLine, player_x, player_y, dest_x, dest_y);
    moveCount := 0;
    end;
  end;

  if isMoving then begin
    inc(moveCount, 244);

    while (moveCount >= 256) do begin
      getPoint(moveLine, pt);
      Dec(moveCount, 256);

      player_x := pt.x;
      player_y := pt.y;

      if (pt.x = moveLine.x1) and (pt.y = moveLine.y1) then begin
        isMoving := false;
        Exit;
      end;
    end;
  end;

    {mark_dirty_tiles_around_bb(player_x - 16, player_y - 16, player_x + 40, player_y + 32);}
  engine.pressedKeys := [];

{  If Timer_GetTicks > 3000 Then Loop_Cancel;
}


{
  If player_input <> 0 Then writeln('player input ', player_input);
  If I_IsKeyDown(kA) Then writeln('A pressed');


  If I_WasKeyPressed(kA) Then writeln('A was pressed');
}
  {pg := 0;
  }
  EGA_SetDrawPage(pg);
  set_rotate(0, 0);
  set_mask($ff);

  EGA_BeginDrawTiles;
  For y := 0 To 11 Do
    Begin
      For x := 0 To 19 Do
        Begin
          If (pages[pg].dirty_tiles[x + y * 20] = 1) Then
            Begin
               draw_tile_fast(x, y, map.tiles^[y * 20 + x].t * 128); 
            End;
        End;
    End;
  EGA_EndDrawTiles;

   
  {player_y := player_y div 2;}
  EGA_DrawSpriteSlow(player_x, player_y, res.sprite_player);
  EGA_DrawSpriteSlow(cursor_x, cursor_y, res.sprite_cursor);
  
  {
  draw_string(96, 20, 15, 'Healing Potion');
  }

  {  draw_string(player_x - 15, player_y - 15, 15, Concat('X: ', itoa(player_x), '  Y:', WordToString(player_y)));}
  { draw_string(player_x - 15, player_y - 15, 7, 'Healing Potion');
  } EGA_ShowPage(pg);
  WaitVerticalRetrace;
  fillchar(pages[pg].dirty_tiles, 240, 0);
  pg := 1 - pg;
End;

procedure do_keyboard_test;
  var i: integer;
  Begin

  for i := 0 to 10 do begin
    writeln(i, ':');
    Timer_Delay(1000);
    Event_ProcessEvents;
    writeln(I_IsKeyDown(kDn));
    if(I_IsKeyDown(kEsc)) then Exit;
    engine.pressedKeys := [];

  end;

end;

Procedure Shutdown;
Begin
  EGA_Close;
  Neo_Shutdown;
End;

var j: word;

Begin
  ExitProc := @Shutdown;
  Neo_Init;

  ClrScr;
  TextColor(7);
  TextBackground(4);
  writeln('                                   NEO Engine                                   ');
  TextBackground(0);

  writeln('Loading...');

  assign(f, 'tile.ega');
  reset(f, 8192);
  blockread(f, tile, 1);
  System.Close(f);

  assign(f, 'font.ega');
  reset(f, 3072);
  blockread(f, font_data, 1);
  System.Close(f);

  GetMem(res.sqrt, 65528);
  assign(f, 'data\sqrt.bin');
  reset(f, 65528);
  blockread(f, res.sqrt^, 1);
  System.Close(f);

  EGA_LoadSprite('player.ega', res.sprite_player);
  EGA_LoadSprite('cursor.ega', res.sprite_cursor);
  

  Timer_Init;
  Neo_Mouse_Init;

  Map_Alloc(20, 18, map);
  For i := 0 to 239 do map.tiles^[i].t := mapData[i];

  writeln('Fixed point: Max coord val in pixels: ', f2i(32767));

  
  readln;
  {Exit;
  }
  
{
  do_keyboard_test;
  Exit;
}

  EGA_Init;

  setup_ega_for_masked_writes;

  set_rotate(0, 0);

  pg := 0;
  EGA_SetDrawPage(pg);
  EGA_ShowPage(1);

  fillchar(pages[0].dirty_tiles, 240, 1);
  fillchar(pages[1].dirty_tiles, 240, 1);

  player_x := 10;
  player_y := 10;

  EGA_SetDrawPage(0);
  EGA_ShowPage(0);
set_rotate(0, 0);
  set_mask($ff);

  EGA_ClearScreen;


  for i := 0 to 3 do begin
    EGA_SetPlanes(1 shl i);

    for j := 0 to (32 * 64 - 1) do begin
      MEM[$A400:j] := tile[i * 32 + j];
    end;
  end;


  Keybrd_Init;

  Loop_SetUpdateProc(Update);
  Loop_SetDrawProc(Draw);
  Loop_Run;
  Exit;
  
  EGA_SetDrawPage(0);
  for i := 1 to 60 do begin
    {EGA_ClearScreen;}
    EGA_SetDrawPage(pg);
    EGA_SetPlanes(15);
    
    EGA_BeginDrawTiles;
  
      For y := 0 To 11 Do
      Begin

        For x := 0 To 19 Do
          Begin
            draw_tile_fast(x, y, 128 * 1);
          End;
          
      End;
      
      EGA_EndDrawTiles;

      set_mask($ff);
      {for y := 0 to 9 do begin
        EGA_DrawSpriteFast(-1 + y * 2, i, @sprite[0]);
      end;
      for y := 0 to 9 do begin
        EGA_DrawSpriteSlow(-8 + y * 24, 50 + i, @sprite[0]);
      end;
      }
      EGA_ShowPage(pg);
      WaitVerticalRetrace;
      pg := 1 - pg;
  end;

  readln;

  { Shutdown gets called here }
  Exit;

End.
