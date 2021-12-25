#include "ds.h"

void Part1() {
	FILE *f = fopen("in6.txt", "rb");

	Array<int> fish = {};

	while (true) {
		int n = 0;

		if (fscanf(f, "%d,", &n) == -1) {
			break;
		}

		fish.Add(n);
	}

	for (int i = 0; i < 80; i++) {
		int originalLength = fish.Length();

		for (int i = 0; i < originalLength; i++) {
			if (fish[i]) {
				fish[i]--;
			} else {
				fish[i] = 6;
				fish.Add(8);
			}
		}
	}

	printf("part 1: %d\n", fish.Length());

	fish.Free();

	fclose(f);
}

void Part2() {
	FILE *f = fopen("in6.txt", "rb");

	int64_t countPerTimerValue[9] = {};

	while (true) {
		int n = 0;

		if (fscanf(f, "%d,", &n) == -1) {
			break;
		}

		countPerTimerValue[n]++;
	}

	for (int i = 0; i < 256; i++) {
		int64_t newCounts[9] = {};

		newCounts[6] += countPerTimerValue[0];
		newCounts[8] += countPerTimerValue[0];

		for (int i = 0; i < 8; i++) {
			newCounts[i] += countPerTimerValue[i + 1];
		}

		memcpy(countPerTimerValue, newCounts, sizeof(newCounts));
	}

	int64_t total = 0;

	for (int i = 0; i < 9; i++) {
		total += countPerTimerValue[i];
	}

	printf("part 2: %ld\n", total);

	fclose(f);
}

int main() {
	Part1();
	Part2();
}
