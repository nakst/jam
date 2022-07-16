#define SLOT_USED       (1 << 0)
#define SLOT_DESTROYING (1 << 1)

#define FLAG_SOLID (1 << 0)
#define FLAG_MAIN  (1 << 1)

#define TAG_FLOOR (1)
#define TAG_GOAL  (2)

#define MSG_CREATE  (1)
#define MSG_DESTROY (2)
#define MSG_STEP    (3)
#define MSG_DRAW    (4)

typedef struct Entity {
	uint8_t slot;
	uint8_t tag;
	uint8_t layer;
	uint8_t flags;
	uint32_t uid;
	uint32_t tick;
	float x, y, w, h;
	int8_t frameCount, stepsPerFrame, texOffX, texOffY;
	Texture texture;
	void (*message)(struct Entity *entity, int message);

	union {
		struct {
			int target;
		} goal;
	};
} Entity;

#define MAX_ENTITIES (1000)
Entity entities[MAX_ENTITIES];

Entity *player;
int dice[6]; // inner, outer, left, right, top, bottom

Texture tileset, font;
Texture diceFaces[7];

#define MOVEMENT_TIME (10)
int movementTick;
int movementX, movementY;
int movementQX, movementQY;
int wonTick;
int levelNameTick = 1;

char levelName[32];

int fontOffsetX[50];
int fontOffsetY[50];

void Load();

const char *level1[] = {
	"/////...............",
	"/////...............",
	"/////...............",
	"/////....####.......",
	"/////..###  #.......",
	".......#5   #.......",
	".......###p##.......",
	".........###........",
	"....................",
	"....................",
	"....................",
	"....................",
	"....................",
};

void NullMessage(Entity *entity, int message) {}

Entity *EntityCreate(Entity *templateEntity, float x, float y, uint32_t uid) {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = entities + i;
		if (entity->slot & SLOT_USED) continue;
		if (templateEntity) *entity = *templateEntity;
		else memset(entity, 0, sizeof(Entity));
		entity->slot |= SLOT_USED;
		if (!entity->frameCount) entity->frameCount = 1;
		if (!entity->stepsPerFrame) entity->stepsPerFrame = 1;
		entity->x = x;
		entity->y = y;
		if (!entity->w) entity->w = entity->texture.width;
		if (!entity->h) entity->h = entity->texture.height;
		entity->uid = uid;
		if (!entity->message) entity->message = NullMessage;
		entity->message(entity, MSG_CREATE);
		return entity;
	}
	
	static Entity fake = {};
	return &fake;
}

void EntityDestroy(Entity *entity) {
	if (~entity->slot & SLOT_DESTROYING) {
		entity->slot |= SLOT_DESTROYING;
		entity->message(entity, MSG_DESTROY);
	}
}

Entity *EntityFind(float x, float y, float w, float h, uint8_t tag, uint8_t flags, Entity *exclude) {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = entities + i;
		if (entity->slot != SLOT_USED) continue;
		if (tag && entity->tag != tag) continue;
		if ((entity->flags & flags) != flags) continue;
		if (entity == exclude) continue;
		if (x >= entity->x + entity->w || entity->x >= x + w) continue;
		if (y >= entity->y + entity->h || entity->y >= y + h) continue;
		return entity;
	}
	
	return NULL;
}

void Player(Entity *entity, int message) {
	if (message == MSG_DRAW) {
		if (movementTick) {
			float t = ((float) movementTick / MOVEMENT_TIME);
			int mdx = movementX * 16 * t;
			int mdy = movementY * 16 * t;

			if (movementY == 1) {
				BlitScaled(diceFaces[dice[1]], entity->x + mdx, entity->y + mdy * 2, 16, 16 - mdy);
				BlitScaled(diceFaces[dice[4]], entity->x + mdx, entity->y + mdy, 16, mdy);
			} else if (movementY == -1) {
				BlitScaled(diceFaces[dice[1]], entity->x + mdx, entity->y + mdy, 16, 16 + mdy);
				BlitScaled(diceFaces[dice[5]], entity->x + mdx, entity->y + 16 + mdy * 2, 16, -mdy);
			} else if (movementX == 1) {
				BlitScaled(diceFaces[dice[1]], entity->x + mdx * 2, entity->y + mdy, 16 - mdx, 16);
				BlitScaled(diceFaces[dice[2]], entity->x + mdx, entity->y + mdy, mdx, 16);
			} else if (movementX == -1) {
				BlitScaled(diceFaces[dice[1]], entity->x + mdx, entity->y + mdy, 16 + mdx, 16);
				BlitScaled(diceFaces[dice[3]], entity->x + 16 + mdx * 2, entity->y + mdy, -mdx, 16);
			}
		} else if (wonTick) {
			BlendScaled(diceFaces[dice[1]], entity->x - wonTick * wonTick, entity->y - wonTick * wonTick, 
					16 + wonTick * wonTick * 2, 16 + wonTick * wonTick * 2, 0xFF - wonTick * 0x10);
		} else {
			Blit(diceFaces[dice[1]], entity->x, entity->y);
		}
	}
}

