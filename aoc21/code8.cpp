#include "ds.h"

char fileBuffer[65536];

uint8_t digits[10] = {
	0b1110111, 0b0100100, 0b1011101, 0b1101101, 0b0101110, 
	0b1101011, 0b1111011, 0b0100101, 0b1111111, 0b1101111,
};

void Part1() {
	int count = 0;

	char *position = fileBuffer;

	while (true) {
		position = strchr(position, '|');
		if (!position) break;
		position += 2;

		while (true) {
			int size = 0;

			while (true) {
				size++, position++;

				if (*position == ' ' || *position == '\n') {
					break;
				}
			}

			if (size == 2 || size == 4 || size == 3 || size == 7) {
				count++;
			}

			if (*position == '\n') {
				break;
			}

			position++;
		}
	}

	printf("part 1: %d\n", count);
}

void Part2() {
	char *position = fileBuffer;

	int64_t totalSum = 0;

	while (true) {
		if (*position == 0) {
			break;
		}

		Array<uint8_t> words = {};

		while (true) {
			if (*position == '|') {
				break;
			}

			uint8_t word = 0;

			while (true) {
				char c = *position++;

				word |= 1 << (c - 'a');

				if (*position == ' ' || *position == '\n') {
					break;
				}
			}

			// printf("%x\n", word);

			words.Add(word);

			position++;
		}

		int assignments[7] = {};
		bool foundValidAssignment = false;

		while (true) {
			bool valid = true;

			for (int i = 0; i < 7; i++) {
				for (int j = 0; j < i; j++) {
					if (assignments[i] == assignments[j]) {
						valid = false;
						break;
					}
				}
			}

			for (int i = 0; i < words.Length(); i++) {
				uint8_t word = words[i];
				uint8_t mapped = 0;

				for (int j = 0; j < 7; j++) {
					if (word & (1 << j)) {
						mapped |= 1 << assignments[j];
					}
				}

				if (mapped == digits[0] || mapped == digits[1] || mapped == digits[2] || mapped == digits[3] || mapped == digits[4]
						|| mapped == digits[5] || mapped == digits[6] || mapped == digits[7] || mapped == digits[8] || mapped == digits[9]) {
					// Valid digit!
				} else {
					valid = false;
					break;
				}
			}

			if (valid) {
				foundValidAssignment = true;
				break;
			}

			for (int i = 0; i < 7; i++) {
				assignments[i]++;

				if (assignments[i] == 7) {
					assert(i != 6);
					assignments[i] = 0;
				} else {
					break;
				}
			}
		}

		words.Free();

		assert(foundValidAssignment);

		// printf("--\n");

		position += 2;

		int value = 0;

		while (true) {
			uint8_t mapped = 0;

			while (true) {
				char c = *position++;
				mapped |= 1 << assignments[c - 'a'];

				if (*position == ' ' || *position == '\n') {
					break;
				}
			}

			value *= 10;
			bool found = false;

			for (int i = 0; i < 10; i++) {
				if (mapped == digits[i]) {
					found = true;
					value += i;
					break;
				}
			}

			assert(found);

			if (*position == '\n') {
				break;
			}

			position++;
		}

		// printf("%d\n", value);

		totalSum += value;

		position++;

		// printf("\n\n");
	}

	printf("part 2: %ld\n", totalSum);
}

int main() {
	FILE *f = fopen("in8.txt", "rb");
	fread(fileBuffer, 1, sizeof(fileBuffer), f);
	fclose(f);

	Part1();
	Part2();
}
