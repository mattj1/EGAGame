unit fixed;
interface

type 
   fix16 = integer;

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

implementation

uses gamedata;

function sal(x: fix16; n: byte): fix16; assembler;
asm
   mov ax, x
   mov cl, n
   sal ax, cl
end;

function sar(x: fix16; n: byte): fix16; assembler;
asm
   mov ax, x
   mov cl, n
   sar ax, cl
end;

function i2f(x: integer): fix16; assembler;
asm
   mov ax, x
   mov cl, 4
   sal ax, cl
end;

function f2i(x:fix16): integer; assembler;
asm
   mov ax, x
   mov cl, 4
   sar ax, cl
end;

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
   fix16Sqrt := res.sqrt^[val];
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
begin
end.