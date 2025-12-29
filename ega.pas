unit ega;
{$L TEST.OBJ}
interface

type 
  byte_ptr = ^byte;
  
  EGASprite = record
    column_count, width, height: word;

    num_channels, num_color_channels: word;
    
    planes_and: word;
    planes_or: word;

    data: Pointer;
  end;


procedure EGA_Init;
procedure EGA_SetPlanes(planes_mask: byte);
procedure EGA_Close;
procedure EGA_DrawTileFast(x, y, src_seg, src_offs, dest_seg: word);

procedure draw_tile_fast(x, y: word; vmem_offs: word);
procedure EGA_DrawSpriteFast(x, y: integer; var sprite: EGASprite);
procedure EGA_DrawSpriteSlow(x, y: integer; var sprite: EGASprite);
procedure setup_ega_for_masked_writes;
procedure set_rotate(op, rotate: byte);
procedure EGA_SetDrawPage(p: word);
procedure EGA_ShowPage(p: word);
procedure set_mask(bit_mask: byte);
procedure WaitVerticalRetrace;
procedure EGA_ClearScreen;


procedure EGA_BeginDrawTiles;
procedure EGA_EndDrawTiles;

procedure EGA_DrawSpriteMaskFast2(src_seg, src_offs, src_skip, dest_seg, dest_offs, dest_rewind, num_cols, num_rows: word);

function EGA_LoadSprite(path: string; var out: EGASprite): boolean;

implementation

procedure EGA_Init; external;
procedure EGA_SetPlanes(planes_mask: byte); External;
procedure EGA_DrawTileFast(x, y, src_seg, src_offs, dest_seg: word); external;
procedure EGA_DrawSpriteMaskFast2(
      src_seg, src_offs, src_skip, dest_seg, dest_offs, dest_rewind, num_cols, num_rows: word);
      external;

var 
   i, p, x, y, o: integer;
   draw_segment: word;

const
  INPUT_STATUS_1 = $3DA;  { EGA/VGA input status register }
  VRETRACE_BIT   = $08;   { Bit 3 indicates vertical retrace }

{ Wait for vertical retrace to end }
procedure WaitVRetraceEnd;
begin
  while (Port[INPUT_STATUS_1] and VRETRACE_BIT) <> 0 do
    { busy wait };
end;

{ Wait for vertical retrace to start }
procedure WaitVRetraceStart;
begin
  while (Port[INPUT_STATUS_1] and VRETRACE_BIT) = 0 do
    { busy wait };
end;

