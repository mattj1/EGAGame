@config -set cpu cycles 360000
@del src\app.exe
@del src\*.tpu
@del src\*.obj

tasm /ml /l src\test.asm
@if errorlevel 1 goto DONE
bpc src\app.pas
@config -set cpu cycles 2400
src\app.exe
:DONE
@config -set cpu cycles 3000

