set -e
mkdir -p bin embed audio
# ../integrate/bin/shrink_png embed/*.png
# ffmpeg -i track1.wav -b:a 64k track1.opus
gcc -o bin/embed_tool embed.c
bin/embed_tool
# clang -o bin/lb -lX11 luigi_backend.c -fsanitize=address -g -D UI_LINUX
clang --target=wasm32 -c -O2 -flto -DDEFMEM -nostdlib -Wall -Wextra -Wno-unused-parameter -o bin/main.o main.c
wasm-ld --no-entry --export=__heap_base --export=GenerateFrame --export=Initialise --export=HandleEvent --allow-undefined --import-memory --initial-memory=16777216 -o bin/game.wasm bin/main.o
rm -f bin/game.zip
zip -q bin/game.zip index.html
zip -q bin/game.zip bin/game.wasm
zip -q bin/game.zip audio/*
