#define TWALL   (1)
#define TPLATE  (2)
#define THINT   (3)
#define TPLAYER (4)
#define TCRATE1 (5)
#define TCRATE2 (6)
#define TCRATE3 (7)
#define TCRATE4 (8)
#define TCRATE5 (9)

typedef struct Particle {
	int32_t x, y, dy, rot;
} Particle;

typedef struct GameData {
	Particle particles[100];
	uint8_t over[25][25];
	uint8_t under[25][25];
	int px;
	int py;
	float sx;
	float sy;
	int hint;
	bool win;
	bool didPush;
	int winTimer;
} GameData;

Texture tileset;
GameData gd;

static void GameInitialise() {
	tileset = TextureCreate(tileset_png, sizeof(tileset_png));

	for (uint32_t i = 0, j = 0, k = 0; i < sizeof(over_csv); i++) {
		if (over_csv[i] == '\n' || over_csv[i] == ',') {
			gd.over[j % 25][j / 25] = k < 10 ? k : 0;

			if (k == TPLAYER) {
				gd.px = j % 25;
				gd.py = j / 25;
			}

			k = 0, j++;
		} else {
			k = (k * 10) + over_csv[i] - '0';
		}
	}

	for (uint32_t i = 0, j = 0, k = 0; i < sizeof(under_csv); i++) {
		if (under_csv[i] == '\n' || under_csv[i] == ',') {
			gd.under[j % 25][j / 25] = k < 10 ? k : 0;
			k = 0, j++;
		} else {
			k = (k * 10) + under_csv[i] - '0';
		}
	}

	PLAY_SOUND("audio/bgm.opus", true, 0.6);

	// Preload sound effects.
	PLAY_SOUND("audio/plate.wav", false, 0.0);
	PLAY_SOUND("audio/pull.wav", false, 0.0);
	PLAY_SOUND("audio/push.wav", false, 0.0);
	PLAY_SOUND("audio/bump.wav", false, 0.0);
	PLAY_SOUND("audio/hint.wav", false, 0.0);
	PLAY_SOUND("audio/win.opus", false, 0.0);
}

#define PLATE_SFX() if (gd.over[x + dx][y + dy] >= TCRATE1 \
		&& gd.over[x + dx][y + dy] <= TCRATE5 \
		&& gd.under[x + dx][y + dy] == TPLATE) { PLAY_SOUND("audio/plate.wav", false, 0.6); }

static bool Push(int x, int y, int dx, int dy) {
	int v = gd.over[x][y] ?: gd.under[x][y];

	if (v == TWALL || v == THINT) {
		PLAY_SOUND("audio/bump.wav", false, 0.6);
		return false;
	} else if (v == 0 || v == TPLATE) {
		return true;
	} else {
		if (!Push(x + dx, y + dy, dx, dy)) { return false; }
		if (v >= TCRATE1 && v <= TCRATE5) { gd.didPush = true; }
		gd.over[x][y] = 0;
		gd.over[x + dx][y + dy] = v;
		PLATE_SFX();
		return true;
	}
}

static bool Pull(int x, int y, int dx, int dy) {
	if (!Push(x, y, dx, dy)) {
		PLAY_SOUND("audio/bump.wav", false, 0.6);
		return false;
	}

	bool didPull = false;

	while (true) {
		x -= dx;
		y -= dy;

		if (gd.over[x][y] >= TCRATE1 && gd.over[x][y] <= TCRATE5) {
			gd.over[x + dx][y + dy] = gd.over[x][y];
			gd.over[x][y] = 0;
			PLATE_SFX();
			didPull = true;
		} else {
			if (didPull) { PLAY_SOUND("audio/pull.wav", false, 0.6); }
			else if (gd.didPush) { PLAY_SOUND("audio/push.wav", false, 0.6); }
			return true;
		}
	}
}

static float FABSF(float x) {
	return x > 0 ? x : -x;
}

