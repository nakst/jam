#include "ds.h"

char count[1000][1000];

void Part1() {
	FILE *f = fopen("in5.txt", "rb");

	while (true) {
		int x1, y1, x2, y2;

		if (fscanf(f, "%d,%d -> %d,%d\n", &x1, &y1, &x2, &y2) == -1) {
			break;
		}

		if (x1 == x2) {
			MAKELT(y1, y2);
			for (int y = y1; y <= y2; y++) count[x1][y]++;
		} else if (y1 == y2) {
			MAKELT(x1, x2);
			for (int x = x1; x <= x2; x++) count[x][y1]++;
		}
	}

	int c = 0;

	for (int i = 0; i < 1000; i++) {
		for (int j = 0; j < 1000; j++) {
			if (count[i][j] >= 2) {
				c++;
			}
		}
	}

	printf("part 1: %d\n", c);

	fclose(f);
}

void Part2() {
	FILE *f = fopen("in5.txt", "rb");

	while (true) {
		int x1, y1, x2, y2;

		if (fscanf(f, "%d,%d -> %d,%d\n", &x1, &y1, &x2, &y2) == -1) {
			break;
		}

		if (x1 == x2) {
			MAKELT(y1, y2);
			for (int y = y1; y <= y2; y++) count[x1][y]++;
		} else if (y1 == y2) {
			MAKELT(x1, x2);
			for (int x = x1; x <= x2; x++) count[x][y1]++;
		} else if (x1 <= x2) {
			if (y1 <= y2) { for (int x = x1; x <= x2; x++) count[x][y1 + (x - x1)]++; }
			else          { for (int x = x1; x <= x2; x++) count[x][y1 - (x - x1)]++; }
		} else {
			if (y1 <= y2) { for (int x = x1; x >= x2; x--) count[x][y1 + (x1 - x)]++; }
			else          { for (int x = x1; x >= x2; x--) count[x][y1 - (x1 - x)]++; }
		}
	}

	int c = 0;

	for (int i = 0; i < 1000; i++) {
		for (int j = 0; j < 1000; j++) {
			if (count[i][j] >= 2) {
				c++;
			}
		}
	}

	printf("part 2: %d\n", c);

	fclose(f);
}

int main() {
	Part1();
	memset(count, 0, sizeof(count));
	Part2();
}
