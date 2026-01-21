{$X+}
program app;

uses
  {$ifdef PLATFORM_DOS}
  crt, 
  {$endif}

  {$ifdef PLATFORM_DESKTOP}
  raylib,
  {$endif}

  common,
  engine,
  Text,
  ega,
  level,
  fixed,
  gamedata,
  trace,
  entity,
  res,
  player;

type
  TPageInfo = record
    dirty_tiles: array[0..239] of byte;
  end;


var

  pages: array[0..1] of TPageInfo;
  font_data: array[0..3071] of byte;
  tile: array[0..8192] of byte;
  f: file;
  pg: word;
  x, y, i: word;
  dest_x, dest_y: fix16;
  cursor_x, cursor_y: word;
  map: TMap;

  plyr, monster: PEntity;


const
  mapData: array[0 .. 239] of word = (
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 5, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 5, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 6, 0,
    2, 0, 0, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 0, 2,
    2, 0, 8, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 8, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 5, 2, 0, 0, 0, 0, 4, 0, 0, 0, 2,
    2, 0, 0, 0, 3, 7, 0, 0, 0, 0, 2, 4, 0, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10);

  procedure mark_dirty_tiles_around_bb(left, top, right, bottom: integer);
  var
    x, y, x0, x1, y0, y1: word;
  begin
    if (bottom < 0) or (right < 0) then Exit;

    if left < 0 then left := 0;
    if top < 0 then top := 0;
    if right > 319 then right := 319;
    if bottom > 191 then bottom := 191;


    x0 := left shr 4;
    x1 := right shr 4;
    y0 := top shr 4;
    y1 := bottom shr 4;

    for y := y0 to y1 do
    begin
      for x := x0 to x1 do
      begin
        pages[0].dirty_tiles[x + y * map.Width] := 1;
        pages[1].dirty_tiles[x + y * map.Width] := 1;
      end;
    end;

  end;

var
  player_input: word;


  procedure Update(deltaTime: integer);
  begin
  end;

  {$ifdef PLATFORM_DESKTOP}
procedure Raylib_DrawTile(x, y, t: word);
var
  srcRect, dstRect: TRectangle;
  img: TImage;
begin
  img := EGA_Raylib_GetBackbuffer;
   srcRect.x := (t and 7) * 16;
   srcRect.y := (t and $fff8) shl 1;

   srcRect.width := 16;
   srcRect.height := 16;

   dstRect.x := x;
   dstRect.y := y;
   dstRect.width := 16;
   dstRect.height := 16;

   ImageDraw(@img, assets.raylib_tiles, srcRect, dstRect, WHITE);
