unit fixed;
interface

type 
   fix16 = integer;

function i2f(x: integer): fix16;
function f2i(x:fix16): integer;
function fix16Div(val1, val2: fix16): fix16;

implementation


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
   fix16Div := (sal(val1, 2) div sar(val2, 2));
end;
{ Pixel range is -2048 to 2047 }

begin
end.