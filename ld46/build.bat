@echo off
embed
cl /nologo /GS- /Gs9999999 /Gm- /EHa- /GF /Gy /GA /GR- /O1 /Os /Fe:ld46.exe ..\main.cpp ^
	/link /NODEFAULTLIB kernel32.lib user32.lib opengl32.lib gdi32.lib ole32.lib windowscodecs.lib shlwapi.lib Winmm.lib shell32.lib advapi32.lib ^
	/subsystem:windows /OPT:REF /OPT:ICF /STACK:0x100000,0x100000 ^
	&& rcedit.exe /I ld46.exe ..\icon.ico ^
  	&& ld46