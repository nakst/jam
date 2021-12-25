#include "ds.h"

#define N (100)

void Part1() {
	char map[N][N + 1];
	int a[N][N];
	int b[N][N];
	FILE *f = fopen("in15.txt", "rb");
	for (int i = 0; i < N; i++) fscanf(f, "%s\n", map[i]);
	fclose(f);

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			a[i][j] = -1;
			b[i][j] = INT_MAX;
			map[i][j] -= '0';
		}
	}

	int xx = 0, xy = 0;
	int nextA = 0;
	a[xx][xy] = nextA++;
	b[xx][xy] = 0;

	while (a[N - 1][N - 1] == -1) {
		if (xx >     0) MINIMIZE(b[xx - 1][xy], b[xx][xy] + map[xx - 1][xy]);
		if (xx < N - 1) MINIMIZE(b[xx + 1][xy], b[xx][xy] + map[xx + 1][xy]);
		if (xy >     0) MINIMIZE(b[xx][xy - 1], b[xx][xy] + map[xx][xy - 1]);
		if (xy < N - 1) MINIMIZE(b[xx][xy + 1], b[xx][xy] + map[xx][xy + 1]);

		bool first = true;

		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				if (a[i][j] == -1 && (first || b[i][j] < b[xx][xy])) {
					xx = i, xy = j;
					first = false;
				}
			}
		}

		assert(!first);
		a[xx][xy] = nextA++;
	}

	printf("part 1: %d\n", b[N - 1][N - 1]);
}

#undef N
#define N (500)

struct Node {
	uint16_t x, y;
};

// Sorted by b.
Array<Node> unvisited = {};

char map[N][N + 1];
int a[N][N];
int b[N][N];

int Compare(int x1, int y1, int x2, int y2) {
	if (b[x1][y1] > b[x2][y2]) return 1;
	if (b[x1][y1] < b[x2][y2]) return -1;
	if (x1 > x2) return 1;
	if (x1 < x2) return -1;
	if (y1 > y2) return 1;
	if (y1 < y2) return -1;
	assert(x1 == x2 && y1 == y2);
	return 0;
}

int Find(int x, int y) {
	uintptr_t currentIndex = 0;
	bool alreadyInArray = false;
	SEARCH(unvisited.Length(), result = Compare(x, y, unvisited[index].x, unvisited[index].y);, currentIndex, alreadyInArray);
	assert((a[x][y] == -1) == alreadyInArray);
	if (alreadyInArray) assert(unvisited[currentIndex].x == x && unvisited[currentIndex].y == y);
	return alreadyInArray ? currentIndex : -1;
}

void Insert(int x, int y) {
	uintptr_t currentIndex = 0;
	bool alreadyInArray = false;
	SEARCH(unvisited.Length(), result = Compare(x, y, unvisited[index].x, unvisited[index].y);, currentIndex, alreadyInArray);
	assert(!alreadyInArray && a[x][y] == -1);
	Node node = { (uint16_t) x, (uint16_t) y };
	unvisited.Insert(node, currentIndex);
}

void Apply(int x, int y, int k) {
	int index = Find(x, y);

	if (index != -1) {
		unvisited.Delete(index);
		MINIMIZE(b[x][y], k);
		Insert(x, y);
	} else {
		assert(b[x][y] <= k);
	}
}

void Part2() {
	FILE *f = fopen("in15.txt", "rb");
	for (int i = 0; i < 100; i++) fscanf(f, "%s\n", map[i]);
	fclose(f);

	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			map[i][j] -= '0';
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			map[i][j] = (map[i % 100][j % 100] + (i / 100 + j / 100));
			if (map[i][j] >= 10) map[i][j] -= 9;
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			a[i][j] = -1;
			b[i][j] = N * N * 10 + 1;
		}
	}

	for (uint16_t i = 0; i < N; i++) {
		for (uint16_t j = 0; j < N; j++) {
			Node node = { i, j };
			unvisited.Add(node);
		}
	}

	int xx = 0, xy = 0;
	int nextA = 0;
	a[xx][xy] = nextA++;
	b[xx][xy] = 0;
	unvisited.Delete(0);

	while (a[N - 1][N - 1] == -1) {
		if (xx >     0) Apply(xx - 1, xy, b[xx][xy] + map[xx - 1][xy]);
		if (xx < N - 1) Apply(xx + 1, xy, b[xx][xy] + map[xx + 1][xy]);
		if (xy >     0) Apply(xx, xy - 1, b[xx][xy] + map[xx][xy - 1]);
		if (xy < N - 1) Apply(xx, xy + 1, b[xx][xy] + map[xx][xy + 1]);

		assert(unvisited.Length());
		xx = unvisited[0].x;
		xy = unvisited[0].y;
		unvisited.Delete(0);
		a[xx][xy] = nextA++;
	}

	printf("part 2: %d\n", b[N - 1][N - 1]);
	unvisited.Free();
}

int main() {
	Part1();
	Part2();
}