void Controller(Entity *entity, int message) {
	if (message == MSG_DRAW) {
		if (levelNameTick == 0) {
			int mdx = movementX * movementTick * 16 / MOVEMENT_TIME;
			int mdy = movementY * movementTick * 16 / MOVEMENT_TIME;

			if (movementTick) Blit(diceFaces[dice[0]], 2 * 16 + mdx - 2 * 16 * movementX, 2 * 16);
			if (movementTick) Blit(diceFaces[dice[0]], 2 * 16, 2 * 16 + mdy - 2 * 16 * movementY);
			Blit(diceFaces[dice[1]], 2 * 16 + mdx, 2 * 16 + mdy);
			Blit(diceFaces[dice[2]], 1 * 16 + mdx, 2 * 16);
			Blit(diceFaces[dice[3]], 3 * 16 + mdx, 2 * 16);
			Blit(diceFaces[dice[4]], 2 * 16, 1 * 16 + mdy);
			Blit(diceFaces[dice[5]], 2 * 16, 3 * 16 + mdy);

			Blit(TextureSub(tileset, 2 * 16, 5 * 16, 16, 16), 0 * 16, 0 * 16);
			Blit(TextureSub(tileset, 4 * 16, 4 * 16, 16, 16), 1 * 16, 0 * 16);
			Blit(TextureSub(tileset, 4 * 16, 4 * 16, 16, 16), 2 * 16, 0 * 16);
			Blit(TextureSub(tileset, 4 * 16, 4 * 16, 16, 16), 3 * 16, 0 * 16);
			Blit(TextureSub(tileset, 3 * 16, 5 * 16, 16, 16), 4 * 16, 0 * 16);
			Blit(TextureSub(tileset, 4 * 16, 5 * 16, 16, 16), 0 * 16, 4 * 16);
			Blit(TextureSub(tileset, 5 * 16, 4 * 16, 16, 16), 1 * 16, 4 * 16);
			Blit(TextureSub(tileset, 5 * 16, 4 * 16, 16, 16), 2 * 16, 4 * 16);
			Blit(TextureSub(tileset, 5 * 16, 4 * 16, 16, 16), 3 * 16, 4 * 16);
			Blit(TextureSub(tileset, 5 * 16, 5 * 16, 16, 16), 4 * 16, 4 * 16);
			Blit(TextureSub(tileset, 2 * 16, 4 * 16, 16, 16), 0 * 16, 1 * 16);
			Blit(TextureSub(tileset, 2 * 16, 4 * 16, 16, 16), 0 * 16, 2 * 16);
			Blit(TextureSub(tileset, 2 * 16, 4 * 16, 16, 16), 0 * 16, 3 * 16);
			Blit(TextureSub(tileset, 3 * 16, 4 * 16, 16, 16), 4 * 16, 1 * 16);
			Blit(TextureSub(tileset, 3 * 16, 4 * 16, 16, 16), 4 * 16, 2 * 16);
			Blit(TextureSub(tileset, 3 * 16, 4 * 16, 16, 16), 4 * 16, 3 * 16);
		} else {
			int lc = 4;
			for (int i = 0; levelName[i]; i++) if (levelName[i] == '\n') lc++;

			int x = 0;
			int y = GAME_HEIGHT / 2 - lc * 12 / 2;

			for (int i = 0; levelName[i]; i++) {
				char c = levelName[i];

				if (!x) {
					int lc = 0;

					for (int j = i; levelName[j] && levelName[j] != '\n'; j++) {
						lc++;
					}

					x = GAME_WIDTH / 2 - lc * 8 / 2;
				}

				if (c == '\n') {
					y += 12;
					x = 0;
				} else {
					Texture texture = { 0 };

					if (c >= 'a' && c <= 'l') {
						texture = TextureSub(font, (c - 'a') * 8, 8 * 0, 8, 8);
					} else if (c >= 'm' && c <= 'x') {
						texture = TextureSub(font, (c - 'm') * 8, 8 * 1, 8, 8);
					} else if (c >= 'y' && c <= 'z') {
						texture = TextureSub(font, (c - 'y') * 8, 8 * 2, 8, 8);
					} else if (c >= '0' && c <= '9') {
						texture = TextureSub(font, (c - '0') * 8 + 8 * 2, 8 * 2, 8, 8);
					}

					Blit(texture, x + fontOffsetX[i % 50], y + fontOffsetY[i % 50]);
					x += 8;
				}
			}
		}
	} else if (message == MSG_STEP) {
		if (levelNameTick != 0) {
			if (levelNameTick == 1) {
				Load();
			}

			levelNameTick++;

			if (levelNameTick == 240) {
				levelNameTick = 0;
			}

			if ((levelNameTick % 40) == 0) {
				for (int i = 0; i < 50; i++) {
					fontOffsetX[i] = GetRandomByte() & 1;
					fontOffsetY[i] = GetRandomByte() & 1;
				}
			}
		} else if (wonTick != 0) {
			wonTick++;

			if (wonTick == 15) {
				wonTick = 0;
				levelNameTick = 1;

				for (int i = 0; i < MAX_ENTITIES; i++) {
					if ((entities[i].slot & SLOT_USED) && entities[i].message != Controller) {
						EntityDestroy(&entities[i]);
					}
				}
			}
		} else if (movementTick == 0) {
			if (keysHeld[KEY_LEFT]) {
				movementTick = 1;
				movementX = -1;
				movementY = 0;
			} else if (keysHeld[KEY_RIGHT]) {
				movementTick = 1;
				movementX = 1;
				movementY = 0;
			} else if (keysHeld[KEY_UP]) {
				movementTick = 1;
				movementX = 0;
				movementY = -1;
			} else if (keysHeld[KEY_DOWN]) {
				movementTick = 1;
				movementX = 0;
				movementY = 1;
			} else if (movementQX || movementQY) {
				movementTick = 1;
				movementX = movementQX;
				movementY = movementQY;
			}

			if (movementTick) {
				// Decide if the move is valid.
				int newX = player->x + movementX * 16;
				int newY = player->y + movementY * 16;
				bool valid = true;

				if (EntityFind(newX, newY, 1, 1, 0, FLAG_SOLID, NULL)) {
					valid = false;
				}

				if (!valid) {
					movementTick = 0;
					movementQX = movementQY = 0;
					PlaySound("audio/bump_sfx.wav", 18, false, 1.0);
				}
			}
		} else {
			movementTick++;

			if (movementTick >= MOVEMENT_TIME - 1) {
				if (keysHeld[KEY_LEFT]) {
					movementQX = -1;
					movementQY = 0;
				} else if (keysHeld[KEY_RIGHT]) {
					movementQX = 1;
					movementQY = 0;
				} else if (keysHeld[KEY_UP]) {
					movementQX = 0;
					movementQY = -1;
				} else if (keysHeld[KEY_DOWN]) {
					movementQX = 0;
					movementQY = 1;
				}
			} else {
				movementQX = movementQY = 0;
			}

			if (movementTick == MOVEMENT_TIME) {
				movementTick = 0;

				if (movementX == -1) {
					int t = dice[0];
					dice[0] = dice[2];
					dice[2] = dice[1];
					dice[1] = dice[3];
					dice[3] = t;
					player->x -= 16;
				} else if (movementX == 1) {
					int t = dice[0];
					dice[0] = dice[3];
					dice[3] = dice[1];
					dice[1] = dice[2];
					dice[2] = t;
					player->x += 16;
				} else if (movementY == -1) {
					int t = dice[0];
					dice[0] = dice[4];
					dice[4] = dice[1];
					dice[1] = dice[5];
					dice[5] = t;
					player->y -= 16;
				} else if (movementY == 1) {
					int t = dice[0];
					dice[0] = dice[5];
					dice[5] = dice[1];
					dice[1] = dice[4];
					dice[4] = t;
					player->y += 16;
				}

				Entity *goal = EntityFind(player->x, player->y, 1, 1, TAG_GOAL, 0, NULL);

				if (goal && dice[1] == goal->goal.target) {
					wonTick = 1;
					PlaySound("audio/win_sfx.wav", 17, false, 0.8);
				}

				{
					static int prior = 0;
					int r = 0;
					while (r == prior) r = GetRandomByte() % 3;
					prior = r;
					if (r == 0) PlaySound("audio/dice_roll_1.wav", 21, false, 1.0);
					if (r == 1) PlaySound("audio/dice_roll_2.wav", 21, false, 1.0);
					if (r == 2) PlaySound("audio/dice_roll_3.wav", 21, false, 1.0);
				}
			}
		}
	}
}

