#include "ds.h"

void Part1() {
	FILE *f = fopen("in2.txt", "rb");

	int horizontalPosition = 0;
	int depth = 0;

	while (true) {
		char string[64];
		int units;

		if (fscanf(f, "%s %d\n", string, &units) == -1) {
			break;
		}

		// printf("[%s] %d\n", string, units);

		if (0 == strcmp(string, "forward")) {
			horizontalPosition += units;
		} else if (0 == strcmp(string, "down")) {
			depth += units;
		} else if (0 == strcmp(string, "up")) {
			depth -= units;
		}
	}

	fclose(f);

	printf("part 1: %d\n", horizontalPosition * depth);
}

void Part2() {
	FILE *f = fopen("in2.txt", "rb");

	int64_t horizontalPosition = 0;
	int64_t depth = 0;
	int64_t aim = 0;

	while (true) {
		char string[64];
		int units;

		if (fscanf(f, "%s %d\n", string, &units) == -1) {
			break;
		}

		// printf("[%s] %d\n", string, units);

		if (0 == strcmp(string, "forward")) {
			horizontalPosition += units;
			depth += units * aim;
		} else if (0 == strcmp(string, "down")) {
			aim += units;
		} else if (0 == strcmp(string, "up")) {
			aim -= units;
		}
	}

	fclose(f);

	printf("part 2: %ld\n", horizontalPosition * depth);
}

int main() {
	Part1();
	Part2();
}
