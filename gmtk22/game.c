#define SLOT_USED       (1 << 0)
#define SLOT_DESTROYING (1 << 1)

#define FLAG_SOLID (1 << 0)
#define FLAG_MAIN  (1 << 1)
#define FLAG_PUSH  (1 << 2)
#define FLAG_HEAVY (1 << 3)

#define TAG_FLOOR  (1)
#define TAG_CHECK  (2)
#define TAG_GOAL   (3)
#define TAG_ICE    (4)
#define TAG_BUTTON (5)
#define TAG_DOOR   (6)

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
	float x, y, w, h, dx, dy;
	int8_t frameCount, stepsPerFrame, frameRow, texOffX, texOffY;
	Texture texture;
	void (*message)(struct Entity *entity, int message);

	union {
		struct { int target; } check;
		struct { bool pushing; } push; 
	};
} Entity;

#define MAX_ENTITIES (500)
Entity entities[MAX_ENTITIES];

Entity templateStar;

Entity *player;
int dice[6]; // inner, outer, left, right, top, bottom

Texture tileset, font, textureStar;
Texture diceFaces[7];

#define MOVEMENT_TIME (12)
int movementTick;
int movementX, movementY;
int movementQX, movementQY;
int wonTick;
int levelNameTick = 1;

char levelName[32];

int fontOffsetX[50];
int fontOffsetY[50];

void Load();

#if 0
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
#endif

const char *level1[] = {
	"/////.########......",
	"/////.#     #.......",
	"/////.#b c  #.######",
	"/////.#     ###    #",
	"/////.### p iiii ii#",
	"........##d#########",
	".........#6#........",
	".........# #........",
	".........#g#........",
	"..........#.........",
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
		if (!entity->frameRow) entity->frameRow = 1;
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
			int w = wonTick * wonTick / 20;
			BlendScaled(diceFaces[dice[1]], entity->x - w, entity->y - w, 16 + w * 2, 16 + w * 2, 0xFF - wonTick * 0x6);
		} else {
			Blit(diceFaces[dice[1]], entity->x, entity->y);
		}
	}
}

