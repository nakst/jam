#include "ds.h"

char grid[10][11];
int64_t flashCount;

void Flash(int i, int j) {
	grid[i][j] = 'F';
	flashCount++;

	for (int di = -1; di <= 1; di++) {
		for (int dj = -1; dj <= 1; dj++) {
			if (i + di >= 0 && i + di < 10 && j + dj >= 0 && j + dj < 10 && (di || dj)) {
				if (grid[i + di][j + dj] >= '0' && grid[i + di][j + dj] <= '9') {
					grid[i + di][j + dj]++;

					if (grid[i + di][j + dj] == '9' + 1) {
						Flash(i + di, j + dj);
					}
				}
			}
		}
	}
}

void Part1() {
	FILE *f = fopen("in11.txt", "rb");

	for (int i = 0; i < 10; i++) {
		fscanf(f, "%s\n", grid[i]);
	}

	fclose(f);

	for (int step = 0; step < 100; step++) {
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				grid[i][j]++;
			}
		}

		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				if (grid[i][j] == '9' + 1) {
					Flash(i, j);
				}
			}
		}

		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				if (grid[i][j] == 'F') {
					grid[i][j] = '0';
				}
			}
		}
	}

	printf("part 1: %ld\n", flashCount);
}

void Part2() {
	FILE *f = fopen("in11.txt", "rb");

	for (int i = 0; i < 10; i++) {
		fscanf(f, "%s\n", grid[i]);
	}

	fclose(f);

	int64_t step = 0;

	while (true) {
		int64_t initialFlashCount = flashCount;

		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				grid[i][j]++;
			}
		}

		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				if (grid[i][j] == '9' + 1) {
					Flash(i, j);
				}
			}
		}

		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				if (grid[i][j] == 'F') {
					grid[i][j] = '0';
				}
			}
		}

		step++;

		int64_t finalFlashCount = flashCount;

		if (finalFlashCount - initialFlashCount == 100) {
			break;
		}
	}

	printf("part 2: %ld\n", step);
}

int main() {
	Part1();
	Part2();
}
