#include "ds.h"

void Part1() {
	bool initializationCubes[101][101][101] = {};
	FILE *f = fopen("in22.txt", "rb");

	while (true) {
		char verb[16];
		int xf, xt, yf, yt, zf, zt;

		if (fscanf(f, "%s x=%d..%d,y=%d..%d,z=%d..%d\n", verb, &xf, &xt, &yf, &yt, &zf, &zt) == -1) {
			break;
		}

		xf += 50;
		yf += 50;
		zf += 50;
		xt += 51;
		yt += 51;
		zt += 51;

		MAXIMIZE(xf, 0);
		MAXIMIZE(yf, 0);
		MAXIMIZE(zf, 0);
		MAXIMIZE(xt, 0);
		MAXIMIZE(yt, 0);
		MAXIMIZE(zt, 0);

		MINIMIZE(xf, 101);
		MINIMIZE(yf, 101);
		MINIMIZE(zf, 101);
		MINIMIZE(xt, 101);
		MINIMIZE(yt, 101);
		MINIMIZE(zt, 101);

		for (int x = xf; x < xt; x++) {
			for (int y = yf; y < yt; y++) {
				for (int z = zf; z < zt; z++) {
					initializationCubes[x][y][z] = verb[1] == 'n';
				}
			}
		}
	}

	fclose(f);

	int count = 0;

	for (int x = 0; x <= 100; x++) {
		for (int y = 0; y <= 100; y++) {
			for (int z = 0; z <= 100; z++) {
				if (initializationCubes[x][y][z]) {
					count++;
				}
			}
		}
	}

	printf("part 1: %d\n", count);
}

struct Cuboid {
	int xf, xt, yf, yt, zf, zt;
};

Array<Cuboid> array = {};

void ProcessCuboid(Cuboid insert, bool on) {
	for (int i = 0; i < array.Length(); i++) {
		// Remove overlapping regions.
		Cuboid source = array[i];

		// Clamp the cuboid being inserted to the source cuboid.
		Cuboid clamped = insert;
		MAXIMIZE(clamped.xf, source.xf);
		MAXIMIZE(clamped.yf, source.yf);
		MAXIMIZE(clamped.zf, source.zf);
		MINIMIZE(clamped.xt, source.xt);
		MINIMIZE(clamped.yt, source.yt);
		MINIMIZE(clamped.zt, source.zt);

		if (clamped.xf >= clamped.xt || clamped.yf >= clamped.yt || clamped.zf >= clamped.zt) {
			// If the clamped cuboid has non-positive area, there is nothing to remove.
			continue;
		}

		// Remove the source cuboid from the array.
		array.Delete(i);
		i--;

		// We want to remove the clamped cuboid from the source cuboid.
		// So we break up the source cuboid into 27 sections and add all but the middle back.
		int xp[4] = { source.xf, clamped.xf, clamped.xt, source.xt };
		int yp[4] = { source.yf, clamped.yf, clamped.yt, source.yt };
		int zp[4] = { source.zf, clamped.zf, clamped.zt, source.zt };

		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					if (j == 1 && k == 1 && l == 1) {
						continue;
					}

					Cuboid add;
					add.xf = xp[j], add.xt = xp[j + 1];
					add.yf = yp[k], add.yt = yp[k + 1];
					add.zf = zp[l], add.zt = zp[l + 1];

					if (add.xf < add.xt && add.yf < add.yt && add.zf < add.zt) {
						// Only add the cuboid to the array if it has non-zero volume!
						array.Add(add);
					}
				}
			}
		}
	}

	if (on) {
		// Insert the new cuboid.
		array.Add(insert);
	}
}

void Part2() {
	FILE *f = fopen("in22.txt", "rb");

	while (true) {
		char verb[16];
		Cuboid insert = {};

		if (fscanf(f, "%s x=%d..%d,y=%d..%d,z=%d..%d\n", verb, &insert.xf, &insert.xt, &insert.yf, &insert.yt, &insert.zf, &insert.zt) == -1) {
			break;
		}

		insert.xt++;
		insert.yt++;
		insert.zt++;

		ProcessCuboid(insert, verb[1] == 'n');
	}

	fclose(f);

	int64_t count = 0;

	for (int i = 0; i < array.Length(); i++) {
		count += (int64_t) (array[i].xt - array[i].xf) * (int64_t) (array[i].yt - array[i].yf) * (int64_t) (array[i].zt - array[i].zf);
	}

	printf("part 2: %ld\n", count);

	array.Free();
}

int main() {
	Part1();
	Part2();
}