void Wall(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (entity->tick == 0) {
			bool l = EntityFind(entity->x - 16, entity->y, 1, 1, 0, FLAG_MAIN, NULL);
			bool r = EntityFind(entity->x + 16, entity->y, 1, 1, 0, FLAG_MAIN, NULL);
			bool t = EntityFind(entity->x, entity->y - 16, 1, 1, 0, FLAG_MAIN, NULL);
			bool b = EntityFind(entity->x, entity->y + 16, 1, 1, 0, FLAG_MAIN, NULL);

			if (l) entity->texture = TextureSub(tileset, 3 * 16, 1 * 16, 16, 16);
			if (r) entity->texture = TextureSub(tileset, 2 * 16, 1 * 16, 16, 16);
			if (t) entity->texture = TextureSub(tileset, 4 * 16, 1 * 16, 16, 16);
			if (b) entity->texture = TextureSub(tileset, 5 * 16, 1 * 16, 16, 16);
			if (l && t) entity->texture = TextureSub(tileset, 3 * 16, 3 * 16, 16, 16);
			if (l && b) entity->texture = TextureSub(tileset, 5 * 16, 3 * 16, 16, 16);
			if (r && t) entity->texture = TextureSub(tileset, 2 * 16, 3 * 16, 16, 16);
			if (r && b) entity->texture = TextureSub(tileset, 4 * 16, 3 * 16, 16, 16);
		}
	}
}

