#include "ds.h"

struct String {
	char s[64];
};

void Part1() {
	FILE *f = fopen("in3.txt", "rb");

	int oneCount[12] = {};
	int zeroCount[12] = {};

	while (true) {
		char string[64];

		if (fscanf(f, "%s\n", string) == -1) {
			break;
		}

		for (int i = 0; i < 12; i++) {
			if (string[i] == '1') {
				oneCount[i]++;
			} else {
				zeroCount[i]++;
			}
		}
	}

	fclose(f);

	int x = 0, y = 0;

	for (int i = 0; i < 12; i++) {
		if (oneCount[i] > zeroCount[i]) {
			x |= 1 << (11 - i);
		} else {
			y |= 1 << (11 - i);
		}
	}

	printf("part 1: %d\n", x * y);
}

void Part2() {
	FILE *f = fopen("in3.txt", "rb");
	Array<String> list1 = {};
	Array<String> list2 = {};

	while (true) {
		String string;

		if (fscanf(f, "%s\n", string.s) == -1) {
			break;
		}

		list1.Add(string);
		list2.Add(string);
	}

	fclose(f);

	int bit = 0;
	
	while (list1.Length() > 1) {
		int oneCount = 0, zeroCount = 0;

		for (int i = 0; i < list1.Length(); i++) {
			if (list1[i].s[bit] == '1') {
				oneCount++;
			} else {
				zeroCount++;
			}
		}

		char keep = zeroCount > oneCount ? '0' : '1';

		for (int i = 0; i < list1.Length(); i++) {
			if (list1[i].s[bit] != keep) {
				list1.Delete(i);
				i--;
			}
		}

		bit++;
	}

	bit = 0;

	while (list2.Length() > 1) {
		int oneCount = 0, zeroCount = 0;

		for (int i = 0; i < list2.Length(); i++) {
			if (list2[i].s[bit] == '1') {
				oneCount++;
			} else {
				zeroCount++;
			}
		}

		char keep = zeroCount > oneCount ? '1' : '0';

		for (int i = 0; i < list2.Length(); i++) {
			if (list2[i].s[bit] != keep) {
				list2.Delete(i);
				i--;
			}
		}

		bit++;
	}

	assert(list1.Length());
	assert(list2.Length());

	int x = 0, y = 0;

	for (int i = 0; i < 12; i++) {
		if (list1[0].s[i] == '1') x |= 1 << (11 - i);
		if (list2[0].s[i] == '1') y |= 1 << (11 - i);
	}

	printf("part 2: %d\n", x * y);

	list1.Free();
	list2.Free();
}

int main() {
	Part1();
	Part2();
}