static void GameUpdate() {
	int dx = 0;
	int dy = 0;

	if (gd.win) {
		if (gd.winTimer == 0) {
			for (uint32_t i = 0; i < sizeof(gd.particles) / sizeof(gd.particles[0]); i++) {
				gd.particles[i].x = GetRandomByte();
				gd.particles[i].y = -FABSF(GetRandomByte() * 300);
				gd.particles[i].dy = GetRandomByte();
				gd.particles[i].rot = GetRandomByte();
			}

			PLAY_SOUND("audio/bgm.opus", -1, 0.0);
			PLAY_SOUND("audio/win.opus", false, 0.6);
		}

		if (gd.winTimer >= 60) {
			for (uint32_t i = 0; i < sizeof(gd.particles) / sizeof(gd.particles[0]); i++) {
				gd.particles[i].y += gd.particles[i].dy;
			}
		}

		gd.winTimer++;
	} else if (keysHeld[KEY_LEFT]) {
		keysHeld[KEY_LEFT] = 0;
		dx = -1;
	} else if (keysHeld[KEY_RIGHT]) {
		keysHeld[KEY_RIGHT] = 0;
		dx = 1;
	} else if (keysHeld[KEY_UP]) {
		keysHeld[KEY_UP] = 0;
		dy = -1;
	} else if (keysHeld[KEY_DOWN]) {
		keysHeld[KEY_DOWN] = 0;
		dy = 1;
	}

	if (dx || dy) {
		if (gd.hint) {
			gd.hint = 0;
		} else if (gd.under[gd.px + dx][gd.py + dy] == THINT) {
			gd.hint = (gd.px + dx) + (gd.py + dy) * 25;
			PLAY_SOUND("audio/hint.wav", false, 0.6);
		} else if (keysHeld[KEY_SHIFT]) {
			if (Pull(gd.px, gd.py, dx, dy)) {
				gd.px += dx;
				gd.py += dy;
			}
		} else {
			gd.didPush = false;

			if (Push(gd.px, gd.py, dx, dy)) {
				gd.px += dx;
				gd.py += dy;
				if (gd.didPush) { PLAY_SOUND("audio/push.wav", false, 0.6); }
			}
		}

		uint8_t solution[] = {
			0, 2, 3, 0, 5,
			0, 0, 0, 3, 4,
			3, 0, 4, 0, 0,
			0, 3, 0, 5, 1,
			5, 4, 1, 2, 0,
		};

		gd.win = true;

		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				if (gd.under[i * 5 + 2][j * 5 + 2] == TPLATE) {
					if (!solution[i + j * 5]) { Panic(); }

					if (gd.over[i * 5 + 2][j * 5 + 2]
							!= TCRATE1 + solution[i + j * 5] - 1) {
						gd.win = false;
					}
				} else {
					if (solution[i + j * 5]) { Panic(); }
				}
			}
		}
	}

	if (gd.winTimer > 60) {
		gd.sx += ((gd.winTimer + 1) & 3) * 0.25f;
		gd.sy += (gd.winTimer & 3) * 0.25f;
	}

	int sxt = ((gd.px / 5) * 5 - 5) * 5;
	int syt = ((gd.py / 5) * 5 - 5) * 5;
	gd.sx += (sxt - gd.sx) * 0.2f;
	gd.sy += (syt - gd.sy) * 0.2f;
	if (FABSF(sxt - gd.sx) < 0.5f) { gd.sx = sxt; }
	if (FABSF(syt - gd.sy) < 0.5f) { gd.sy = syt; }
}

static void ShowHint(const char *string) {
	for (int y = 0; y < 5; y++) {
		for (int x = 0; x < 5; x++) {
			char c = string[x + y * 5];
			if (c == ' ') { continue; }
			int mx = (c - 'A') % 5;
			int my = (c - 'A') / 5;

			for (int j = 0; j < 5; j++) {
				for (int i = 0; i < 5; i++) {
					int v = gd.under[mx * 5 + i][my * 5 + j];
					Fill(25 + x * 5 + i, 25 + y * 5 + j, 1, 1,
							v == TWALL || v == THINT ? 0x5555ff
							: i == 0 || i == 4 || j == 0 || j == 4
							? 0xaa0000 : 0);
				}
			}
		}
	}
}

