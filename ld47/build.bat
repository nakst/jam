@echo off

cd bin

cl ..\embed.cpp && embed

cl /nologo /GS- /Gs9999999 /Gm- /EHa- /GF /Gy /GA /GR- /O1 /Os /Fe:ld47.exe ..\main.cpp ^
	/link /NODEFAULTLIB kernel32.lib user32.lib opengl32.lib gdi32.lib ole32.lib windowscodecs.lib shlwapi.lib Winmm.lib shell32.lib advapi32.lib ^
	/subsystem:windows /OPT:REF /OPT:ICF /STACK:0x100000,0x100000 
..\..\rcedit  ld47.exe  --set-icon ..\icon.ico 
..\..\upx ld47.exe
copy ld47.exe ld47_small.exe

cl /nologo /GS- /Gs9999999 /Gm- /EHa- /GF /Gy /GA /GR- /Zi /Fe:ld47.exe ..\main.cpp ^
	/link /NODEFAULTLIB kernel32.lib user32.lib opengl32.lib gdi32.lib ole32.lib windowscodecs.lib shlwapi.lib Winmm.lib shell32.lib advapi32.lib ^
	/subsystem:windows /OPT:REF /OPT:ICF /STACK:0x100000,0x100000 ^
  	&& ld47
	

