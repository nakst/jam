#include "ds.h"

struct Board {
	int numbers[5][5];
	bool seen[5][5];
	bool alreadyWon;
};

void Part1() {
	FILE *f = fopen("in4.txt", "rb");
	Array<int> calledNumbers = {};
	Array<Board> boards = {};

	while (true) {
		int number = 0;
		fscanf(f, "%d", &number);
		calledNumbers.Add(number);
		char c = fgetc(f);
		if (c == '\n') break;
	}

	for (int i = 0; i < 100; i++) {
		Board board = {};

		for (int j = 0; j < 5; j++) {
			for (int k = 0; k < 5; k++) {
				assert(fscanf(f, "%d", &board.numbers[j][k]) != -1);
			}
		}

		boards.Add(board);
	}

	for (int i = 0; i < calledNumbers.Length(); i++) {
		for (int j = 0; j < boards.Length(); j++) {
			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					if (boards[j].numbers[k][l] == calledNumbers[i]) {
						boards[j].seen[k][l] = true;
					}
				}
			}

			for (int k = 0; k < 5; k++) {
				bool completeRow = true;
				bool completeColumn = true;

				for (int l = 0; l < 5; l++) {
					if (!boards[j].seen[k][l]) {
						completeRow = false;
					} 
					
					if (!boards[j].seen[l][k]) {
						completeColumn = false;
					}
				}

				if (completeRow || completeColumn) {
					int sumOfUnmarkedNumbers = 0;

					for (int l = 0; l < 5; l++) {
						for (int m = 0; m < 5; m++) {
							if (!boards[j].seen[l][m]) {
								sumOfUnmarkedNumbers += boards[j].numbers[l][m];
							}
						}
					}

					printf("part 1: %d\n", sumOfUnmarkedNumbers * calledNumbers[i]);
					goto done;
				}
			}
		}
	}

	done:;
	fclose(f);
	calledNumbers.Free();
	boards.Free();
}

void Part2() {
	FILE *f = fopen("in4.txt", "rb");
	Array<int> calledNumbers = {};
	Array<Board> boards = {};
	int finalScore = 0;

	while (true) {
		int number = 0;
		fscanf(f, "%d", &number);
		calledNumbers.Add(number);
		char c = fgetc(f);
		if (c == '\n') break;
	}

	for (int i = 0; i < 100; i++) {
		Board board = {};

		for (int j = 0; j < 5; j++) {
			for (int k = 0; k < 5; k++) {
				assert(fscanf(f, "%d", &board.numbers[j][k]) != -1);
			}
		}

		boards.Add(board);
	}

	for (int i = 0; i < calledNumbers.Length(); i++) {
		for (int j = 0; j < boards.Length(); j++) {
			if (boards[j].alreadyWon) {
				continue;
			}

			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					if (boards[j].numbers[k][l] == calledNumbers[i]) {
						boards[j].seen[k][l] = true;
					}
				}
			}

			for (int k = 0; k < 5; k++) {
				bool completeRow = true;
				bool completeColumn = true;

				for (int l = 0; l < 5; l++) {
					if (!boards[j].seen[k][l]) {
						completeRow = false;
					} 
					
					if (!boards[j].seen[l][k]) {
						completeColumn = false;
					}
				}

				if (completeRow || completeColumn) {
					int sumOfUnmarkedNumbers = 0;

					for (int l = 0; l < 5; l++) {
						for (int m = 0; m < 5; m++) {
							if (!boards[j].seen[l][m]) {
								sumOfUnmarkedNumbers += boards[j].numbers[l][m];
							}
						}
					}

					finalScore = sumOfUnmarkedNumbers * calledNumbers[i];
					boards[j].alreadyWon = true;
				}
			}
		}
	}

	printf("part 2: %d\n", finalScore);
	fclose(f);
	calledNumbers.Free();
	boards.Free();
}

int main() {
	Part1();
	Part2();
}
