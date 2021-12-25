#include "ds.h"

struct State1 {
	char hallway[11];
	char rooms[4][2];
	int64_t energy;
};

int64_t minimumEnergy1;
MapShort<uint64_t, int64_t> minimumEnergySeenAtState1;

void Go(State1 state, int indent) {
	const int64_t energyUsage[] = { 1, 10, 100, 1000 };

	{
		uint64_t hash = 0;
		char *p = (char *) &state;
		for (int i = 0; i < 11 + 4 * 2; i++) hash |= (uint64_t) (p[i] ? p[i] - 'A' + 1 : 0) << (uint64_t) (3 * i);
		int64_t m = minimumEnergySeenAtState1.Get(hash);
		if (m && m < state.energy) return;
		minimumEnergySeenAtState1.Put(hash, state.energy);
	}

#if 0
	if (indent < 3) {
		for (int i = 0; i < indent; i++) printf("\t");
		printf("=============\n");
		for (int i = 0; i < indent; i++) printf("\t");
		printf("|");
		for (int i = 0; i < 11; i++) printf("%c", state.hallway[i] ?: ' ');
		printf("|\n");
		for (int i = 0; i < indent; i++) printf("\t");
		printf("|==%c=%c=%c=%c==|\n", state.rooms[0][0] ?: ' ', state.rooms[1][0] ?: ' ', state.rooms[2][0] ?: ' ', state.rooms[3][0] ?: ' ');
		for (int i = 0; i < indent; i++) printf("\t");
		printf("  |%c|%c|%c|%c|  \n", state.rooms[0][1] ?: ' ', state.rooms[1][1] ?: ' ', state.rooms[2][1] ?: ' ', state.rooms[3][1] ?: ' ');
		for (int i = 0; i < indent; i++) printf("\t");
		printf("  ========= \n");
		for (int i = 0; i < indent; i++) printf("\t");
		printf("%ld\n\n", state.energy);
	}
#endif

	// Check if done.

	if (state.rooms[0][0] == 'A' && state.rooms[0][1] == 'A'
			&& state.rooms[1][0] == 'B' && state.rooms[1][1] == 'B'
			&& state.rooms[2][0] == 'C' && state.rooms[2][1] == 'C'
			&& state.rooms[3][0] == 'D' && state.rooms[3][1] == 'D') {
		for (int i = 0; i < 11; i++) assert(state.hallway[i] == 0);
		if (!minimumEnergy1) minimumEnergy1 = state.energy;
		MINIMIZE(minimumEnergy1, state.energy);
		return;
	}

	// Moving hallway -> rooms.

	for (int i = 0; i < 11; i++) {
		if (state.hallway[i]) {
			// Is the room for the pod okay to move into?
			char p0 = state.rooms[state.hallway[i] - 'A'][0];
			char p1 = state.rooms[state.hallway[i] - 'A'][1];

			if ((p0 == 0 || p0 == state.hallway[i]) && (p1 == 0 || p1 == state.hallway[i])) {
				// Is the path clear?
				int xf = (state.hallway[i] - 'A') * 2 + 2;
				int xt = i;
				MAKELT(xf, xt);
				bool clear = true;

				for (int j = xf; j <= xt; j++) {
					if (state.hallway[j] && j != i) {
						clear = false;
						break;
					}
				}

				if (clear) {
					// Can move it into the room.
					State1 ns = state;
					int64_t steps = xt - xf;

					if (state.rooms[state.hallway[i] - 'A'][1] == 0) {
						ns.rooms[state.hallway[i] - 'A'][1] = state.hallway[i];
						steps += 2;
					} else {
						ns.rooms[state.hallway[i] - 'A'][0] = state.hallway[i];
						steps += 1;
					}

					ns.hallway[i] = 0;
					ns.energy += steps * energyUsage[state.hallway[i] - 'A'];
					Go(ns, indent + 1);
				}
			}
		}
	}

	// Moving room -> hallway.

	for (int i = 0; i < 4; i++) {
		int x = i * 2 + 2;
		int my;

		if (state.rooms[i][0] - 'A' == i && state.rooms[i][1] - 'A' == i) {
			// Room is complete.
			continue;
		}

		if (state.rooms[i][0]) {
			my = 0;
		} else if (state.rooms[i][1] && state.rooms[i][1] - 'A' != i /* no reason to move the bottommost if good */) {
			my = 1;
		} else {
			continue;
		}

		// For every place to move to.
		for (int j = 0; j < 11; j++) {
			if (j == 2 || j == 4 || j == 6 || j == 8) continue;

			int xf = j;
			int xt = x;
			MAKELT(xf, xt);
			bool clear = true;

			for (int k = xf; k <= xt; k++) {
				if (state.hallway[k]) {
					clear = false;
					break;
				}
			}

			if (clear) {
				State1 ns = state;
				int64_t steps = xt - xf + my + 1;
				ns.rooms[i][my] = 0;
				ns.hallway[j] = state.rooms[i][my];
				ns.energy += steps * energyUsage[state.rooms[i][my] - 'A'];
				Go(ns, indent + 1);
			}
		}
	}
}

