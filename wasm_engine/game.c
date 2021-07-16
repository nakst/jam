Texture testTexture, testTexture2;
int x = 0, y = 0;

void GameInitialise() {
	testTexture = TextureCreate(test_png, test_pngBytes);
	testTexture2 = TextureCreate(test2_png, test2_pngBytes);
	PlaySound("track1.opus", 11, true);
}

void GameUpdate() {
	if (keysHeld[KEY_LEFT]) {
		x -= 4;
	} else if (keysHeld[KEY_RIGHT]) {
		x += 4;
	}

	if (keysHeld[KEY_UP]) {
		y -= 4;
	} else if (keysHeld[KEY_DOWN]) {
		y += 4;
	}
}

void GameRender() {
	for (uintptr_t i = 0; i < GAME_HEIGHT; i++) {
		uint32_t color = 0xFF000000 | ((i * 255 / GAME_HEIGHT) << 16);

		for (uintptr_t j = 0; j < GAME_WIDTH; j++) {
			imageData[i * GAME_WIDTH + j] = color | ((j & 3) ? 0x80 : 0xFF) | ((i & 3) ? 0x8000 : 0x00);
		}
	}

	Draw(testTexture, x, y);
	Draw(testTexture2, 10 + 150, 10);
}
