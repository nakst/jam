cd aoc21
gcc -o runner runner.cpp
./runner "gcc -O0 -o"
./runner "gcc -O1 -o"
./runner "gcc -O2 -o"
./runner "gcc -O3 -o"
./runner "clang -O0 -o"
./runner "clang -O1 -o"
./runner "clang -O2 -o"
./runner "clang -O3 -o"
