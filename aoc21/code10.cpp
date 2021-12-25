#include "ds.h"

void Part1() {
	FILE *f = fopen("in10.txt", "rb");

	int score = 0;

	while (true) {
		char line[1024];

		if (-1 == fscanf(f, "%s\n", line)) {
			break;
		}

		Array<char> stack = {};

		for (int i = 0; line[i]; i++) {
			if (line[i] == '(') {
				stack.Add(')');
			} else if (line[i] == ')') {
				if (stack.Last() != ')') {
					score += 3;
					goto nextLine;
				} else {
					stack.Pop();
				}
			} else if (line[i] == '[') {
				stack.Add(']');
			} else if (line[i] == ']') {
				if (stack.Last() != ']') {
					score += 57;
					goto nextLine;
				} else {
					stack.Pop();
				}
			} else if (line[i] == '{') {
				stack.Add('}');
			} else if (line[i] == '}') {
				if (stack.Last() != '}') {
					score += 1197;
					goto nextLine;
				} else {
					stack.Pop();
				}
			} else if (line[i] == '<') {
				stack.Add('>');
			} else if (line[i] == '>') {
				if (stack.Last() != '>') {
					score += 25137;
					goto nextLine;
				} else {
					stack.Pop();
				}
			}
		}

		nextLine:;
		stack.Free();
	}

	fclose(f);

	printf("part 1: %d\n", score);
}

void Part2() {
	FILE *f = fopen("in10.txt", "rb");

	Array<int64_t> scores = {};

	while (true) {
		char line[1024];

		if (-1 == fscanf(f, "%s\n", line)) {
			break;
		}

		Array<char> stack = {};

		int64_t score = 0;

		for (int i = 0; line[i]; i++) {
			if (line[i] == '(') {
				stack.Add(')');
			} else if (line[i] == ')') {
				if (stack.Last() != ')') {
					goto discard;
				} else {
					stack.Pop();
				}
			} else if (line[i] == '[') {
				stack.Add(']');
			} else if (line[i] == ']') {
				if (stack.Last() != ']') {
					goto discard;
				} else {
					stack.Pop();
				}
			} else if (line[i] == '{') {
				stack.Add('}');
			} else if (line[i] == '}') {
				if (stack.Last() != '}') {
					goto discard;
				} else {
					stack.Pop();
				}
			} else if (line[i] == '<') {
				stack.Add('>');
			} else if (line[i] == '>') {
				if (stack.Last() != '>') {
					goto discard;
				} else {
					stack.Pop();
				}
			}
		}

		while (stack.Length()) {
			score *= 5;
			if (stack.Last() == ')') score += 1;
			if (stack.Last() == ']') score += 2;
			if (stack.Last() == '}') score += 3;
			if (stack.Last() == '>') score += 4;
			stack.Pop();
		}

		scores.Add(score);

		discard:;
		stack.Free();
	}

	fclose(f);

	qsort(scores.array, scores.Length(), sizeof(int64_t), CompareIntegers64Descending);

	printf("part 2: %ld\n", scores[scores.Length() / 2]);

	scores.Free();
}

int main() {
	Part1();
	Part2();
}