static void GameRender() {
	Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x555555);

	int32_t winOverYOffset = gd.winTimer > 180 ? (gd.winTimer - 180) / 11 : 0;

	for (int y = 0; y < 25; y++) {
		for (int x = 0; x < 25; x++) {
			if (gd.under[x][y]) {
				Blit(TextureSub(tileset, (gd.under[x][y] % 5) * 5,
							(gd.under[x][y] / 5) * 5, 5, 5),
						x * 5 - gd.sx, y * 5 - gd.sy);
			} else {
				if (x % 5 == 0 || x % 5 == 4 || y % 5 == 0 || y % 5 == 4) {
					Blit(TextureSub(tileset, 0, 35, 5, 5), x * 5 - gd.sx, y * 5 - gd.sy);
				} else {
					Blit(TextureSub(tileset, 0, 0, 5, 5), x * 5 - gd.sx, y * 5 - gd.sy);
				}
			}

			if (gd.over[x][y]) {
				Blit(TextureSub(tileset, (gd.over[x][y] % 5) * 5,
							(gd.over[x][y] / 5) * 5, 5, 5),
						x * 5 - gd.sx, y * 5 - gd.sy - winOverYOffset);
			}
		}
	}

	if (gd.winTimer) {
		if (gd.winTimer >= 60) {
			for (uint32_t i = 0; i < sizeof(gd.particles) / sizeof(gd.particles[0]); i++) {
				uint8_t n = ((gd.particles[i].rot + gd.winTimer / 5) % 3);
				Blend(TextureSub(tileset, 5 + n * 5, 35, 5, 5),
						gd.particles[i].x % 90 - 5, gd.particles[i].y >> 8, 0xff);
			}
		}

		for (int i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
			if (gd.winTimer > 360) {
				imageData[i] = 0xff000000;
			} else if (gd.winTimer > 300) {
				if ((imageData[i] & 0xffffff) == 0x555555) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0x0000aa) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0x0055aa) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0x5555ff) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0x55ffff) { imageData[i] = 0xff555555; }
				if ((imageData[i] & 0xffffff) == 0xaa0000) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0xaa00aa) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0xff55ff) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0xaaaaaa) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0xffffff) { imageData[i] = 0xffaa0000; }
			} else if (gd.winTimer > 240) {
				if ((imageData[i] & 0xffffff) == 0x555555) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0x0000aa) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0x0055aa) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0x5555ff) { imageData[i] = 0xff555555; }
				if ((imageData[i] & 0xffffff) == 0x55ffff) { imageData[i] = 0xff0000aa; }
				if ((imageData[i] & 0xffffff) == 0xaa0000) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0xaa00aa) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0xff55ff) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0xaaaaaa) { imageData[i] = 0xffaa0000; }
				if ((imageData[i] & 0xffffff) == 0xffffff) { imageData[i] = 0xffaa00aa; }
			} else if (gd.winTimer > 180) {
				if ((imageData[i] & 0xffffff) == 0x555555) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0x0000aa) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0x0055aa) { imageData[i] = 0xff555555; }
				if ((imageData[i] & 0xffffff) == 0x5555ff) { imageData[i] = 0xff0000aa; }
				if ((imageData[i] & 0xffffff) == 0x55ffff) { imageData[i] = 0xff0055aa; }
				if ((imageData[i] & 0xffffff) == 0xaa0000) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0xaa00aa) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0xff55ff) { imageData[i] = 0xffaa0000; }
				if ((imageData[i] & 0xffffff) == 0xaaaaaa) { imageData[i] = 0xffaa00aa; }
				if ((imageData[i] & 0xffffff) == 0xffffff) { imageData[i] = 0xffff55ff; }
			} else if (gd.winTimer > 120) {
				if ((imageData[i] & 0xffffff) == 0x555555) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0x0000aa) { imageData[i] = 0xff555555; }
				if ((imageData[i] & 0xffffff) == 0x0055aa) { imageData[i] = 0xff0000aa; }
				if ((imageData[i] & 0xffffff) == 0x5555ff) { imageData[i] = 0xff0055aa; }
				if ((imageData[i] & 0xffffff) == 0x55ffff) { imageData[i] = 0xff5555ff; }
				if ((imageData[i] & 0xffffff) == 0xaa0000) { imageData[i] = 0xff000000; }
				if ((imageData[i] & 0xffffff) == 0xaa00aa) { imageData[i] = 0xffaa0000; }
				if ((imageData[i] & 0xffffff) == 0xff55ff) { imageData[i] = 0xffaa00aa; }
				if ((imageData[i] & 0xffffff) == 0xaaaaaa) { imageData[i] = 0xffff55ff; }
				if ((imageData[i] & 0xffffff) == 0xffffff) { imageData[i] = 0xffaaaaaa; }
			}
		}
	}

	if (gd.hint || gd.winTimer > 480) {
		Fill(23, 23, 29, 29, 0);
		Fill(24, 24, 27, 27, 0xffffff);
		Fill(25, 25, 25, 25, 0);

		if (gd.win) {
			ShowHint("YOU  WIN  THANKFOR  PLAYN");
		} else if (gd.hint == 1) {
			ShowHint("EACH ONCE IN   EVERYROW  ");
		} else if (gd.hint == 17) {
			ShowHint("HOLD SHIFTTO   PULL BLOCK");
		} else if (gd.hint == 603) {
			ShowHint("EACH ONCE IN   EVERYCOLMN");
		} else if (gd.hint == 524) {
			Blit(TextureSub(tileset, 0, 10, 25, 25), 25, 25);
		}
	}
}