end;
  {$endif}

  procedure draw_string(x, y, color: word; s: string);
  var
    i, l, ch, column, is_shift: word;
    fs: EGASprite;
    {$ifdef PLATFORM_DESKTOP}
  srcRect, dstRect: TRectangle;
  img: TImage;
  {$endif}
  begin
    l := length(s) - 1;

    {$ifdef PLATFORM_DOS}
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
  
  for i := 0 to l do begin
    ch := ord(s[i + 1]) - 32;
    if ch <> 0 then begin
      fs.data := @font_data[(1536 * is_shift) + 16 * (ord(s[i + 1]) - 32)];
      EGA_DrawSpriteFast(x shr 3, y, fs);
    end;
    inc(x, 4);
    is_shift := 1 - is_shift;
  end;
  {$else}


    img := EGA_Raylib_GetBackbuffer;
    srcRect.Width := 4;
    srcRect.Height := 8;
    dstRect.Width := 4;
    dstRect.Height := 8;

    for i := 0 to l do
    begin
      ch := Ord(s[i + 1]) - 32;

      srcRect.x := (ch and 15) * 8;
      srcRect.y := (ch and $fff0) shr 1;

      dstRect.x := x + i * 4;
      dstRect.y := y;

      ImageDraw(@img, assets.raylib_font, srcRect, dstRect, WHITE);
    end;
  {$endif}

  end;

  procedure Draw;
  var
    i: integer;
  var
    x, y: integer;
    dx, dy, l: fix16;
    mouse_buttons: word;
    pt: LinePoint;

    e: PEntity;
    bounds: TBounds;
  begin

    player_input := 0;

    if I_IsKeyDown(kUp) then player_input := player_input or 1;
    if I_IsKeyDown(kDn) then player_input := player_input or 2;
    if I_IsKeyDown(kLf) then player_input := player_input or 4;
    if I_IsKeyDown(kRt) then player_input := player_input or 8;

    if I_IsKeyDown(kEsc) then Loop_Cancel;

    mark_dirty_tiles_around_bb(plyr^.origin.x - 16, plyr^.origin.y -
      16, plyr^.origin.x + 40, plyr^.origin.y + 32);
    mark_dirty_tiles_around_bb(monster^.origin.x, monster^.origin.y,
      monster^.origin.x + monster^.maxs.x, monster^.origin.y + 16);

    mark_dirty_tiles_around_bb(cursor_x - 8, cursor_y - 8, cursor_x + 8, cursor_y + 8);
{
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
}
    {$ifdef PLATFORM_DOS}
  asm
    mov ax, 3
    int $33
    mov mouse_buttons, bx
    mov cursor_x, cx
    mov cursor_y, dx
  end;
  cursor_x := cursor_x div 2;
  {$else}
    Neo_Mouse_GetStatus(cursor_x, cursor_y, mouse_buttons);
    cursor_x := cursor_x div 2;
    cursor_y := cursor_y div 2;
  {$endif}

    { TODO: Detect when the mouse actually goes down }



    if (mouse_buttons and 1) <> 0 then
    begin
      Player_SetTarget(plyr^, cursor_x, cursor_y);
    end;

    Player_Update(plyr^);



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
    {$ifdef PLATFORM_DESKTOP}
  EGA_ClearScreen;
  {$endif}
    EGA_SetDrawPage(pg);
    set_rotate(0, 0);
    set_mask($ff);

    EGA_BeginDrawTiles;
    for y := 0 to (map.Height - 1) do
    begin
      for x := 0 to (map.Width - 1) do
      begin

        {$ifdef PLATFORM_DOS}
          If (pages[pg].dirty_tiles[x + y * map.width] = 1) Then
            Begin
               EGA_DrawTileFast(x, y, map.tiles^[x + y * map.width].t * 128); 
            End;
  {$else}
        Raylib_DrawTile(x * 16, y * 16, map.tiles^[x + y * map.Width].t);
  {$endif}
      end;
    end;
    EGA_EndDrawTiles;


    Dec(plyr^.stateTime);
    if plyr^.stateTime = 0 then
    begin
      Entity_SetState(plyr^, plyr^.state^.nextState);
    end;


    {player_y := player_y div 2;}
    EGA_DrawSpriteSlow(plyr^.origin.x, plyr^.origin.y,
      assets.sprites[plyr^.state^.spriteState]);
    EGA_DrawSpriteSlow(monster^.origin.x, monster^.origin.y,
      assets.sprites[monster^.state^.spriteState]);

    EGA_DrawSpriteSlow(cursor_x - 8, cursor_y - 8, assets.sprites[SPRITE_STATE_CURSOR0]);

{
  draw_string(16, 128, 15, 'Debug messages here');
  draw_string(16, 128 + 8, 15, ' Debug messages here');
  draw_string(16, 128 + 16, 15, '  Debug messages here');
}
    {  draw_string(player_x - 15, player_y - 15, 15, Concat('X: ', itoa(player_x), '  Y:', WordToString(player_y)));}
  { draw_string(player_x - 15, player_y - 15, 7, 'Healing Potion');
  }
    EGA_ShowPage(pg);
    EGA_WaitVerticalRetrace;
    fillchar(pages[pg].dirty_tiles, 240, 0);

    pg := 1 - pg;
  end;

  procedure do_keyboard_test;
  var
    i: integer;
  begin

    for i := 0 to 10 do
    begin
      writeln(i, ':');
      Timer_Delay(1000);
      Event_ProcessEvents;
      writeln(I_IsKeyDown(kDn));
      if (I_IsKeyDown(kEsc)) then Exit;
      engine.pressedKeys := [];

    end;

  end;

  procedure Shutdown;
  begin
    EGA_Close;
    Neo_Shutdown;
  end;

var
  j: word;
  {$ifdef fpc}
  tmpOutput: textfile;
  {$endif}
