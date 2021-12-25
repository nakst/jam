#include "ds.h"

// The input file contains the following procedure repeated 14 times:
//	inp w   
//	mul x 0 
//	add x z 
//	mod x 26
//	div z {1 or 26}
//	add x [l1]
//	eql x w 
//	eql x 0 
//	mul y 0 
//	add y 25
//	mul y x 
//	add y 1 
//	mul z y 
//	mul y 0 
//	add y w 
//	add y [l2] 
//	mul y x 
//	add z y 
//
// Which is the same as:
// 	w = input_digit()
// 	x = (z % 26) + l1 != w
// 	z /= 1; or z /= 26;
//	z = z * (25 * x + 1) + x * (w + l2)
//
// If x is 0, then the last line is equivalent to
//	z = z * 1 + 0 = z
// If x is 1, then it is equivalent to
//	z = z * 26 + (w + l2)
//
// Based on the input data, we see that 0 <= w + l2 < 26.
// We also see that the division by 26 happens if and only if l1 < 0.
// We also see that exactly half of the iterations have the division by 26.
// We also see that whenever l1 > 0, l1 >= 10.
// 	This means x will always be 1.
//
// **Assume that we need to get x = 0 for every time l1 < 0.**

#define OP_INPUT_DIGIT (-1)
#define OP_ADD (1)
#define OP_MUL (2)
#define OP_DIV (3)
#define OP_MOD (4)
#define OP_EQL (5)

struct Instruction {
	int op, left, right, lit;
};

Array<Instruction> instructions = {};
int64_t literals[14][2];
bool div26[14];

void ParseInput() {
	instructions.Free();
	FILE *f = fopen("in24.txt", "rb");

	while (true) {
		char mn[10];

		if (fscanf(f, "%s", mn) == -1) {
			break;
		}

		if (0 == strcmp(mn, "inp")) {
			char c;
			fscanf(f, " %c\n", &c);
			assert(c >= 'w' && c <= 'z');
			c -= 'w';
			instructions.Add({ .op = OP_INPUT_DIGIT, .left = c, .right = 0, .lit = false });
		} else if (0 == strcmp(mn, "add") || 0 == strcmp(mn, "mul") || 0 == strcmp(mn, "div") || 0 == strcmp(mn, "mod") || 0 == strcmp(mn, "eql")) {
			char c;
			char d[10];
			fscanf(f, " %c %s\n", &c, d);
			assert(c >= 'w' && c <= 'z');
			c -= 'w';
			int op = 0 == strcmp(mn, "add") ? OP_ADD : 0 == strcmp(mn, "mul") ? OP_MUL : 0 == strcmp(mn, "div") ? OP_DIV 
				: 0 == strcmp(mn, "mod") ? OP_MOD : 0 == strcmp(mn, "eql") ? OP_EQL : 0;
			assert(op);

			if (d[0] == '-' || isdigit(d[0])) {
				instructions.Add({ .op = op, .left = c, .right = atoi(d), .lit = true });
			} else {
				assert(d[0] >= 'w' && d[0] <= 'z');
				instructions.Add({ .op = op, .left = (int) c, .right = d[0] - 'w', .lit = false });
			}
		} else {
			assert(false);
		}
	}

	fclose(f);

	int cd26 = 0;

	for (int i = 0; i < 14; i++) {
		div26[i] = instructions[i * 18 + 4].right == 26;
		literals[i][0] = instructions[i * 18 + 5].right;
		literals[i][1] = instructions[i * 18 + 15].right;

		assert(instructions[i * 18 + 0].op == OP_INPUT_DIGIT && instructions[i * 18 + 0].left == 0);
		assert(instructions[i * 18 + 1].op == OP_MUL && instructions[i * 18 + 1].left == 1 && instructions[i * 18 + 1].right == 0 && instructions[i * 18 + 1].lit);
		assert(instructions[i * 18 + 2].op == OP_ADD && instructions[i * 18 + 2].left == 1 && instructions[i * 18 + 2].right == 3 && !instructions[i * 18 + 2].lit);
		assert(instructions[i * 18 + 3].op == OP_MOD && instructions[i * 18 + 3].left == 1 && instructions[i * 18 + 3].right == 26 && instructions[i * 18 + 3].lit);
		assert(instructions[i * 18 + 4].op == OP_DIV && instructions[i * 18 + 4].left == 3 && instructions[i * 18 + 4].lit);
		assert(instructions[i * 18 + 4].right == 26 || instructions[i * 18 + 4].right == 1);
		assert(instructions[i * 18 + 5].op == OP_ADD && instructions[i * 18 + 5].left == 1 && instructions[i * 18 + 5].lit);
		assert(instructions[i * 18 + 6].op == OP_EQL && instructions[i * 18 + 6].left == 1 && instructions[i * 18 + 6].right == 0 && !instructions[i * 18 + 6].lit);
		assert(instructions[i * 18 + 7].op == OP_EQL && instructions[i * 18 + 7].left == 1 && instructions[i * 18 + 7].right == 0 && instructions[i * 18 + 7].lit);
		assert(instructions[i * 18 + 8].op == OP_MUL && instructions[i * 18 + 8].left == 2 && instructions[i * 18 + 8].right == 0 && instructions[i * 18 + 8].lit);
		assert(instructions[i * 18 + 9].op == OP_ADD && instructions[i * 18 + 9].left == 2 && instructions[i * 18 + 9].right == 25 && instructions[i * 18 + 9].lit);
		assert(instructions[i * 18 + 10].op == OP_MUL && instructions[i * 18 + 10].left == 2 && instructions[i * 18 + 10].right == 1 && !instructions[i * 18 + 10].lit);
		assert(instructions[i * 18 + 11].op == OP_ADD && instructions[i * 18 + 11].left == 2 && instructions[i * 18 + 11].right == 1 && instructions[i * 18 + 11].lit);
		assert(instructions[i * 18 + 12].op == OP_MUL && instructions[i * 18 + 12].left == 3 && instructions[i * 18 + 12].right == 2 && !instructions[i * 18 + 12].lit);
		assert(instructions[i * 18 + 13].op == OP_MUL && instructions[i * 18 + 13].left == 2 && instructions[i * 18 + 13].right == 0 && instructions[i * 18 + 13].lit);
		assert(instructions[i * 18 + 14].op == OP_ADD && instructions[i * 18 + 14].left == 2 && instructions[i * 18 + 14].right == 0 && !instructions[i * 18 + 14].lit);
		assert(instructions[i * 18 + 15].op == OP_ADD && instructions[i * 18 + 15].left == 2 && instructions[i * 18 + 15].lit);
		assert(instructions[i * 18 + 16].op == OP_MUL && instructions[i * 18 + 16].left == 2 && instructions[i * 18 + 16].right == 1 && !instructions[i * 18 + 16].lit);
		assert(instructions[i * 18 + 17].op == OP_ADD && instructions[i * 18 + 17].left == 3 && instructions[i * 18 + 17].right == 2 && !instructions[i * 18 + 17].lit);

		assert(div26[i] == (literals[i][0] < 0));
		assert(literals[i][0] < 0 || literals[i][0] >= 10);
		if (div26[i]) cd26++;
	}

	assert(cd26 == 7);
}

