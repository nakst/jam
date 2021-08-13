@echo off
if not exist embed.exe clang -o embed.exe embed.c -Wno-deprecated-declarations -g
embed
clang --target=wasm32 -c -Os -flto -nostdlib -Wall -Wextra -o main.o main.c
wasm-ld --no-entry --export=__heap_base --export=GenerateFrame --export=Initialise --export=HandleEvent --allow-undefined --import-memory --initial-memory=16777216 -o game.wasm main.o
call :printsize game.wasm
goto :eof
:printsize
echo size: %~z1 
del main.o
del embed.h
del embed_def.h
rem ffmpeg -i track1.wav -b:a 64k track1.opus
