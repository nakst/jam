#include "ds.h"

void Part1() {
	int p1, p2, d = 1, dr = 0, s1 = 0, s2 = 0, t = 0;
	FILE *f = fopen("in21.txt", "rb");
	fscanf(f, "Player 1 starting position: %d\n", &p1);
	fscanf(f, "Player 2 starting position: %d\n", &p2);
	fclose(f);

	while (s1 < 1000 && s2 < 1000) {
		int m = d + ((d + 1) % 100) + ((d + 2) % 100);
		d = (d + 3) % 100;
		dr += 3;

		if (t) {
			p2 = ((p2 + m - 1) % 10) + 1;
			s2 += p2;
		} else {
			p1 = ((p1 + m - 1) % 10) + 1;
			s1 += p1;
		}

		t = (t + 1) % 2;
	}

	printf("part 1: %d\n", (s1 < 1000 ? s1 : s2) * dr);
}

void CountWins(int p1, int p2, int s1, int s2, int t, uint64_t multiplier, uint64_t *player1Wins, uint64_t *player2Wins) {
	int pn;

	if (s1 >= 21) {
		*player1Wins += multiplier;
	} else if (s2 >= 21) {
		*player2Wins += multiplier;
	} else if (t) {
		pn = (((p2 - 1) + 3) % 10) + 1; CountWins(p1, pn, s1, s2 + pn, 1 - t, multiplier * 1, player1Wins, player2Wins);
		pn = (((p2 - 1) + 4) % 10) + 1; CountWins(p1, pn, s1, s2 + pn, 1 - t, multiplier * 3, player1Wins, player2Wins);
		pn = (((p2 - 1) + 5) % 10) + 1; CountWins(p1, pn, s1, s2 + pn, 1 - t, multiplier * 6, player1Wins, player2Wins);
		pn = (((p2 - 1) + 6) % 10) + 1; CountWins(p1, pn, s1, s2 + pn, 1 - t, multiplier * 7, player1Wins, player2Wins);
		pn = (((p2 - 1) + 7) % 10) + 1; CountWins(p1, pn, s1, s2 + pn, 1 - t, multiplier * 6, player1Wins, player2Wins);
		pn = (((p2 - 1) + 8) % 10) + 1; CountWins(p1, pn, s1, s2 + pn, 1 - t, multiplier * 3, player1Wins, player2Wins);
		pn = (((p2 - 1) + 9) % 10) + 1; CountWins(p1, pn, s1, s2 + pn, 1 - t, multiplier * 1, player1Wins, player2Wins);
	} else {
		pn = (((p1 - 1) + 3) % 10) + 1; CountWins(pn, p2, s1 + pn, s2, 1 - t, multiplier * 1, player1Wins, player2Wins);
		pn = (((p1 - 1) + 4) % 10) + 1; CountWins(pn, p2, s1 + pn, s2, 1 - t, multiplier * 3, player1Wins, player2Wins);
		pn = (((p1 - 1) + 5) % 10) + 1; CountWins(pn, p2, s1 + pn, s2, 1 - t, multiplier * 6, player1Wins, player2Wins);
		pn = (((p1 - 1) + 6) % 10) + 1; CountWins(pn, p2, s1 + pn, s2, 1 - t, multiplier * 7, player1Wins, player2Wins);
		pn = (((p1 - 1) + 7) % 10) + 1; CountWins(pn, p2, s1 + pn, s2, 1 - t, multiplier * 6, player1Wins, player2Wins);
		pn = (((p1 - 1) + 8) % 10) + 1; CountWins(pn, p2, s1 + pn, s2, 1 - t, multiplier * 3, player1Wins, player2Wins);
		pn = (((p1 - 1) + 9) % 10) + 1; CountWins(pn, p2, s1 + pn, s2, 1 - t, multiplier * 1, player1Wins, player2Wins);
	}
}

void Part2() {
	int p1, p2;
	FILE *f = fopen("in21.txt", "rb");
	fscanf(f, "Player 1 starting position: %d\n", &p1);
	fscanf(f, "Player 2 starting position: %d\n", &p2);
	fclose(f);
	uint64_t player1Wins = 0, player2Wins = 0;
	CountWins(p1, p2, 0, 0, 0, 1, &player1Wins, &player2Wins);
	MAXIMIZE(player1Wins, player2Wins);
	printf("part 2: %ld\n", player1Wins);
}

int main() {
	Part1();
	Part2();
}
