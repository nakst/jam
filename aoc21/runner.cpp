#include "ds.h"
#include <time.h>

void RunTimed(const char *call, const char *name) {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	uint64_t start = (uint64_t) time.tv_sec * 1000000000 + time.tv_nsec;
	system(call);
	clock_gettime(CLOCK_MONOTONIC, &time);
	uint64_t end = (uint64_t) time.tv_sec * 1000000000 + time.tv_nsec;
	uint64_t timeMs = (end - start) / 1000000;
	printf("%s: %lu ms\n", name, timeMs);
}

int main() {
	char call[128], name[128];

	system("mkdir bin");

	for (int i = 1; i <= 25; i++) {
		printf("compiling day %d...\n", i);
		sprintf(call, "gcc -o bin/day%d_gcc_o0 -O0 code%d.cpp", i, i);
		system(call);
		sprintf(call, "gcc -o bin/day%d_gcc_o1 -O1 code%d.cpp", i, i);
		system(call);
		sprintf(call, "gcc -o bin/day%d_gcc_o2 -O2 code%d.cpp", i, i);
		system(call);
		sprintf(call, "gcc -o bin/day%d_gcc_o3 -O3 code%d.cpp", i, i);
		system(call);
		sprintf(call, "clang -o bin/day%d_clang_o0 -O0 code%d.cpp", i, i);
		system(call);
		sprintf(call, "clang -o bin/day%d_clang_o1 -O1 code%d.cpp", i, i);
		system(call);
		sprintf(call, "clang -o bin/day%d_clang_o2 -O2 code%d.cpp", i, i);
		system(call);
		sprintf(call, "clang -o bin/day%d_clang_o3 -O3 code%d.cpp", i, i);
		system(call);
	}

	for (int i = 1; i <= 25; i++) { sprintf(call, "bin/day%d_gcc_o0 > /dev/null", i); sprintf(name, "day %d (gcc -O0)", i); RunTimed(call, name); }
	for (int i = 1; i <= 25; i++) { sprintf(call, "bin/day%d_gcc_o1 > /dev/null", i); sprintf(name, "day %d (gcc -O1)", i); RunTimed(call, name); }
	for (int i = 1; i <= 25; i++) { sprintf(call, "bin/day%d_gcc_o2 > /dev/null", i); sprintf(name, "day %d (gcc -O2)", i); RunTimed(call, name); }
	for (int i = 1; i <= 25; i++) { sprintf(call, "bin/day%d_gcc_o3 > /dev/null", i); sprintf(name, "day %d (gcc -O3)", i); RunTimed(call, name); }
	for (int i = 1; i <= 25; i++) { sprintf(call, "bin/day%d_clang_o0 > /dev/null", i); sprintf(name, "day %d (clang -O0)", i); RunTimed(call, name); }
	for (int i = 1; i <= 25; i++) { sprintf(call, "bin/day%d_clang_o1 > /dev/null", i); sprintf(name, "day %d (clang -O1)", i); RunTimed(call, name); }
	for (int i = 1; i <= 25; i++) { sprintf(call, "bin/day%d_clang_o2 > /dev/null", i); sprintf(name, "day %d (clang -O2)", i); RunTimed(call, name); }
	for (int i = 1; i <= 25; i++) { sprintf(call, "bin/day%d_clang_o3 > /dev/null", i); sprintf(name, "day %d (clang -O3)", i); RunTimed(call, name); }

	system("rm -r bin");

	return 0;
}
