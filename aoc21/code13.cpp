#include "ds.h"

int main() {
#define SIZE (2000)
	bool dots[SIZE][SIZE] = {};

	FILE *f = fopen("in13.txt", "rb");

	while (true) {
		int x, y;

		if (fscanf(f, "%d,%d\n", &x, &y) != 2) {
			break;
		}

		dots[x][y] = true;
	}

	bool first = true;

	while (true) {
		char buffer[32];

		if (fscanf(f, "fold along %s\n", buffer) == -1) {
			break;
		}

		bool xAxis = buffer[0] == 'x';
		int offset = atoi(buffer + 2);

		for (int x = 0; x < SIZE; x++) {
			for (int y = 0; y < SIZE; y++) {
				if (!dots[x][y]) continue;
				int tx = x, ty = y;
				if ( xAxis && x > offset) tx = 2 * offset - x;
				if (!xAxis && y > offset) ty = 2 * offset - y;
				dots[x][y] = false;
				dots[tx][ty] = true;
			}
		}

		if (first) {
			int count = 0;

			for (int i = 0; i < SIZE; i++) {
				for (int j = 0; j < SIZE; j++) {
					if (dots[i][j]) {
						count++;
					}
				}
			}

			printf("part 1: %d\n", count);
			first = false;
		}
	}

	fclose(f);

	int fi = SIZE, ti = 0, fj = SIZE, tj = 0;

	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (dots[i][j]) {
				MINIMIZE(fi, i);
				MINIMIZE(fj, j);
				MAXIMIZE(ti, i);
				MAXIMIZE(tj, j);
			}
		}
	}

	printf("part 2:\n");

	for (int j = fj; j <= tj; j++) {
		for (int i = fi; i <= ti; i++) {
			printf("%c", dots[i][j] ? 'O' : ' ');
		}

		printf("\n");
	}
}