begin
  ExitProc := @Shutdown;

  Console_SetWriteStdOut(true);

  {$ifndef WIN32}
  {$ifdef fpc}
  Chdir('/home/matt/dos/tg2');
  {$endif}
  {$endif}

  {$ifdef UNIX}
  (*Assign(tmpOutput, '/tmp/textgamepipe');
  Rewrite(tmpOutput);
  Output := tmpOutput;
  Console_Print(#27'[2J');*)
  {$endif}

  Neo_Init;

  {$ifdef PLATFORM_DOS}
  ClrScr;
  TextColor(7);
  TextBackground(4);
  writeln('                                   NEO Engine                                   ');
  TextBackground(0);
  {$endif}

  Console_Print('Loading...');

  {$ifdef PLATFORM_DOS}
  writeln('MemAvail: ', MemAvail, ' MaxAvail: ', MaxAvail);
  {$endif}

  Assign(f, 'data/tile.ega');
  reset(f, 8192);
  blockread(f, tile, 1);
  System.Close(f);

  {$ifdef PLATFORM_DOS}
  assign(f, 'data/font.ega');
  reset(f, 3072);
  blockread(f, font_data, 1);
  System.Close(f);
  {$else}
  assets.raylib_font := LoadImage('dev/font.png');
  assets.raylib_tiles := LoadImage('dev/tile.png');
  {$endif}
{
  GetMem(res.sqrt, 65528);
  assign(f, 'data\sqrt.bin');
  reset(f, 65528);
  blockread(f, res.sqrt^, 1);
  System.Close(f);
}
  EGA_LoadSprite('player', assets.sprites[SPRITE_STATE_PLAYER_STAND0]);
  EGA_LoadSprite('player2', assets.sprites[SPRITE_STATE_PLAYER_MOVE0]);
  EGA_LoadSprite('player3', assets.sprites[SPRITE_STATE_PLAYER_MOVE1]);
  EGA_LoadSprite('cursor', assets.sprites[SPRITE_STATE_CURSOR0]);
  EGA_LoadSprite('monster', assets.sprites[SPRITE_STATE_MONSTER_STAND0]);

  Neo_Timer_Init;
  Neo_Mouse_Init;

  Line_InitPool;
  Entity_Init;

  RegisterEntities;

  Map_Alloc(20, 12, map);

  for i := 0 to (map.width * map.height - 1) do
  begin
    map.tiles^[i].t := mapData[i];
    map.tiles^[i].flags := 0;
    if mapData[i] = 2 then
    begin
      map.tiles^[i].flags := TILEFLAG_SOLID;
    end;
  end;

  writeln('Fixed point: Max coord val in pixels: ', f2i(32767));

  {$ifdef PLATFORM_DOS}
  writeln('MemAvail: ', MemAvail, ' MaxAvail: ', MaxAvail);

  if False then begin
    readln;
    Exit;
  end;
  {$endif}



{
  do_keyboard_test;
  Exit;
}

  {$ifdef PLATFORM_DOS}
  Console_SetWriteStdOut(false);
  {$endif}

  EGA_Init;

  setup_ega_for_masked_writes;

  set_rotate(0, 0);

  pg := 0;
  EGA_SetDrawPage(pg);
  EGA_ShowPage(1);

  fillchar(pages[0].dirty_tiles, 240, 1);
  fillchar(pages[1].dirty_tiles, 240, 1);



  plyr := Entity_Alloc(ET_PLAYER);
  plyr^.origin.x := 20;
  plyr^.origin.y := 20;
  Entity_SetState(plyr^, STATE_PLAYER_STAND0);

  monster := Entity_Alloc(ET_MONSTER);
  monster^.origin.x := 280;
  monster^.origin.y := 100;
  Entity_SetState(monster^, STATE_MONSTER_STAND0);

  EGA_SetDrawPage(0);
  EGA_ShowPage(0);
  set_rotate(0, 0);
  set_mask($ff);

  EGA_ClearScreen;

  {$ifdef PLATFORM_DOS}

  for i := 0 to 3 do begin
    EGA_SetPlanes(1 shl i);

    for j := 0 to (32 * 64 - 1) do begin
      MEM[$A400:j] := tile[i * 32 + j];
    end;
  end;
  {$endif}


  Keybrd_Init;

  Loop_SetUpdateProc(Update);
  Loop_SetDrawProc(Draw);
  Loop_Run;

end.
