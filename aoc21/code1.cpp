#include "ds.h"

void Part1() {
	FILE *f = fopen("in1.txt", "rb");

	int previous = INT_MAX, current, count = 0;

	while (true) {
		int matches = fscanf(f, "%d\n", &current);

		if (matches == -1) {
			break;
		}

		if (current > previous) {
			count++;
		}

		previous = current;
	}

	fclose(f);

	printf("part 1: %d\n", count);
}

void Part2() {
	Array<int> items = {};

	FILE *f = fopen("in1.txt", "rb");

	while (true) {
		int current;

		if (fscanf(f, "%d\n", &current) == -1) {
			break;
		}

		items.Add(current);
	}

	fclose(f);

	int previous = INT_MAX, count = 0;

	for (int i = 0; i < items.Length() - 2; i++) {
		int current = items[i + 0] + items[i + 1] + items[i + 2];

		if (current > previous) {
			count++;
		}

		previous = current;
	}

	items.Free();

	printf("part 2: %d\n", count);
}

int main() {
	Part1();
	Part2();
}
