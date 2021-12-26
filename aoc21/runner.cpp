#include "ds.h"

#ifdef WIN32
#include <windows.h>
#else
#include <time.h>
#endif

int main(int argc, char **argv) {
	char call[128];

	for (int i = 1; i <= 25; i++) {
#ifdef WIN32
		sprintf(call, "%s code%d.cpp", argv[1], i, i); 
		system(call);
		sprintf(call, "code%d > NUL", i); 
		uint64_t start, end, frequency;
		QueryPerformanceFrequency((LARGE_INTEGER *) &frequency);
		QueryPerformanceCounter((LARGE_INTEGER *) &start);
#else
		sprintf(call, "%s -o day%d code%d.cpp", argv[1], i, i); 
		system(call);
		sprintf(call, "./day%d > /dev/null", i); 
		struct timespec time;
		clock_gettime(CLOCK_MONOTONIC, &time);
		uint64_t start = (uint64_t) time.tv_sec * 1000000000 + time.tv_nsec;
#endif
		system(call);
#ifdef WIN32
		QueryPerformanceCounter((LARGE_INTEGER *) &end);
		uint32_t timeMs = (end - start) / (frequency / 1000);
#else
		clock_gettime(CLOCK_MONOTONIC, &time);
		uint64_t end = (uint64_t) time.tv_sec * 1000000000 + time.tv_nsec;
		uint32_t timeMs = (end - start) / 1000000;
#endif
		fprintf(stderr, "day %d (%s): %u ms\n", i, argv[1], timeMs);
#ifdef WIN32
		sprintf(call, "del code%d", i); 
#else
		sprintf(call, "rm day%d", i); 
#endif
		system(call);
	}

	return 0;
}
