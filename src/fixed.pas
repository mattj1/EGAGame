unit fixed;
interface

type 
   fix16 = integer;

  TVec2 = record
    x, y: fix16;
  end;

  TBounds = record
    min: TVec2;
    max: TVec2;
  end;

  LinePoint = record
    x, y: fix16;
  end;

  TLine = record
    dx, dy, x, y, sx, sy, err, x0, y0, x1, y1: fix16;
    didReturnFirst: boolean;
  end;
procedure Line_Init(var line: TLine; x0, y0, x1, y1: fix16);
procedure getPoint(var line: TLine; var pt: LinePoint);


function i2f(x: integer): fix16;
function f2i(x:fix16): integer;
function fix16Div(val1, val2: fix16): fix16;
function fix16Mul(val1, val2: fix16): fix16;
function fix16Sqrt(val: fix16): fix16;

procedure BoundsFromSize(origin, mins, maxs: TVec2; var out: TBounds);
procedure BoundsCenter(bounds: TBounds; var out: TVec2);
function BoundsIntersectsBounds(b0, b1: TBounds): boolean;
procedure UnionBounds(var result: TBounds; a, b: TBounds);
procedure OffsetBounds(const bounds: TBounds; offset: TVec2; var out: TBounds);

function BoundsContainsXY(a: TBounds; x, y: fix16): boolean;

implementation

uses gamedata;

{$ifdef PLATFORM_DOS}
function sal(x: fix16; n: byte): fix16; assembler;
asm
   mov ax, x
   mov cl, n
   sal ax, cl
end;
{$else}
function sal(x: fix16; n: byte): fix16;
begin
  sal := x * (1 shl n);
end;
{$endif}

{$ifdef PLATFORM_DOS}
function sar(x: fix16; n: byte): fix16; assembler;
asm
   mov ax, x
   mov cl, n
   sar ax, cl
end;
{$else}
function sar(x: fix16; n: byte): fix16;
begin
  sar := x div (1 shl n);
end;
{$endif}

{$ifdef PLATFORM_DOS}
function i2f(x: integer): fix16; assembler;
asm
   mov ax, x
   mov cl, 4
   sal ax, cl
end;
{$else}
function i2f(x: fix16): fix16;
begin
  i2f := x * 16;
end;
{$endif}

{$ifdef PLATFORM_DOS}
function f2i(x:fix16): integer; assembler;
asm
   mov ax, x
   mov cl, 4
   sar ax, cl
end;
{$else}
function f2i(x:fix16): integer;
begin
  f2i := x div 16;
end;
{$endif}

function fix16Div(val1, val2: fix16): fix16;
begin
   fix16Div := sal(val1, 2) div sar(val2, 2);
end;

function fix16Mul(val1, val2: fix16): fix16;
begin
   fix16Mul := (sar(val1, 2) * sar(val2, 2));
end;

{ TODO: Mul by frac }

{ Pixel range is -2048 to 2047 }

function fix16Sqrt(val: fix16): fix16;
begin
   fix16Sqrt := assets.sqrt^[val];
end;


procedure Line_Init(var line: TLine; x0, y0, x1, y1: fix16);
begin
  line.dx := Abs(x1 - x0);
  line.dy := Abs(y1 - y0);
  line.x := x0;
  line.y := y0;
  line.x0 := x0;
  line.y0 := y0;
  line.x1 := x1;
  line.y1 := y1;
  line.sx := -1;
  if x0 <= x1 then line.sx := 1;
  line.sy := -1;
  if y0 <= y1 then line.sy := 1;
  line.err := 0;
  line.didReturnFirst := False;
end;

procedure getPoint(var line: TLine; var pt: LinePoint);
begin
  if not line.didReturnFirst then
  begin
    pt.x := line.x0;
    pt.y := line.y0;
    line.didReturnFirst := True;
    Exit;
  end;
  if line.dx > line.dy then
  begin
    line.err := line.err - line.dy;
    if line.err < 0 then
    begin
      line.y := line.y + line.sy;
      line.err := line.err + line.dx;
    end;
    line.x := line.x + line.sx;
  end
  else
  begin
    line.err := line.err - line.dx;
    if line.err < 0 then
    begin
      line.x := line.x + line.sx;
      line.err := line.err + line.dy;
    end;
    line.y := line.y + line.sy;
  end;
  pt.x := line.x;
  pt.y := line.y;
end;

procedure BoundsFromSize(origin, mins, maxs: TVec2; var out: TBounds);
begin
  out.min.x := origin.x + mins.x;
  out.min.y := origin.y + mins.y;
  out.max.x := origin.x + maxs.x;
  out.max.y := origin.y + maxs.y;
end;

procedure BoundsCenter(bounds: TBounds; var out: TVec2);
begin
  out.x := (bounds.min.x + bounds.max.x) div 2;
  out.y := (bounds.min.y + bounds.max.y) div 2;
end;

function BoundsIntersectsBounds(b0, b1: TBounds): boolean;
begin
  if (b0.max.x <= b1.min.x) or
     (b0.max.y <= b1.min.y) or
     (b0.min.x >= b1.max.x) or
     (b0.min.y >= b1.max.y) then
  begin
    BoundsIntersectsBounds := False;
  end
  else
  begin
    BoundsIntersectsBounds := True;
  end;
end;

procedure UnionBounds(var result: TBounds; a, b: TBounds);
begin
  if a.min.x < b.min.x then
    result.min.x := a.min.x
  else
    result.min.x := b.min.x;
    
  if a.min.y < b.min.y then
    result.min.y := a.min.y
  else
    result.min.y := b.min.y;
    
  if a.max.x > b.max.x then
    result.max.x := a.max.x
  else
    result.max.x := b.max.x;
    
  if a.max.y > b.max.y then
    result.max.y := a.max.y
  else
    result.max.y := b.max.y;
end;

procedure OffsetBounds(const bounds: TBounds; offset: TVec2; var out: TBounds);
begin
  out.min.x := bounds.min.x + offset.x;
  out.min.y := bounds.min.y + offset.y;
  out.max.x := bounds.max.x + offset.x;
  out.max.y := bounds.max.y + offset.y;
end;

function BoundsContainsXY(a: TBounds; x, y: fix16): boolean;
begin
  BoundsContainsXY := false;

  if (x > a.max.x) or (x < a.min.x) or (y > a.max.y) or (y < a.min.y) then Exit;

  BoundsCOntainsXY := true;
end;

begin
end.
