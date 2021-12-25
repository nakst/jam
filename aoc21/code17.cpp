#include "ds.h"

int main() {
	int fromX, toX, fromY, toY;
	FILE *f = fopen("in17.txt", "rb");
	fscanf(f, "target area: x=%d..%d, y=%d..%d\n", &fromX, &toX, &fromY, &toY);
	fclose(f);

	int maximumHeight = 0;
	int count = 0;

	for (int dy = -1000; dy <= 1000; dy++) {
		for (int dx = 0; dx <= 1000; dx++) {
			int x = 0, y = 0, cdx = dx, cdy = dy;

			while (x <= toX && y >= fromY) {
				if (x >= fromX && y <= toY) {
					MAXIMIZE(maximumHeight, dy * (dy + 1) / 2);
					count++;
					break;
				}

				x += cdx--;
				y += cdy--;
				if (cdx == -1) cdx = 0;
			}
		}
	}

	printf("part 1: %d\n", maximumHeight);
	printf("part 2: %d\n", count);
}
