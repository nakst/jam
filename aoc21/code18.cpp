#include "ds.h"

#define OPEN (-1)
#define CLOSE (-2)
#define COMMA (-3)

struct Number {
	Array<int> c;
};

Array<Number> numbers = {};

Number Add(Number left, Number right) {
	Number x = {};

	x.c.Add(OPEN);

	for (int i = 0; i < left.c.Length(); i++) {
		x.c.Add(left.c[i]);
	}

	x.c.Add(COMMA);

	for (int i = 0; i < right.c.Length(); i++) {
		x.c.Add(right.c[i]);
	}

	x.c.Add(CLOSE);

	while (true) {
		int n = 0;
		bool didReduction = false;

		for (int i = 0; i < x.c.Length(); i++) {
			if (x.c[i] == OPEN) {
				if (n == 4) {
					// Explode.

					assert(x.c[i + 2] == COMMA);
					assert(x.c[i + 4] == CLOSE);

					for (int j = i; j >= 0; j--) {
						if (x.c[j] >= 0) {
							x.c[j] += x.c[i + 1];
							break;
						}
					}

					for (int j = i + 4; j < x.c.Length(); j++) {
						if (x.c[j] >= 0) {
							x.c[j] += x.c[i + 3];
							break;
						}
					}

					x.c.Delete(i);
					x.c.Delete(i);
					x.c.Delete(i);
					x.c.Delete(i);
					x.c.Delete(i);
					x.c.Insert(0, i);

					didReduction = true;
					break;
				} else {
					n++;
				}
			} else if (x.c[i] == CLOSE) {
				n--;
			}
		}

		if (!didReduction) {
			for (int i = 0; i < x.c.Length(); i++) {
				if (x.c[i] >= 10) {
					int y = x.c[i];
					x.c.Delete(i);
					x.c.Insert(CLOSE, i);
					x.c.Insert((y + 1) / 2, i);
					x.c.Insert(COMMA, i);
					x.c.Insert(y / 2, i);
					x.c.Insert(OPEN, i);
					didReduction = true;
					break;
				}
			}
		}

		if (!didReduction) {
			break;
		}
	}

	return x;
}

int64_t Magnitude(Number x) {
	int n = 0;

	for (int i = 0; i < x.c.Length(); i++) {
		if (x.c[i] == OPEN) {
			n++;
		} else if (x.c[i] == CLOSE) {
			n--;
		} else if (x.c[i] == COMMA) {
			if (n == 1) {
				Number left = {};
				Number right = {};

				assert(x.c.First() == OPEN);
				assert(x.c.Last() == CLOSE);

				for (int j = 1; j < i; j++) {
					left.c.Add(x.c[j]);
				}

				for (int j = i + 1; j < x.c.Length() - 1; j++) {
					right.c.Add(x.c[j]);
				}

				int64_t r = Magnitude(left) * 3 + Magnitude(right) * 2;

				left.c.Free();
				right.c.Free();

				return r;
			}
		} else if (x.c[i] >= 0) {
			if (n == 0) {
				assert(x.c.Length() == 1);
				return x.c[i];
			}
		}
	}

	return n;
}

void Part1() {
	Number left = numbers[0];

	for (int i = 1; i < numbers.Length(); i++) {
		Number oldLeft = left;
		left = Add(oldLeft, numbers[i]);
		if (i >= 2) oldLeft.c.Free();
	}

	printf("part 1: %ld\n", Magnitude(left));
	left.c.Free();
}

void Part2() {
	int64_t m = 0;

	for (int i = 0; i < numbers.Length(); i++) {
		for (int j = 0; j < numbers.Length(); j++) {
			if (i == j) {
				continue;
			}

			Number sum = Add(numbers[i], numbers[j]);
			int64_t x = Magnitude(sum);
			MAXIMIZE(m, x);
			sum.c.Free();
		}
	}

	printf("part 2: %ld\n", m);
}

int main() {
	FILE *f = fopen("in18.txt", "rb");

	while (true) {
		char line[1024];

		if (fscanf(f, "%s\n", line) == -1) {
			break;
		}

		Number number = {};

		for (int i = 0; line[i]; i++) {
			if (line[i] == '[') {
				number.c.Add(OPEN);
			} else if (line[i] == ']') {
				number.c.Add(CLOSE);
			} else if (line[i] == ',') {
				number.c.Add(COMMA);
			} else {
				number.c.Add(line[i] - '0');
			}
		}

		numbers.Add(number);
	}

	fclose(f);

	Part1();
	Part2();

	for (int i = 0; i < numbers.Length(); i++) {
		numbers[i].c.Free();
	}

	numbers.Free();
}
