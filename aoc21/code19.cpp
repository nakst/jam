#include "ds.h"

// Plan: For each pair of beacons, suppose they are the same and then try to see how many other beacons match up.

struct V3 {
	int x, y, z;
};

struct Scanner {
	Array<V3> readings;
	V3 position;
	int orientation;
	bool determinedPositionRelativeToScanner0;
};

Array<Scanner> scanners = {};

bool Equals(V3 a, V3 b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

V3 Map(V3 v, int orientation) {
	int k = orientation / 6;
	int l = orientation % 6;

	if (l == 0) {
		v = { v.x, v.y, v.z };
	} else if (l == 1) {
		v = { v.x, -v.z, v.y };
	} else if (l == 2) {
		v = { v.x, -v.y, -v.z };
	} else if (l == 3) {
		v = { v.x, v.z, -v.y };
	} else if (l == 4) {
		v = { v.z, v.y, -v.x };
	} else if (l == 5) {
		v = { -v.z, v.y, v.x };
	}

	if (k == 0) {
		v = { v.x, v.y, v.z };
	} else if (k == 1) {
		v = { v.y, -v.x, v.z };
	} else if (k == 2) {
		v = { -v.x, -v.y, v.z };
	} else if (k == 3) {
		v = { -v.y, v.x, v.z };
	}

	return v;
}

void Part1() {
	Array<V3> allBeacons = {};

	for (int i = 0; i < scanners.Length(); i++) {
		for (int j = 0; j < scanners[i].readings.Length(); j++) {
			for (int k = 0; k < allBeacons.Length(); k++) {
				if (Equals(allBeacons[k], scanners[i].readings[j])) {
					goto skip;
				}
			}

			allBeacons.Add(scanners[i].readings[j]);

			skip:;
		}
	}

	printf("part 1: %d\n", allBeacons.Length());

	allBeacons.Free();
}

int Abs(int x) {
	return x > 0 ? x : -x;
}

void Part2() {
	int d = 0;

	for (int i = 0; i < scanners.Length(); i++) {
		for (int j = 0; j < scanners.Length(); j++) {
			int distance = Abs(scanners[i].position.x - scanners[j].position.x)
				+ Abs(scanners[i].position.y - scanners[j].position.y)
				+ Abs(scanners[i].position.z - scanners[j].position.z);
			MAXIMIZE(d, distance);
		}
	}

	printf("part 2: %d\n", d);
}

int main() {
	FILE *f = fopen("in19.txt", "rb");

	while (true) {
		int i;

		if (fscanf(f, "--- scanner %d ---\n", &i) == -1) {
			break;
		}

		assert(i == scanners.Length());

		Scanner scanner = {};

		while (true) {
			V3 reading;

			if (fscanf(f, "%d,%d,%d\n", &reading.x, &reading.y, &reading.z) != 3) {
				break;
			}

			scanner.readings.Add(reading);
		}

		scanners.Add(scanner);
		ungetc('-', f);
	}

	fclose(f);

	scanners[0].determinedPositionRelativeToScanner0 = true;
	int remainingPositionsToDetermine = scanners.Length() - 1;

	while (remainingPositionsToDetermine) {
		for (int i = 0; i < scanners.Length(); i++) {
			if (scanners[i].determinedPositionRelativeToScanner0) {
				continue;
			}

			Scanner *si = &scanners[i];

			for (int j = 0; j < scanners.Length(); j++) {
				if (j == i || !scanners[j].determinedPositionRelativeToScanner0) {
					continue;
				}

				Scanner *sj = &scanners[j];

				// We know the relative position of scanner j, and we can use it to try to work out the position of scanner i.

				for (int i = 0; i < si->readings.Length(); i++) {
					for (int j = 0; j < sj->readings.Length(); j++) {
						// Suppose si->readings[i] and sj->readings[j] are the same beacon.

						for (int k = 0; k < 24; k++) {
							// and si has orientation k.
							si->position.x = sj->readings[j].x - Map(si->readings[i], k).x;
							si->position.y = sj->readings[j].y - Map(si->readings[i], k).y;
							si->position.z = sj->readings[j].z - Map(si->readings[i], k).z;
							si->orientation = k;

							// Do 12 readings match?
							
							int matches = 0;

							for (int i = 0; i < si->readings.Length(); i++) {
								for (int j = 0; j < sj->readings.Length(); j++) {
									V3 b0 = sj->readings[j];
									V3 b1 = Map(si->readings[i], k);
									b1.x += si->position.x;
									b1.y += si->position.y;
									b1.z += si->position.z;

									if (Equals(b0, b1)) {
										matches++;
										goto foundMatch;
									}
								}

								foundMatch:;
							}

							if (matches >= 12) {
								// These must be the same beacon!
								// We have located si.

								// Remap its readings.

								for (int i = 0; i < si->readings.Length(); i++) {
									si->readings[i] = Map(si->readings[i], k);
									si->readings[i].x += si->position.x;
									si->readings[i].y += si->position.y;
									si->readings[i].z += si->position.z;
								}

								// And mark as done.
								si->determinedPositionRelativeToScanner0 = true;
								remainingPositionsToDetermine--;
#if 0
								printf("found scanner %ld at %d, %d, %d orientation %d via scanner %ld\n", 
										si - scanners.array, si->position.x, si->position.y, si->position.z, si->orientation, sj - scanners.array);
#endif
								goto loop;
							}
						}
					}
				}
			}
		}

		loop:;
	}

	Part1();
	Part2();

	for (int i = 0; i < scanners.Length(); i++) {
		scanners[i].readings.Free();
	}

	scanners.Free();
}
