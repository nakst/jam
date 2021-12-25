#include "ds.h"

struct Rule {
	char left;
	char right;
	char add;
};

void Part1() {
	FILE *f = fopen("in14.txt", "rb");

	Array<Rule> rules = {};
	Array<char> polymer = {};

	char buffer[64];
	fscanf(f, "%s\n\n", buffer);

	for (int i = 0; buffer[i]; i++) {
		polymer.Add(buffer[i]);
	}

	while (true) {
		Rule rule;
		rule.left = fgetc(f);
		if (rule.left == -1) break;
		rule.right = fgetc(f);
		assert(fgetc(f) == ' ');
		assert(fgetc(f) == '-');
		assert(fgetc(f) == '>');
		assert(fgetc(f) == ' ');
		rule.add = fgetc(f);
		assert(fgetc(f) == '\n');
		rules.Add(rule);
	}

	fclose(f);

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < polymer.Length() - 1; j++) {
			char left = polymer[j];
			char right = polymer[j + 1];

			for (int k = 0; k < rules.Length(); k++) {
				if (rules[k].left == left && rules[k].right == right) {
					polymer.Insert(rules[k].add, j + 1);
					j++;
					break;
				}
			}
		}
	}

	int seen[26] = {};

	for (int i = 0; i < polymer.Length(); i++) {
		seen[polymer[i] - 'A']++;
	}

	int mc = -1, lc = -1;

	for (int i = 0; i < 26; i++) {
		if (!seen[i]) continue;
		if (mc == -1 || seen[i] > seen[mc]) mc = i;
		if (lc == -1 || seen[i] < seen[lc]) lc = i;
	}

	printf("part 1: %d\n", seen[mc] - seen[lc]);

	rules.Free();
	polymer.Free();
}

void Part2() {
	FILE *f = fopen("in14.txt", "rb");

	Array<Rule> rules = {};
	int64_t pairCount[26][26] = {};
	int64_t singleCount[26] = {};

	char buffer[64];
	fscanf(f, "%s\n\n", buffer);

	for (int i = 0; buffer[i + 1]; i++) {
		pairCount[buffer[i] - 'A'][buffer[i + 1] - 'A']++;
	}

	for (int i = 0; buffer[i]; i++) {
		singleCount[buffer[i] - 'A']++;
	}

	while (true) {
		Rule rule;
		rule.left = fgetc(f);
		if (rule.left == -1) break;
		rule.right = fgetc(f);
		assert(fgetc(f) == ' ');
		assert(fgetc(f) == '-');
		assert(fgetc(f) == '>');
		assert(fgetc(f) == ' ');
		rule.add = fgetc(f);
		assert(fgetc(f) == '\n');
		rules.Add(rule);
	}

	fclose(f);

	for (int i = 0; i < 40; i++) {
		int64_t newPairCount[26][26] = {};
		int64_t newSingleCount[26] = {};

		memcpy(newPairCount, pairCount, sizeof(pairCount));
		memcpy(newSingleCount, singleCount, sizeof(singleCount));

		for (int j0 = 0; j0 < 26; j0++) {
			for (int j1 = 0; j1 < 26; j1++) {
				for (int k = 0; k < rules.Length(); k++) {
					if (rules[k].left == j0 + 'A' && rules[k].right == j1 + 'A') {
						int64_t start = pairCount[j0][j1];
						if (!start) break;
						newPairCount[j0][rules[k].add - 'A'] += start;
						newPairCount[rules[k].add - 'A'][j1] += start;
						newPairCount[j0][j1] -= start;
						newSingleCount[rules[k].add - 'A'] += start;
						break;
					}
				}
			}
		}

		memcpy(pairCount, newPairCount, sizeof(pairCount));
		memcpy(singleCount, newSingleCount, sizeof(singleCount));
	}

	int64_t mc = -1, lc = -1;

	for (int i = 0; i < 26; i++) {
		if (!singleCount[i]) continue;
		if (mc == -1 || singleCount[i] > singleCount[mc]) mc = i;
		if (lc == -1 || singleCount[i] < singleCount[lc]) lc = i;
	}

	printf("part 2: %ld\n", singleCount[mc] - singleCount[lc]);

	rules.Free();
}

int main() {
	Part1();
	Part2();
}
