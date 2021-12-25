#include "ds.h"

#define N (100)

char heightMap[N][N + 1] = {};

void Part1() {
	int sum = 0;

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			bool isLowPoint = true;

			if (i !=     0 && heightMap[i - 1][j] <= heightMap[i][j]) isLowPoint = false;
			if (i != N - 1 && heightMap[i + 1][j] <= heightMap[i][j]) isLowPoint = false;
			if (j !=     0 && heightMap[i][j - 1] <= heightMap[i][j]) isLowPoint = false;
			if (j != N - 1 && heightMap[i][j + 1] <= heightMap[i][j]) isLowPoint = false;

			if (isLowPoint) {
				sum += heightMap[i][j] - '0' + 1;
			}
		}
	}

	printf("part 1: %d\n", sum);
}

int FloodFill(int i, int j) {
	if (heightMap[i][j] >= '0' && heightMap[i][j] < '9') {
		heightMap[i][j] = 0;
		int size = 1;
		if (i !=     0) size += FloodFill(i - 1, j);
		if (i != N - 1) size += FloodFill(i + 1, j);
		if (j !=     0) size += FloodFill(i, j - 1);
		if (j != N - 1) size += FloodFill(i, j + 1);
		return size;
	} else {
		return 0;
	}
}

void Part2() {
	Array<int> basinSizes = {};

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			basinSizes.Add(FloodFill(i, j));
		}
	}

	qsort(basinSizes.array, basinSizes.Length(), sizeof(int), CompareIntegersDescending);

	printf("part 2: %d\n", basinSizes[0] * basinSizes[1] * basinSizes[2]);

	basinSizes.Free();
}

int main() {
	FILE *f = fopen("in9.txt", "rb");

	for (int i = 0; i < N; i++) {
		fscanf(f, "%s\n", heightMap[i]);
	}

	fclose(f);

	Part1();
	Part2();
}
