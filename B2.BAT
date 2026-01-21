:RESTART
@config -set cpu cycles 360000
132x60
del app.exe
tasm /mx src\test.asm
@if errorlevel 1 goto DONE
c:\tc\bin\tcc -mm -Iengine\src -Isrc\ -DPLATFORM_DOS src\*.c src\res\res.c test.obj
@REM Why doesn't -mc work?
@if errorlevel 1 goto DONE
@config -set cpu cycles 2400
app.exe
@config -set cpu cycles 3000
@echo Press a key to rebuild...
@PAUSE
@goto RESTART

:DONE
@config -set cpu cycles 3000