void Part1() {
	FILE *f = fopen("in23.txt", "rb");
	char buffer[66];
	fread(buffer, 1, sizeof(buffer), f);
	State1 state = {};
	state.rooms[0][0] = buffer[31];
	state.rooms[0][1] = buffer[31 + 14];
	state.rooms[1][0] = buffer[33];
	state.rooms[1][1] = buffer[33 + 14];
	state.rooms[2][0] = buffer[35];
	state.rooms[2][1] = buffer[35 + 14];
	state.rooms[3][0] = buffer[37];
	state.rooms[3][1] = buffer[37 + 14];
	fclose(f);
	Go(state, 0);
	printf("part 1: %ld\n", minimumEnergy1);
	minimumEnergySeenAtState1.Free();
}

struct State2 {
	char hallway[11];
	char rooms[4][4];
	int64_t energy;
};

int64_t minimumEnergy2;
MapShort<__int128, int64_t> minimumEnergySeenAtState2;

void Go(State2 state, int indent) {
	const int64_t energyUsage[] = { 1, 10, 100, 1000 };

	if ((minimumEnergy2 && state.energy > minimumEnergy2)) {
		return;
	}

	{
		__int128 hash = 0;
		char *p = (char *) &state;
		for (int i = 0; i < 11 + 4 * 4; i++) hash |= (__int128) (p[i] ? p[i] - 'A' + 1 : 0) << (__int128) (3 * i);
		int64_t m = minimumEnergySeenAtState2.Get(hash);
		if (m && m < state.energy) return;
		minimumEnergySeenAtState2.Put(hash, state.energy);
	}

#if 0
	if (indent < 2) {
		for (int i = 0; i < indent; i++) printf("\t");
		printf("=============\n");
		for (int i = 0; i < indent; i++) printf("\t");
		printf("|");
		for (int i = 0; i < 11; i++) printf("%c", state.hallway[i] ?: ' ');
		printf("|\n");
		for (int i = 0; i < indent; i++) printf("\t");
		printf("|==%c=%c=%c=%c==|\n", state.rooms[0][0] ?: ' ', state.rooms[1][0] ?: ' ', state.rooms[2][0] ?: ' ', state.rooms[3][0] ?: ' ');
		for (int i = 0; i < indent; i++) printf("\t");
		printf("  |%c|%c|%c|%c|  \n", state.rooms[0][1] ?: ' ', state.rooms[1][1] ?: ' ', state.rooms[2][1] ?: ' ', state.rooms[3][1] ?: ' ');
		for (int i = 0; i < indent; i++) printf("\t");
		printf("  |%c|%c|%c|%c|  \n", state.rooms[0][2] ?: ' ', state.rooms[1][2] ?: ' ', state.rooms[2][2] ?: ' ', state.rooms[3][2] ?: ' ');
		for (int i = 0; i < indent; i++) printf("\t");
		printf("  |%c|%c|%c|%c|  \n", state.rooms[0][3] ?: ' ', state.rooms[1][3] ?: ' ', state.rooms[2][3] ?: ' ', state.rooms[3][3] ?: ' ');
		for (int i = 0; i < indent; i++) printf("\t");
		printf("  ========= \n");
		for (int i = 0; i < indent; i++) printf("\t");
		printf("%ld\n\n", state.energy);
	}
#endif

	// Check if done.

	if (state.rooms[0][0] == 'A' && state.rooms[0][1] == 'A' && state.rooms[0][2] == 'A' && state.rooms[0][3] == 'A'
			&& state.rooms[1][0] == 'B' && state.rooms[1][1] == 'B' && state.rooms[1][2] == 'B' && state.rooms[1][3] == 'B'
			&& state.rooms[2][0] == 'C' && state.rooms[2][1] == 'C' && state.rooms[2][2] == 'C' && state.rooms[2][3] == 'C'
			&& state.rooms[3][0] == 'D' && state.rooms[3][1] == 'D' && state.rooms[3][2] == 'D' && state.rooms[3][3] == 'D') {
		for (int i = 0; i < 11; i++) assert(state.hallway[i] == 0);
		if (!minimumEnergy2) { minimumEnergy2 = state.energy; }
		MINIMIZE(minimumEnergy2, state.energy);
		return;
	}

	Array<State2> newStates = {};

	// Moving hallway -> rooms.

	for (int i = 0; i < 11; i++) {
		if (state.hallway[i]) {
			// Is the room for the pod okay to move into?
			char p0 = state.rooms[state.hallway[i] - 'A'][0];
			char p1 = state.rooms[state.hallway[i] - 'A'][1];
			char p2 = state.rooms[state.hallway[i] - 'A'][2];
			char p3 = state.rooms[state.hallway[i] - 'A'][3];

			if ((p0 == 0 || p0 == state.hallway[i]) && (p1 == 0 || p1 == state.hallway[i])
					&& (p2 == 0 || p2 == state.hallway[i]) && (p3 == 0 || p3 == state.hallway[i])) {
				// Is the path clear?
				int xf = (state.hallway[i] - 'A') * 2 + 2;
				int xt = i;
				MAKELT(xf, xt);
				bool clear = true;

				for (int j = xf; j <= xt; j++) {
					if (state.hallway[j] && j != i) {
						clear = false;
						break;
					}
				}

				if (clear) {
					// Can move it into the room.
					State2 ns = state;
					int64_t steps = xt - xf;

					if (state.rooms[state.hallway[i] - 'A'][3] == 0) {
						ns.rooms[state.hallway[i] - 'A'][3] = state.hallway[i];
						steps += 4;
					} else if (state.rooms[state.hallway[i] - 'A'][2] == 0) {
						ns.rooms[state.hallway[i] - 'A'][2] = state.hallway[i];
						steps += 3;
					} else if (state.rooms[state.hallway[i] - 'A'][1] == 0) {
						ns.rooms[state.hallway[i] - 'A'][1] = state.hallway[i];
						steps += 2;
					} else {
						ns.rooms[state.hallway[i] - 'A'][0] = state.hallway[i];
						steps += 1;
					}

					ns.hallway[i] = 0;
					ns.energy += steps * energyUsage[state.hallway[i] - 'A'];
					newStates.Add(ns);
					// Go(ns, indent + 1);
				}
			}
		}
	}

	// Moving room -> hallway.

	for (int i = 0; i < 4; i++) {
		int x = i * 2 + 2;
		int my;

		if (state.rooms[i][0] - 'A' == i && state.rooms[i][1] - 'A' == i && state.rooms[i][2] - 'A' == i && state.rooms[i][3] - 'A' == i) {
			// Room is complete.
			continue;
		}

		if (state.rooms[i][0]) {
			my = 0;
		} else if (state.rooms[i][1]) {
			my = 1;
		} else if (state.rooms[i][2]) {
			my = 2;
		} else if (state.rooms[i][3]) {
			my = 3;
		} else {
			continue;
		}

		// If this pod is valid and so is everything below it, there is no reason to move.
		bool allValid = true;
		for (int j = my; j < 4; j++) {
			if (state.rooms[i][j] != i) allValid = false;
		}
		if (allValid) continue;

		// For every place to move to.
		for (int j = 0; j < 11; j++) {
			if (j == 2 || j == 4 || j == 6 || j == 8) continue;

			int xf = j;
			int xt = x;
			MAKELT(xf, xt);
			bool clear = true;

			for (int k = xf; k <= xt; k++) {
				if (state.hallway[k]) {
					clear = false;
					break;
				}
			}

			if (clear) {
				State2 ns = state;
				int64_t steps = xt - xf + my + 1;
				ns.rooms[i][my] = 0;
				ns.hallway[j] = state.rooms[i][my];
				ns.energy += steps * energyUsage[state.rooms[i][my] - 'A'];
				newStates.Add(ns);
				// Go(ns, indent + 1);
			}
		}
	}

	qsort(newStates.array, newStates.Length(), sizeof(newStates[0]), [] (const void *a, const void *b) {
		return CompareIntegers64Ascending(&((const State2 *) a)->energy, &((const State2 *) b)->energy);
	});

	for (int i = 0; i < newStates.Length(); i++) {
		Go(newStates[i], indent + 1);
	}

	newStates.Free();
}

void Part2() {
	FILE *f = fopen("in23.txt", "rb");
	char buffer[66];
	fread(buffer, 1, sizeof(buffer), f);
	State2 state = {};
	state.rooms[0][0] = buffer[31];
	state.rooms[0][1] = 'D';
	state.rooms[0][2] = 'D';
	state.rooms[0][3] = buffer[31 + 14];
	state.rooms[1][0] = buffer[33];
	state.rooms[1][1] = 'C';
	state.rooms[1][2] = 'B';
	state.rooms[1][3] = buffer[33 + 14];
	state.rooms[2][0] = buffer[35];
	state.rooms[2][1] = 'B';
	state.rooms[2][2] = 'A';
	state.rooms[2][3] = buffer[35 + 14];
	state.rooms[3][0] = buffer[37];
	state.rooms[3][1] = 'A';
	state.rooms[3][2] = 'C';
	state.rooms[3][3] = buffer[37 + 14];
	fclose(f);
	Go(state, 0);
	printf("part 2: %ld\n", minimumEnergy2);
	minimumEnergySeenAtState2.Free();
}

int main() {
	Part1();
	Part2();
}
