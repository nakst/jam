#include "ds.h"

#define START (1)
#define END   (2)

struct Cave {
	bool big;
	int visited;
	Array<uint16_t> connections;
};

MapShort<uint16_t, Cave> caves;

int Explore(uint16_t id, uint8_t part, bool visitedSingleSmallCaveTwice = false) {
	if (id == END) {
		return 1;
	}

	Cave *cave = caves.At(id, true);

	if (cave->visited && !cave->big) {
		if (visitedSingleSmallCaveTwice || id == START || part == 1) {
			return 0;
		} else {
			visitedSingleSmallCaveTwice = true;
		}
	}

	int pathCount = 0;
	cave->visited++;

	for (int i = 0; i < cave->connections.Length(); i++) {
		pathCount += Explore(cave->connections[i], part, visitedSingleSmallCaveTwice);
	}

	cave->visited--;
	return pathCount;
}

int main() {
	FILE *f = fopen("in12.txt", "rb");

	while (true) {
		char line[32];

		if (fscanf(f, "%s\n", line) == -1) {
			break;
		}

		for (int i = 0; line[i]; i++) {
			if (line[i] == '-') {
				uint16_t f, t;
				Cave *cave;

				if (line[0] == 's' && line[1] == 't' && line[2] == 'a') f = START;
				else if (line[0] == 'e' && line[1] == 'n' && line[2] == 'd') f = END;
				else f = ((uint16_t) line[0] << 8) | line[1];

				if (line[i + 1] == 's' && line[i + 2] == 't' && line[i + 3] == 'a') t = START;
				else if (line[i + 1] == 'e' && line[i + 2] == 'n' && line[i + 3] == 'd') t = END;
				else t = ((uint16_t) line[i + 1] << 8) | line[i + 2];

				cave = caves.At(f, true);
				cave->big = isupper(f >> 8);
				cave->connections.Add(t);

				cave = caves.At(t, true);
				cave->big = isupper(t >> 8);
				cave->connections.Add(f);

				break;
			}
		}

	}

	fclose(f);

	printf("part 1: %d\n", Explore(START, 1));
	printf("part 2: %d\n", Explore(START, 2));

	for (uintptr_t i = 0; i < caves.capacity; i++) {
		caves.array[i].value.connections.Free();
	}

	caves.Free();
}
