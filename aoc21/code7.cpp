#include "ds.h"

void Part1() {
	FILE *f = fopen("in7.txt", "rb");

	Array<int64_t> positions = {};
	int64_t minimum = INT_MAX;
	int64_t maximum = INT_MIN;

	while (true) {
		int n;

		if (fscanf(f, "%d,", &n) == -1) {
			break;
		}

		positions.Add(n);

		MINIMIZE(minimum, n);
		MAXIMIZE(maximum, n);
	}

	int64_t minimumFuel = INT_MAX;

	for (int target = minimum; target <= maximum; target++) {
		int64_t fuel = 0;

		for (int64_t i = 0; i < positions.Length(); i++) {
			int64_t x = positions[i] > target ? positions[i] - target : target - positions[i];
			fuel += x;
		}

		MINIMIZE(minimumFuel, fuel);
	}

	printf("part 1: %ld\n", minimumFuel);

	fclose(f);
	positions.Free();
}

void Part2() {
	FILE *f = fopen("in7.txt", "rb");

	Array<int64_t> positions = {};
	int64_t minimum = INT_MAX;
	int64_t maximum = INT_MIN;

	while (true) {
		int n;

		if (fscanf(f, "%d,", &n) == -1) {
			break;
		}

		positions.Add(n);

		MINIMIZE(minimum, n);
		MAXIMIZE(maximum, n);
	}

	int64_t minimumFuel = INT_MAX;

	for (int64_t target = minimum; target <= maximum; target++) {
		int64_t fuel = 0;

		for (int64_t i = 0; i < positions.Length(); i++) {
			int64_t x = positions[i] > target ? positions[i] - target : target - positions[i];
			fuel += x * (x + 1) / 2;
		}

		MINIMIZE(minimumFuel, fuel);
	}

	printf("part 2: %ld\n", minimumFuel);

	fclose(f);
	positions.Free();
}

int main() {
	Part1();
	Part2();
}