void Load() {
	memcpy(levelName, "level 1\nit begins", 18);
	dice[0] = 1;
	dice[1] = 6;
	dice[2] = 2;
	dice[3] = 5;
	dice[4] = 3;
	dice[5] = 4;

	for (int i = 0; i < 13; i++) {
		for (int j = 0; j < 20; j++) {
			char c = level1[i][j];
			if (c == '.') continue;
			Entity *tile = EntityCreate(NULL, j * 16, i * 16, 0);

			if (c == ' ' || c == 'p') {
				tile->texture = TextureSub(tileset, 1 * 16, 1 * 16, 16, 16);
				tile->w = tile->h = 16;
				tile->tag = TAG_FLOOR;
				tile->flags |= FLAG_MAIN;
			}

			if (c == '#') {
				tile->w = tile->h = 16;
				tile->flags |= FLAG_SOLID;
				tile->message = Wall;
			}

			if (c == 'p') {
				player = EntityCreate(NULL, j * 16, i * 16, 0);
				player->message = Player;
				player->layer = 1;
				player->w = player->h = 16;
			}

			if (c >= '1' && c <= '6') {
				tile->w = tile->h = 16;
				tile->texture = TextureSub(tileset, 16 * (c - '1'), 16 * 2, 16, 16);
				tile->tag = TAG_GOAL;
				tile->goal.target = c - '0';
				tile->flags |= FLAG_MAIN;
			}
		}
	}
}

void GameInitialise(uint32_t save) {
	tileset = TextureCreate(tileset_png, tileset_pngBytes);
	font = TextureCreate(font_png, font_pngBytes);
	diceFaces[1] = TextureSub(tileset, 0 * 16, 0 * 16, 16, 16);
	diceFaces[2] = TextureSub(tileset, 1 * 16, 0 * 16, 16, 16);
	diceFaces[3] = TextureSub(tileset, 2 * 16, 0 * 16, 16, 16);
	diceFaces[4] = TextureSub(tileset, 3 * 16, 0 * 16, 16, 16);
	diceFaces[5] = TextureSub(tileset, 4 * 16, 0 * 16, 16, 16);
	diceFaces[6] = TextureSub(tileset, 5 * 16, 0 * 16, 16, 16);

	Entity *controller = EntityCreate(NULL, 0, 0, 0);
	controller->message = Controller;
	controller->layer = 3;

	PlaySound("audio/bgm.opus", 14, true, 0.5);
}

void GameUpdate() {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = entities + i;

		if (entity->slot & SLOT_USED) {
			entity->message(entity, MSG_STEP);
			entity->tick++;
		}
	}

	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = entities + i;

		if ((entity->slot & SLOT_USED) && (entity->slot & SLOT_DESTROYING)) {
			entity->slot &= ~SLOT_USED;
		}
	}
}

void GameRender() {
	Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x222323);

	for (int layer = -1; layer <= 3; layer++) {
		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity *entity = entities + i;
			if ((~entity->slot & SLOT_USED) || (entity->layer != layer)) continue;
			if (levelNameTick != 0 && entity->message != Controller) continue;

			if (entity->texture.width && entity->texture.height) {
				uint32_t frame = entity->stepsPerFrame >= 0 
					? ((entity->tick / entity->stepsPerFrame) % entity->frameCount) 
					: (-entity->stepsPerFrame - 1);
				entity->texture.bits += frame * entity->texture.width * entity->texture.stride;
				Blend(entity->texture, entity->x + entity->texOffX, entity->y + entity->texOffY, 0xFF - wonTick * 0x10);
			}

			entity->message(entity, MSG_DRAW);
		}
	}
}
