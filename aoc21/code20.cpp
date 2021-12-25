#include "ds.h"

#define N (100)

char lines[1000][1000];
char lookup[1000];

void Part1() {
	FILE *f = fopen("in20.txt", "rb");
	fscanf(f, "%s\n", lookup);
	assert(strlen(lookup) == 512);
	memset(lines, '.', sizeof(lines));
	fscanf(f, "\n");

	for (int y = 100; y < 100 + N; y++) {
		for (int x = 100; x < 100 + N; x++) {
			lines[y][x] = fgetc(f);
		}

		assert(fgetc(f) == '\n');
	}

	fclose(f);

	for (int i = 0; i < 2; i++) {
		char replacement[1000][1000];

		if (lookup[0] == '#') {
			for (int m = 0; m < 1000; m++) {
				lines[m][0] = (i % 2) ? '#' : '.';
				lines[0][m] = (i % 2) ? '#' : '.';
				lines[m][1000 - 1] = (i % 2) ? '#' : '.';
				lines[1000 - 1][m] = (i % 2) ? '#' : '.';
			}
		}

		for (int y = 1; y < 1000 - 1; y++) {
			for (int x = 1; x < 1000 - 1; x++) {
				int k = 0;
				if (lines[y - 1][x - 1] == '#') k |= 1 << 8;
				if (lines[y - 1][x + 0] == '#') k |= 1 << 7;
				if (lines[y - 1][x + 1] == '#') k |= 1 << 6;
				if (lines[y + 0][x - 1] == '#') k |= 1 << 5;
				if (lines[y + 0][x + 0] == '#') k |= 1 << 4;
				if (lines[y + 0][x + 1] == '#') k |= 1 << 3;
				if (lines[y + 1][x - 1] == '#') k |= 1 << 2;
				if (lines[y + 1][x + 0] == '#') k |= 1 << 1;
				if (lines[y + 1][x + 1] == '#') k |= 1 << 0;
				replacement[y][x] = lookup[k];
			}
		}

		memcpy(lines, replacement, sizeof(replacement));
	}

	int lit = 0;

	for (int y = 1; y < 1000 - 1; y++) {
		for (int x = 1; x < 1000 - 1; x++) {
			if (lines[y][x] == '#') {
				lit++;
			}
		}
	}

	printf("part 1: %d\n", lit);
}

void Part2() {
	FILE *f = fopen("in20.txt", "rb");
	fscanf(f, "%s\n", lookup);
	assert(strlen(lookup) == 512);
	memset(lines, '.', sizeof(lines));
	fscanf(f, "\n");

	for (int y = 100; y < 100 + N; y++) {
		for (int x = 100; x < 100 + N; x++) {
			lines[y][x] = fgetc(f);
		}

		assert(fgetc(f) == '\n');
	}

	fclose(f);

	for (int i = 0; i < 50; i++) {
		char replacement[1000][1000];

		if (lookup[0] == '#') {
			for (int m = 0; m < 1000; m++) {
				lines[m][0] = (i % 2) ? '#' : '.';
				lines[0][m] = (i % 2) ? '#' : '.';
				lines[m][1000 - 1] = (i % 2) ? '#' : '.';
				lines[1000 - 1][m] = (i % 2) ? '#' : '.';
			}
		}

		for (int y = 1; y < 1000 - 1; y++) {
			for (int x = 1; x < 1000 - 1; x++) {
				int k = 0;
				if (lines[y - 1][x - 1] == '#') k |= 1 << 8;
				if (lines[y - 1][x + 0] == '#') k |= 1 << 7;
				if (lines[y - 1][x + 1] == '#') k |= 1 << 6;
				if (lines[y + 0][x - 1] == '#') k |= 1 << 5;
				if (lines[y + 0][x + 0] == '#') k |= 1 << 4;
				if (lines[y + 0][x + 1] == '#') k |= 1 << 3;
				if (lines[y + 1][x - 1] == '#') k |= 1 << 2;
				if (lines[y + 1][x + 0] == '#') k |= 1 << 1;
				if (lines[y + 1][x + 1] == '#') k |= 1 << 0;
				replacement[y][x] = lookup[k];
			}
		}

		memcpy(lines, replacement, sizeof(replacement));
	}

	int lit = 0;

	for (int y = 1; y < 1000 - 1; y++) {
		for (int x = 1; x < 1000 - 1; x++) {
			if (lines[y][x] == '#') {
				lit++;
			}
		}
	}

	printf("part 2: %d\n", lit);
}

int main() {
	Part1();
	Part2();
}
