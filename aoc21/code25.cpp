#include "ds.h"

#define ROWS (137)
#define COLUMNS (139)

void Part1() {
	FILE *f = fopen("in25.txt", "rb");

	char map[ROWS][COLUMNS + 1];

	for (int i = 0; i < ROWS; i++) {
		fscanf(f, "%s\n", map[i]);
	}

	fclose(f);

	int stepIndex = 0;

	while (true) {
		bool move[ROWS][COLUMNS] = {};
		bool anyMoved = false;
		
		for (int i = 0; i < ROWS; i++) {
			for (int j = 0; j < COLUMNS; j++) {
				if (map[i][j] == '>' && map[i][(j + 1) % COLUMNS] == '.') {
					move[i][j] = true;
					anyMoved = true;
				}
			}
		}

		for (int i = 0; i < ROWS; i++) {
			for (int j = 0; j < COLUMNS; j++) {
				if (move[i][j]) {
					map[i][j] = '.';
					map[i][(j + 1) % COLUMNS] = '>';
				}
			}
		}

		memset(move, 0, sizeof(move));
		
		for (int i = 0; i < ROWS; i++) {
			for (int j = 0; j < COLUMNS; j++) {
				if (map[i][j] == 'v' && map[(i + 1) % ROWS][j] == '.') {
					move[i][j] = true;
					anyMoved = true;
				}
			}
		}

		for (int i = 0; i < ROWS; i++) {
			for (int j = 0; j < COLUMNS; j++) {
				if (move[i][j]) {
					map[i][j] = '.';
					map[(i + 1) % ROWS][j] = 'v';
				}
			}
		}

		memset(move, 0, sizeof(move));

		stepIndex++;

		if (!anyMoved) {
			break;
		}
	}

	printf("part 1: %d\n", stepIndex);
}

void Part2() {
	// There is no part 2!
}

int main() {
	Part1();
	Part2();
}