{ Complete vertical retrace synchronization }
procedure WaitVerticalRetrace;
begin
  WaitVRetraceEnd;    { Wait until we're not in retrace }
  WaitVRetraceStart;  { Wait for retrace to begin }
end;

procedure setup_ega_for_masked_writes;
begin
   asm
      mov dx, $3ce

      { Register 0: Set/Reset - set to 0 (we'll use CPU data) }
      mov ax, $0000
      out dx, ax
      
      { Register 1: Enable Set/Reset - set to 0 (disable, use CPU data) }
      mov ax, $0001
      out dx, ax
      
      { Register 3: Data Rotate/Function Select - set to 0 (replace mode, no rotation) }
      mov ax, $0003
      out dx, ax
      
      { Register 8: Bit Mask - set to $FF (all bits enabled initially) }
      mov ax, $FF08
      out dx, ax
   end;
end;

procedure set_mask(bit_mask: byte);
begin
   asm
      mov dx, $3ce
      mov al, 8
      mov ah, bit_mask
      out dx, ax
   end;
end;

procedure EGA_Close;
begin
asm
   mov al, $3
   mov ah, 0
   int $10
end;
end;

procedure set_rotate(op, rotate: byte);
var val: byte;
begin
   val := (op shl 3) or rotate;
   asm
      mov dx, $3ce
      mov al, 3
      mov ah, val
      out dx, ax
   end;
end;

{
  - Aligned on X to 8 pixels
  - Sprite transparency
  - Clipping

  - Batching to reduce plane changes
}


procedure EGA_BeginDrawTiles;
begin
   asm
      { Enable all planes }
      mov dx, $3c4
      mov ax, $0f02
      out dx, ax
      
      { Set up latched copy }
      mov dx, $3ce
      mov ax, $0105
      out dx, ax
   end;
end;

procedure EGA_EndDrawTiles;
begin
   asm
   mov dx, $3ce
   mov ax, $0005
   out dx, ax
   end;
end;

{
   ########  ########     ###    ##      ##         ######## #### ##       ######## 
   ##     ## ##     ##   ## ##   ##  ##  ##            ##     ##  ##       ##       
   ##     ## ##     ##  ##   ##  ##  ##  ##            ##     ##  ##       ##       
   ##     ## ########  ##     ## ##  ##  ##            ##     ##  ##       ######   
   ##     ## ##   ##   ######### ##  ##  ##            ##     ##  ##       ##       
   ##     ## ##    ##  ##     ## ##  ##  ##            ##     ##  ##       ##       
   ########  ##     ## ##     ##  ###  ###  #######    ##    #### ######## ######## 
}

procedure draw_tile_fast(x, y: word; vmem_offs: word);
   var p: word;
   base_offs: word;
   srcseg, srcofs: word;
begin

   if (x < 0) or (x > 39) then Exit;
(*
   x := x shl 1;
   y := y shl 1;
   base_offs := x + (y shl 8) + (y shl 6); {y * 320}
  *) 
   EGA_DrawTileFast(x, y, $A400, vmem_offs, draw_segment);
end;


procedure EGA_DrawSpriteSlow(x, y: integer; var sprite: EGASprite);
var
   src_x0, src_y0, src_x1, src_y1: integer;
   dest_x0, dest_y0, dest_x1, dest_y1: integer;
   
   src_column0, src_column1: integer;
   dst_column0, dst_column1: integer;

   start_column0, start_column1: integer;

   num_rows, num_cols, num_cols2: integer;

   src_seg, src_offs: word;
   dest_seg, dest_offs: word;
   src_skip, dest_rewind: word;

   shift_amount: word;
   clipped: word;
   dest_offs_offs1: word;
begin

   if (x mod 8) = 0 then begin
     EGA_DrawSpriteFast(x div 8, y, sprite);
      Exit;
   end;
   src_x0 := 0;
   src_x1 := (sprite.width - 1);
   src_y0 := 0;
   src_y1 := (sprite.height - 1);

   dest_x0 := x;
   dest_x1 := x + src_x1;
   dest_y0 := y;
   dest_y1 := y + src_y1;

   if (dest_x0 > 319) or (dest_x1 < 0) or (dest_y0 > 199) or (dest_y1 < 0) then Exit;

   { Clipping }

   clipped := 0;

   if dest_x0 < 0 then begin
      clipped := 2;
      Inc(src_x0, -dest_x0);
      dest_x0 := 0;
   end;
   
   if dest_x1 > 319 then begin
      clipped := 1;
      Dec(src_x1, dest_x1 - 319);
      dest_x1 := 319;
   end;   

   if dest_y0 < 0 then begin
      Inc(src_y0, -dest_y0);
      dest_y0 := 0;
   end;

   if dest_y1 > 199 then begin
      Dec(src_y1, dest_y1 - 199);
      dest_y1 := 199;
   end;

   src_column0 := src_x0 shr 3;
   src_column1 := src_x1 shr 3;
   dst_column0 := dest_x0 shr 3;
   dst_column1 := dest_x1 shr 3;

   num_rows := dest_y1 - dest_y0; { +1 ?}
   num_cols := src_column1 - src_column0;
   num_cols2 := num_cols;

   dest_offs_offs1 := 1;

   src_seg := seg(sprite.data^);
   
   dest_rewind := num_rows * 40 + (sprite.height - 1);
   dest_rewind := (num_rows + 1) * 40 - 1;
   src_skip := sprite.height * 4 + ((sprite.height - num_rows - 1));
   dest_offs := (dst_column0) + (dest_y0) * 40;

   shift_amount := x and 7;

   if clipped = 0 then begin
      src_offs := ofs(sprite.data^) + sprite.height * (src_column0 * 5 + 4) + src_y0;
   end;

   if clipped = 2 then begin
      { Number of columns for the first iteration decreases }
      Dec(num_cols);

      { Increase source column }
      src_offs := ofs(sprite.data^) + sprite.height * ((src_column0 + 1) * 5 + 4) + src_y0;

      { Destination offset doesn't have 1 added to it }
      dest_offs_offs1 := 0;
   end;

   if clipped = 1 then begin
      { If we're being clipped on the right, for the 2nd iteration, draw less columns}
      Dec(num_cols2);

      src_offs := ofs(sprite.data^) + sprite.height * (src_column0 * 5 + 4) + src_y0;
   end;

   { Draw mask using AND operation }
   EGA_SetPlanes(15);
   set_rotate(1, shift_amount);

   set_mask($ff shr shift_amount);

   if num_cols >= 0 then
      EGA_DrawSpriteMaskFast2(src_seg, src_offs, src_skip, 
         draw_segment, dest_offs, dest_rewind, 
         num_cols + 1, num_rows + 1);

   src_offs := ofs(sprite.data^) + sprite.height * (src_column0 * 5 + 4) + src_y0;

   set_mask($ff shl (8-shift_amount));

   if num_cols2 >= 0 then
      EGA_DrawSpriteMaskFast2(
            src_seg, src_offs, src_skip, 
            draw_segment, dest_offs + dest_offs_offs1, dest_rewind, 
            num_cols2 + 1, num_rows + 1);

  { Draw sprite using OR operation }

   set_rotate(2, shift_amount);
  
   for p := 0 to 3 do begin
      EGA_SetPlanes((1 shl p)); 

      set_mask($ff shr shift_amount);

      src_offs := ofs(sprite.data^) + sprite.height * (src_column0 * 5 + p) + src_y0;

      if clipped = 2 then begin
         { Increase source column - could just add here }
         src_offs := ofs(sprite.data^) + sprite.height * ((src_column0 + 1) * 5 + p) + src_y0;
      end;
      
      if num_cols >= 0 then
         EGA_DrawSpriteMaskFast2(src_seg, src_offs, src_skip, 
            draw_segment, dest_offs, dest_rewind, 
            num_cols + 1, num_rows + 1);

      src_offs := ofs(sprite.data^) + sprite.height * (src_column0 * 5 + p) + src_y0;

      set_mask($ff shl (8-shift_amount));

      if num_cols2 >= 0 then
         EGA_DrawSpriteMaskFast2(
               src_seg, src_offs, src_skip, 
               draw_segment, dest_offs + dest_offs_offs1, dest_rewind, 
               num_cols2 + 1, num_rows + 1);

   end;
end;


procedure EGA_DrawSpriteFast(x, y: integer; var sprite: EGASprite);
var dummy: byte;  p, o, i: word;
   start_y, end_y: integer;
   start_row, end_row : integer;
   _y: integer;
   offs: integer;

   data_ptr, mask_ptr: byte_ptr;
   data, mask: byte;
   
   src_column0, src_column1: integer;
   dst_column0, dst_column1: integer;
   

   src_seg, src_offs: word;
   dest_offs: word;
   src_skip, dest_rewind: word;
   num_rows, num_cols: word;

begin
   
   src_column0 := 0;
   src_column1 := sprite.column_count - 1;
   dst_column0 := x;
   dst_column1 := x + src_column1;

  if (dst_column0 > 39) or (dst_column1 < 0) then Exit;
   
   { Clipping }

   if dst_column0 < 0 then begin
      inc(src_column0, -dst_column0);
      dst_column0 := 0;
   end;

   if dst_column1 > 39 then begin
      dec(src_column1, dst_column1 - 39);
      dst_column1 := 39;
   end;

   start_y := y;

   x := x * 8;
   
   start_y := y;
   end_y := y + (sprite.height - 1);
   
   if end_y < 0 then Exit;
   if start_y > 199 then Exit;

   if start_y < 0 then start_y := 0;   
   if end_y > 199 then end_y := 199;

   if end_y < start_y then Exit;
   
   start_row := start_y - y;
   end_row := end_y - y;

   num_rows := end_row - start_row;
   num_cols := src_column1 - src_column0 + 1;

   src_seg := seg(sprite.data^);
   src_offs := ofs(sprite.data^) + sprite.height * (src_column0 * sprite.num_channels + sprite.num_color_channels) + start_row;
   src_skip := sprite.height * (sprite.num_color_channels) + ((sprite.height - num_rows - 1));

   dest_offs := (dst_column0) + (start_y) * 40;
   dest_rewind := num_rows * 40 + (sprite.height - 1);
   dest_rewind := (num_rows + 1) * 40 - 1;
   {dest_rewind := 16 * 40 - 1;}

   Inc(num_rows);
   { Set AND operation }
   set_rotate(1, 0);
   set_mask($ff);
   EGA_SetPlanes(15);
   EGA_DrawSpriteMaskFast2(src_seg, src_offs, src_skip, draw_segment, dest_offs, dest_rewind, num_cols, num_rows);

   { Set OR operation }
   set_rotate(2, 0);
   for p := 0 to (sprite.num_color_channels - 1) do begin
      EGA_SetPlanes(((1 shl p) and sprite.planes_and) or sprite.planes_or);
      src_offs := ofs(sprite.data^) + sprite.height * (src_column0 * sprite.num_channels + p) + start_row;
      EGA_DrawSpriteMaskFast2(src_seg, src_offs, src_skip, draw_segment, dest_offs, dest_rewind, num_cols, num_rows);
   end;
   
end;


procedure EGA_ClearScreen;
begin
   EGA_SetPlanes(15);
   set_mask($FF);
   set_rotate(0, 0);
{ $0900 - light blue }
   asm
      { set-reset value }
      mov ax, $0900
      mov ax, $0000 
      out dx, ax
     
      { enable set/reset } 
      mov ax, $0f01
      out dx, ax
 
      mov ax, draw_segment
      mov es, ax
      mov di, 0

      mov cx, 4000
      mov ax, $ffff
      rep stosw
     
      { disable set/reset } 
      mov ax, $0001
      out dx, ax
   end;
end;


procedure EGA_SetDrawPage(p: word);
begin
   draw_segment := $A000 + $1F4 * p;
end;

procedure EGA_ShowPage(p: word);
begin
   if p = 0 then begin
      asm
      { start address high }
      mov dx, $3d4
      mov al, $0C
      mov ah, $0
      out dx, ax

      { start address low}
      mov dx, $3d4
      mov al, $0D
      mov ah, $0
      out dx, ax
   end;  
   end else begin
      asm
      { start address high }
      mov dx, $3d4
      mov al, $0C
      mov ah, $1F
      out dx, ax

      { start address low}
      mov dx, $3d4
      mov al, $0D
      mov ah, $40
      out dx, ax
   end;  
   end;

end;

function EGA_LoadSprite(path: string; var out: EGASprite): boolean;
var f: file;
begin

  EGA_LoadSprite := false;

  assign(f, path);
  reset(f, 1);

  blockread(f, out.column_count, 2);
  blockread(f, out.height, 2);

  out.width := out.column_count * 8;
 
  out.num_color_channels := 4;
  out.num_channels := 5;
  out.planes_and := $ff;
  out.planes_or := $00;
  { writeln(out.column_count, ' ', out.height); }
  
  GetMem(out.data, out.column_count * out.height * 5);
  blockread(f, out.data^, out.column_count * out.height * 5); 

  System.Close(f);

  EGA_LoadSprite := true;

end;
begin
end.