void Controller(Entity *entity, int message) {
	if (message == MSG_DRAW) {
		if (levelNameTick == 0 && wonTick == 0) {
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

			if (wonTick) {
				y += 10 - SmoothStep(wonTick / 45.0f) * 10;
			}

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

					Blend(texture, x + fontOffsetX[i % 50], y + fontOffsetY[i % 50], 
							wonTick == 0 ? 0xFF : wonTick * 5);
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

			if (levelNameTick == 180) {
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

			if (wonTick == 45) {
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
		} else if (movementTick == 1) {
			int newX = player->x + movementX * 16;
			int newY = player->y + movementY * 16;
			bool valid = true;

			while (true) {
				if (EntityFind(newX, newY, 1, 1, 0, FLAG_PUSH, NULL)) {
					newX += movementX * 16;
					newY += movementY * 16;
					continue;
				}

				if (EntityFind(newX, newY, 1, 1, 0, FLAG_SOLID, NULL)) {
					valid = false;
				}

				break;
			}
			
			if (!valid) {
				movementTick = 0;
				movementQX = movementQY = 0;
				PlaySound("audio/bump_sfx.wav", 18, false, 1.0);
			} else {
				movementTick++;

				newX = player->x + movementX * 16;
				newY = player->y + movementY * 16;

				if (EntityFind(newX, newY, 1, 1, TAG_ICE, 0, NULL)) {
					PlaySound("audio/slip_sfx.wav", 18, false, 1.0);
				}

				bool pushing = false;

				while (true) {
					Entity *push = EntityFind(newX, newY, 1, 1, 0, FLAG_PUSH, NULL);

					if (push) {
						push->push.pushing = true;
						pushing = true;
						newX += movementX * 16;
						newY += movementY * 16;
						continue;
					}

					break;
				}

				if (pushing) {
					PlaySound("audio/crate_sfx.wav", 19, false, 0.9);
				}
			}
		} else if (movementTick == MOVEMENT_TIME) {
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

			for (int i = 0; i < MAX_ENTITIES; i++) {
				if ((entities[i].slot & SLOT_USED) && (entities[i].flags & FLAG_PUSH) && entities[i].push.pushing) {
					entities[i].x += movementX * 16;
					entities[i].y += movementY * 16;
					entities[i].push.pushing = false;
				}
			}

			Entity *ice = EntityFind(player->x, player->y, 1, 1, TAG_ICE, 0, NULL);

			if (ice) {
				movementTick = 1;
			} else {
				Entity *goal = EntityFind(player->x, player->y, 1, 1, TAG_GOAL, 0, NULL);

				if (goal) {
					wonTick = 1;
					PlaySound("audio/win_sfx.wav", 17, false, 0.8);

					for (int i = 0; i < 20; i++) {
						Entity *entity = EntityCreate(&templateStar, player->x, player->y, 0);
						float speed = (GetRandomByte() / 255.0f) * 2.0f + 1.0f;
						float angle = (GetRandomByte() / 255.0f) * 6.25f;
						entity->dx = speed * EsCRTcosf(angle);
						entity->dy = speed * EsCRTsinf(angle);
					}
				}

				Entity *check = EntityFind(player->x, player->y, 1, 1, TAG_CHECK, 0, NULL);

				if (check && dice[1] != check->check.target) {
					movementX = -movementX;
					movementY = -movementY;
					movementTick = 1;
					PlaySound("audio/reject_sfx.wav", 20, false, 0.6);
				} else {
					static int prior = 0;
					int r = 0;
					while (r == prior) r = GetRandomByte() % 3;
					prior = r;
					if (r == 0) PlaySound("audio/dice_roll_1.wav", 21, false, 1.0);
					if (r == 1) PlaySound("audio/dice_roll_2.wav", 21, false, 1.0);
					if (r == 2) PlaySound("audio/dice_roll_3.wav", 21, false, 1.0);
				}

				bool doorsDown = false;
				static bool previousDoorsDown = false;

				for (int i = 0; i < MAX_ENTITIES; i++) {
					if ((entities[i].slot & SLOT_USED) && (entities[i].tag == TAG_BUTTON)) {
						if (EntityFind(entities[i].x, entities[i].y, 1, 1, 0, FLAG_HEAVY, NULL)) {
							doorsDown = !doorsDown;
						}
					}
				}

				if (previousDoorsDown != doorsDown) {
					PlaySound("audio/door_move_sfx.wav", 23, false, 0.6);
				}

				previousDoorsDown = doorsDown;

				for (int i = 0; i < MAX_ENTITIES; i++) {
					if ((entities[i].slot & SLOT_USED) && (entities[i].tag == TAG_DOOR)) {
						if (doorsDown) entities[i].flags &= ~FLAG_SOLID;
						else entities[i].flags |= FLAG_SOLID;
						entities[i].stepsPerFrame = doorsDown ? -2 : -1;
					}
				}
			}
		} else {
			movementTick++;

#if 0
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
#endif
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

			if (c == ' ' || c == 'p' || c == 'c') {
				tile->texture = TextureSub(tileset, 1 * 16, 1 * 16, 16, 16);
				tile->w = tile->h = 16;
				tile->tag = TAG_FLOOR;
				tile->flags |= FLAG_MAIN;
			}

			if (c == 'i') {
				tile->texture = TextureSub(tileset, 0 * 16, 4 * 16, 16, 16);
				tile->w = tile->h = 16;
				tile->tag = TAG_ICE;
				tile->flags |= FLAG_MAIN;
			}

			if (c == 'b') {
				tile->texture = TextureSub(tileset, 0 * 16, 5 * 16, 16, 16);
				tile->w = tile->h = 16;
				tile->tag = TAG_BUTTON;
				tile->flags |= FLAG_MAIN;
			}

			if (c == 'd') {
				tile->texture = TextureSub(tileset, 1 * 16, 5 * 16, 16, 16);
				tile->w = tile->h = 16;
				tile->tag = TAG_DOOR;
				tile->flags |= FLAG_MAIN | FLAG_SOLID;
				tile->frameCount = 2;
				tile->stepsPerFrame = -1;
			}

			if (c == 'c') {
				tile = EntityCreate(NULL, j * 16, i * 16, 0);
				tile->texture = TextureSub(tileset, 0 * 16, 3 * 16, 16, 16);
				tile->w = tile->h = 16;
				tile->flags |= FLAG_MAIN | FLAG_PUSH | FLAG_HEAVY;
				tile->layer = 1;
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
				player->flags |= FLAG_HEAVY;
			}

			if (c >= '1' && c <= '6') {
				tile->w = tile->h = 16;
				tile->texture = TextureSub(tileset, 16 * (c - '1'), 16 * 2, 16, 16);
				tile->tag = TAG_CHECK;
				tile->check.target = c - '0';
				tile->flags |= FLAG_MAIN;
			}

			if (c == 'g') {
				tile->w = tile->h = 16;
				tile->texture = TextureSub(tileset, 16 * 1, 16 * 3, 16, 16);
				tile->frameCount = 2;
				tile->stepsPerFrame = 30;
				tile->tag = TAG_GOAL;
				tile->flags |= FLAG_MAIN;
			}
		}
	}
}

void GameInitialise(uint32_t save) {
	tileset = TextureCreate(tileset_png, tileset_pngBytes);
	font = TextureCreate(font_png, font_pngBytes);
	textureStar = TextureCreate(star_png, star_pngBytes);

	diceFaces[1] = TextureSub(tileset, 0 * 16, 0 * 16, 16, 16);
	diceFaces[2] = TextureSub(tileset, 1 * 16, 0 * 16, 16, 16);
	diceFaces[3] = TextureSub(tileset, 2 * 16, 0 * 16, 16, 16);
	diceFaces[4] = TextureSub(tileset, 3 * 16, 0 * 16, 16, 16);
	diceFaces[5] = TextureSub(tileset, 4 * 16, 0 * 16, 16, 16);
	diceFaces[6] = TextureSub(tileset, 5 * 16, 0 * 16, 16, 16);

	templateStar.layer = 2;
	templateStar.texture = TextureSub(textureStar, 0, 0, 16, 16);
	templateStar.frameRow = 4;
	templateStar.stepsPerFrame = 4;
	templateStar.frameCount = 16;

	Entity *controller = EntityCreate(NULL, 0, 0, 0);
	controller->message = Controller;
	controller->layer = 3;

	// PlaySound("audio/bgm.opus", 14, true, 0.5);
}

void GameUpdate() {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = entities + i;

		if (entity->slot & SLOT_USED) {
			entity->x += entity->dx;
			entity->y += entity->dy;
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

			int x = entity->x + entity->texOffX;
			int y = entity->y + entity->texOffY;

			if ((entity->flags & FLAG_PUSH) && entity->push.pushing) {
				x += movementX * movementTick * 16 / MOVEMENT_TIME;
				y += movementY * movementTick * 16 / MOVEMENT_TIME;
			}

			if (entity->texture.width && entity->texture.height) {
				uint32_t frame = entity->stepsPerFrame >= 0 
					? ((entity->tick / entity->stepsPerFrame) % entity->frameCount) 
					: (-entity->stepsPerFrame - 1);
				Texture texture = entity->texture;
				texture.bits += (frame % entity->frameRow) * texture.width;
				texture.bits += (frame / entity->frameRow) * texture.height * texture.stride;
				Blend(texture, x, y, 0xFF - wonTick * 0x6);
			}

			entity->message(entity, MSG_DRAW);
		}
	}
}
