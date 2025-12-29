@config -set cpu cycles 360000
@del app.exe
@del *.tpu
@del *.obj

tasm /ml /l test.asm
@if errorlevel 1 goto DONE
bpc app.pas
@config -set cpu cycles 2400
app.exe
:DONE
@config -set cpu cycles 3000