int64_t Evaluate(int64_t *data) {
	int64_t variables[4] = {};
	int inputIndex = 0;

	for (int i = 0; i < instructions.Length(); i++) {
		if (instructions[i].op == OP_INPUT_DIGIT) {
			variables[instructions[i].left] = data[inputIndex++];
		} else if (instructions[i].op == OP_ADD) {
			variables[instructions[i].left] += instructions[i].lit ? instructions[i].right : variables[instructions[i].right];
		} else if (instructions[i].op == OP_MUL) {
			variables[instructions[i].left] *= instructions[i].lit ? instructions[i].right : variables[instructions[i].right];
		} else if (instructions[i].op == OP_DIV) {
			variables[instructions[i].left] /= instructions[i].lit ? instructions[i].right : variables[instructions[0].right];
		} else if (instructions[i].op == OP_MOD) {
			variables[instructions[i].left] %= instructions[i].lit ? instructions[i].right : variables[instructions[i].right];
		} else if (instructions[i].op == OP_EQL) {
			variables[instructions[i].left] = variables[instructions[i].left] == (instructions[i].lit ? instructions[i].right : variables[instructions[i].right]);
		}
	}

	return variables[3];
}

bool Problem(int64_t *input, int upto) {
	// **Assume that we need to get x = 0 for every time l1 < 0.**

	int64_t z = 0; // z = zq * 26 + zr
	assert(upto >= 0 && upto <= 13);

	for (int i = 0; i <= upto; i++) {
		bool x = (z % 26) + literals[i][0] != input[i];
		if (div26[i]) { z /= 26; if (x) return true; }
		if (x) { z = z * 26 + input[i] + literals[i][1]; }
	}

	return false;
}

void Part1() {
	int64_t input[] = { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 };
	int upto = 0;

	while (true) {
		while (input[upto] == 1) {
			input[upto] = 10;
			upto--;
		} 
		
		input[upto]--;

		if (!Problem(input, upto)) {
			if (upto == 13) {
				break;
			} else {
				upto++;
			}
		}
	}
	
	assert(Evaluate(input) == 0);
	printf("part 1: ");
	for (int i = 0; i < 14; i++) printf("%ld", input[i]);
	printf("\n");
}

void Part2() {
	int64_t input[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int upto = 0;

	while (true) {
		while (input[upto] == 9) {
			input[upto] = 0;
			upto--;
		} 
		
		input[upto]++;

		if (!Problem(input, upto)) {
			if (upto == 13) {
				break;
			} else {
				upto++;
			}
		}
	}
	
	assert(Evaluate(input) == 0);
	printf("part 2: ");
	for (int i = 0; i < 14; i++) printf("%ld", input[i]);
	printf("\n");
}

int main() {
	ParseInput();
	Part1();
	Part2();
	instructions.Free();
}
