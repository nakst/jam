#define SLOT_USED       (1 << 0)
#define SLOT_DESTROYING (1 << 1)

#define MSG_STEP   (1)
#define MSG_DRAW   (2)
#define MSG_CREATE (3)

#define FLAG_SOLID (1 << 0)
#define FLAG_HIDE  (1 << 1)
#define FLAG_DEATH (1 << 2)
#define FLAG_BOOST (1 << 3)

#define TAG_GEM           (1)
#define TAG_SAVE          (2)
#define TAG_HELI          (3)
#define TAG_DOUBLE_JUMP   (4)
#define TAG_BOOST_UP      (5)
#define TAG_DOOR          (6)
#define TAG_SWITCH        (7)
#define TAG_DOOR_INVERSE  (8)
#define TAG_BOOST_DOWN    (9)
#define TAG_BOOST_LEFT   (10)
#define TAG_BOOST_RIGHT  (11)
#define TAG_EXIT         (12)
#define TAG_SECRET       (13)
#define TAG_BIG_GEM      (14)

typedef struct Entity {
	uint8_t slot;
	uint8_t tag;
	int8_t layer;
	uint8_t flags;
	uint8_t alpha;
	uint8_t extraData;
	int16_t timer;
	uint32_t uid;
	uint32_t tick;
	float x, y, w, h, dx, dy;
	int8_t frameCount, stepsPerFrame, frameRow, texOffX, texOffY;
	Texture texture;
	void (*message)(struct Entity *entity, int message);
} Entity;

#define MAX_ENTITIES (300)
Entity entities[MAX_ENTITIES];
Texture tileset, endingText;
Entity *player;

int gemsCollected;
int secretsCollected;
bool keyPressUp;
bool playerCanDoubleJump;
int didSaveTimer;
bool insideSave;
float saveX, saveY;
int saveRoomX, saveRoomY;
int saveGemsCollected;
int saveSecretsCollected;
uint32_t runFrameCount;
uint32_t bestRunFrameCount;
uint32_t deathCount;
uint32_t bestDeathCount;
uint32_t saveGemList[16];
int roomX, roomY;
bool loadRoom;
bool spawnParticlesAtPlayer;
uint32_t gemList[16];
float heliTimer;
int backgroundMotion;
int boosting;
float deathTimer;
bool hardMode;
bool endOfGame;
float endTimer;
int endTimer2;
bool showingEndScreen;
bool menuOpen;
int menuHighlighted;
uint64_t roomsVisited;
bool playerOnGround;
int globalTimer;
float bossRoomPosition;
float bossRoomTimer;
int bossRoomDirection;

Entity templateBlock;
Entity templateUpBlock;
Entity templateGem;
Entity templateSave;
Entity templateSaveEasy;
Entity templateDeath;
Entity templateDeathHard;
Entity templateDeathHorz;
Entity templateDeathVert;
Entity templateDoubleJump;
Entity templateHeli;
Entity templateBoostUp;
Entity templateDoor;
Entity templateDoorInverse;
Entity templateSwitch;
Entity templateBoostDown;
Entity templateBoostLeft;
Entity templateBoostRight;
Entity templateJumpHelper;
Entity templateExit;
Entity templateSecretBlock;
Entity templateSecret;
int roomData[GAME_HEIGHT / 16][GAME_WIDTH / 16];

char saveBuffer[256];

int colorOfRoom[10][10] = {
//        0  1  2  3  4  5  6  7  8  9
/* 0 */	{ 0, 5, 5, 5, 5, 5, 5, 0, 0, 0 },
/* 1 */	{ 1, 1, 1, 2, 2, 2, 5, 0, 0, 0 },
/* 2 */	{ 1, 0, 0, 2, 3, 2, 0, 0, 0, 0 },
/* 3 */	{ 1, 4, 4, 3, 3, 0, 0, 0, 0, 0 },
/* 4 */	{ 4, 4, 0, 3, 3, 6, 6, 0, 0, 0 },
/* 5 */	{ 6, 6, 6, 3, 3, 6, 6, 0, 0, 0 },
/* 6 */	{ 6, 6, 6, 6, 6, 6, 6, 0, 0, 0 },
/* 7 */	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/* 8 */	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/* 9 */	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

#ifdef UI_LINUX
bool editMode = true;
#else
bool editMode = false;
#endif
int selectedEntityIndex = 1;

Entity *templateList[] = {
	NULL,
	&templateBlock,
	&templateGem,
	&templateSave,
	&templateDeath,
	&templateUpBlock,
	&templateDoubleJump,
	&templateDeathHorz,
	&templateDeathVert,
	&templateHeli,
	&templateBoostUp,
	&templateDoor,
	&templateDoorInverse,
	&templateSwitch,
	&templateBoostLeft,
	&templateBoostRight,
	&templateSaveEasy,
	&templateDeathHard,
	&templateJumpHelper,
	&templateExit,
	&templateSecretBlock,
	&templateSecret,
};

uint64_t FNV1a(const void *key, size_t keyBytes) {
	uint64_t hash = 0xCBF29CE484222325;

	for (uintptr_t i = 0; i < keyBytes; i++) {
		hash = (hash ^ ((uint8_t *) key)[i]) * 0x100000001B3;
	}

	return hash;
}

uint32_t WriteU32InHex(char *out, uint32_t x) {
	out[0] = ((x >> 28) & 15) + 'A';
	out[1] = ((x >> 24) & 15) + 'A';
	out[2] = ((x >> 20) & 15) + 'A';
	out[3] = ((x >> 16) & 15) + 'A';
	out[4] = ((x >> 12) & 15) + 'A';
	out[5] = ((x >>  8) & 15) + 'A';
	out[6] = ((x >>  4) & 15) + 'A';
	out[7] = ((x >>  0) & 15) + 'A';
	return 8;
}

uint32_t ReadU32InHex(const char *in, uint32_t *x) {
	*x = 0
		| ((in[0] - 'A') << 28)
		| ((in[1] - 'A') << 24)
		| ((in[2] - 'A') << 20)
		| ((in[3] - 'A') << 16)
		| ((in[4] - 'A') << 12)
		| ((in[5] - 'A') <<  8)
		| ((in[6] - 'A') <<  4)
		| ((in[7] - 'A') <<  0);
	return 8;
}

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
		entity->alpha = 0xFF;
		if (!entity->message) entity->message = NullMessage;
		entity->message(entity, MSG_CREATE);
		return entity;
	}
	
	static Entity fake = {};
	return &fake;
}

void EntityDestroy(Entity *entity) {
	entity->slot |= SLOT_DESTROYING;
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

void LoadRoom() {
	if (roomY >= 10 || roomX < 0 || roomX > 10) {
		roomX = 2;
		roomY = 2;
		player->x = 206;
		player->y = 200;
	}

	if (roomX >= 0 && roomY >= 0 && roomX < 8 && roomY < 8) {
		roomsVisited |= (1 << (roomY * 8 + roomX));
	}

	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (&entities[i] != player) {
			entities[i].slot = 0;
		}
	}

#ifdef UI_LINUX
	char buf[64];
	snprintf(buf, sizeof(buf), "embed/r%d_%d.dat", roomX, roomY);
	FILE *f = fopen(buf, "rb");
	if (f) {
		fread(&roomData[0][0], 1, sizeof(roomData), f);
		fclose(f);
	} else {
		memset(roomData, 0, sizeof(roomData));
	}
#else
	char buf[64];
	buf[0] = 'r';
	buf[1] = roomX + '0';
	buf[2] = '_';
	buf[3] = roomY + '0';
	buf[4] = '_';
	buf[5] = 'd';
	buf[6] = 'a';
	buf[7] = 't';
	buf[8] = 0;

	bool found = false;

	for (uintptr_t i = 0; i < sizeof(embedItems) / sizeof(embedItems[0]); i++) {
		if (embedItems[i].name) {
			if (0 == strcmp(embedItems[i].name, buf)) {
				memcpy(roomData, embedItems[i].pointer, sizeof(roomData));
				found = true;
				break;
			}
		}
	}

	if (!found) { memset(roomData, 0, sizeof(roomData)); }
#endif

	for (int j = 0; j < GAME_HEIGHT / 16; j++) {
		for (int i = 0; i < GAME_WIDTH / 16; i++) {
			if (roomData[j][i]) {
				EntityCreate(templateList[roomData[j][i]], i * 16, j * 16, 
						(roomX << 24) | (roomY << 16) | (i << 8) | (j << 0));
			}
		}
	}

	if (roomX == 1 && roomY == 5) {
		Entity *entity = EntityCreate(NULL, GAME_WIDTH / 2 - 55 / 2, GAME_HEIGHT / 2 - 55 / 2, 0);
		entity->tag = TAG_BIG_GEM;
		entity->w = entity->h = 55;
		entity->texture = TextureSub(endingText, 9, 120, 55, 55);
	}

	bossRoomPosition = 0;
	bossRoomTimer = 0;

	if (roomY == 6) {
		bossRoomDirection = 1;
	} else if (roomX == 6 && roomY == 5) {
		bossRoomDirection = 2;
	} else if ((roomX == 5 || roomX == 6) && roomY == 4) {
		bossRoomDirection = 3;
	} else if (roomX == 2 && roomY == 5) {
		bossRoomDirection = 3;
	}
}

void Death(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		entity->w -= 4;
		entity->h -= 4;
		entity->x += 2;
		entity->y += 2;
		entity->texOffX -= 2;
		entity->texOffY -= 2;
	}
}

void DeathHard(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		Death(entity, message);

		if (!editMode && !hardMode) {
			EntityDestroy(entity);
		} else {
			entity->texture = TextureSub(tileset, 16, 0, 16, 16);
		}
	}
}

void DeathHorz(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		entity->w -= 4;
		entity->h -= 4;
		entity->x += 2;
		entity->y += 2;
		entity->texOffX -= 2;
		entity->texOffY -= 2;
		entity->dx = 2;
	} else if (message == MSG_STEP) {
		if (EntityFind(entity->x + entity->dx, entity->y, entity->w, entity->h, 0, FLAG_SOLID, NULL)) {
			entity->dx = -entity->dx;
			entity->texture = TextureSub(tileset, entity->dx > 0 ? 32 : 48, 16, 16, 16);
		}
	}
}

void DeathVert(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		entity->w -= 4;
		entity->h -= 4;
		entity->x += 2;
		entity->y += 2;
		entity->texOffX -= 2;
		entity->texOffY -= 2;
		entity->dy = 2;
	} else if (message == MSG_STEP) {
		if (EntityFind(entity->x, entity->y + entity->dy, entity->w, entity->h, 0, FLAG_SOLID, NULL)) {
			entity->dy = -entity->dy;
			entity->texture = TextureSub(tileset, entity->dy > 0 ? 64 : 80, 16, 16, 16);
		}
	}
}

void Heli(Entity *entity, int message) {
	if (message == MSG_STEP) {
		entity->alpha = heliTimer ? 0x40 : 0xFF;
	}
}

void SaveM(Entity *entity, int message) {
	if (message == MSG_STEP) {
		entity->alpha = insideSave ? 0x40 : 0xFF;
	}
}

void SaveEasyM(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		if (!editMode) {
			entity->texture = TextureSub(tileset, 64, 0, 16, 16);

			if (hardMode) {
				EntityDestroy(entity);
			}
		}
	} else if (message == MSG_STEP) {
		entity->alpha = insideSave ? 0x40 : 0xFF;
	}
}

void Gem(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		for (uintptr_t i = 0; i < sizeof(gemList) / sizeof(gemList[0]); i++) {
			if (gemList[i] == entity->uid) {
				EntityDestroy(entity);
			}
		}
	}
}

void DoubleJump(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (playerOnGround && entity->timer > 2) {
			entity->timer -= 2; // Restore a bit quicker if the player is waiting on the ground!
		}

		if (entity->timer) {
			entity->timer--;
			entity->layer = -100;
		} else {
			entity->layer = 0;
		}
	}
}

float /* -1..1 */ CheapSineApproximation01(float x /* 0..1 */) {
	return ((((-56.8889f * x + 142.2222f) * x - 103.1111f) * x + 12.4444f) * x + 5.3333f) * x;
}

void JumpHelper(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		entity->extraData = entity->x / 16;
		entity->layer = 1;
	} else if (message == MSG_STEP) {
		entity->timer++;
		if (entity->timer == 240) entity->timer = 0;
		entity->x = (float) entity->extraData * 16 - 100 * CheapSineApproximation01(entity->timer / 240.0f);
	}
}

void Block(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		entity->texture = TextureSub(tileset, 32 + 16 * colorOfRoom[roomY][roomX], 48, 16, 16);
	}
}

void Secret(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		if ((roomX == 3 && roomY == 1 && (secretsCollected & (1 << 0)))
				|| (roomX == 2 && roomY == 3 && (secretsCollected & (1 << 1)))
				|| (roomX == 0 && roomY == 1 && (secretsCollected & (1 << 2)))
				|| (roomX == 2 && roomY == 1 && (secretsCollected & (1 << 3)))) {
			EntityDestroy(entity);
		}
	} else if (message == MSG_STEP) {
		float dx = player->x - entity->x;
		float dy = player->y - entity->y;
		float dist = __builtin_sqrtf(dx * dx + dy * dy);
		float alpha = LinearMap(0, 48, 1, 0, dist);
		entity->alpha = alpha < 0.0f ? 0 : (alpha * 255.0f);
	}
}

void UpBlock(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		entity->h = 1;
	} else if (message == MSG_STEP) {
		if (player->dy >= 0 && player->y + player->h - 1 < entity->y) {
			entity->flags |= FLAG_SOLID;
		} else {
			entity->flags &= ~FLAG_SOLID;
		}
	}
}

void Particle(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (!entity->timer--) {
			EntityDestroy(entity);
		}
	}
}

void WriteNewGame() {
	uint32_t pos = 0;
	pos += WriteU32InHex(&saveBuffer[pos], 206);
	pos += WriteU32InHex(&saveBuffer[pos], 200);
	pos += WriteU32InHex(&saveBuffer[pos], 2);
	pos += WriteU32InHex(&saveBuffer[pos], 2);
	pos += WriteU32InHex(&saveBuffer[pos], 0);
	pos += WriteU32InHex(&saveBuffer[pos], 0);
	pos += WriteU32InHex(&saveBuffer[pos], 0);
	pos += WriteU32InHex(&saveBuffer[pos], bestRunFrameCount);
	pos += WriteU32InHex(&saveBuffer[pos], 0);
	pos += WriteU32InHex(&saveBuffer[pos], bestDeathCount);
	pos += WriteU32InHex(&saveBuffer[pos], 0);
	pos += WriteU32InHex(&saveBuffer[pos], 0);
	for (uintptr_t i = 0; i < sizeof(saveGemList) / sizeof(saveGemList[0]); i++) {
		pos += WriteU32InHex(&saveBuffer[pos], 0);
	}
	pos += WriteU32InHex(&saveBuffer[pos], FNV1a(&saveBuffer[0], pos));
	if (pos >= sizeof(saveBuffer)) Panic();
	SaveGameToCookie(saveBuffer, pos);
}

void DoDeath() {
	deathCount++;

	for (int i = 0; i < 20; i++) {
		Entity *particle = EntityCreate(NULL, player->x + player->w / 2, player->y + player->h / 2, 0);
		particle->dx = ((int8_t) GetRandomByte() / 128.0f);
		particle->dy = ((int8_t) GetRandomByte() / 128.0f);
		particle->texture = TextureSub(tileset, 5, 80, 3, 3);
		particle->layer = 4;
		particle->timer = 20;
		particle->message = Particle;
	}

	if (heliTimer) {
		PLAY_SOUND("audio/sfx_heli2.ogg", false, 0.0);
	}

	deathTimer = 1.0f;
	player->dx = 0;
	player->dy = -1;
	PLAY_SOUND("audio/sfx_death.wav", false, 0.5);
}

void Player(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (endOfGame) {
			if (endTimer < 1.0f) {
				endTimer += 0.005f;
			}

			endTimer2++;

			float t = (float) endTimer2 / 160.0f;
			entity->y += CheapSineApproximation01(t - __builtin_floorf(t)) / 11.0f;
			entity->dy = 0.0f;

			{
				Entity *particle = EntityCreate(NULL, entity->x + entity->w / 2, entity->y + entity->h / 2, 0);
				particle->dx = ((int8_t) GetRandomByte() / 128.0f);
				particle->dy = ((int8_t) GetRandomByte() / 128.0f);
				particle->texture = TextureSub(tileset, 10, 80, 3, 3);
				particle->timer = 100;
				particle->alpha = 128;
				particle->message = Particle;
			}

			return;
		} else {
			endTimer = 0;
			endTimer2 = 0;
		}

		runFrameCount++;

		// resetting

		if (deathTimer > 0.0f) {
			deathTimer -= 1.0f / 20.0f;
			entity->alpha = 255 * deathTimer;

			if (deathTimer <= 0.0f) {
				deathTimer = 0.0f;
				player->x = saveX, player->y = saveY;
				heliTimer = 0;
				player->texture = TextureSub(tileset, 0, 16, 16, 16);
				player->texOffX = -5;
				player->texOffY = -8;
				player->frameCount = 1;
				roomX = saveRoomX, roomY = saveRoomY;
				memcpy(&gemList[0], &saveGemList[0], sizeof(gemList));
				loadRoom = true;
				spawnParticlesAtPlayer = true;
				insideSave = true;
				player->dx = 0;
				player->dy = 0;
				gemsCollected = saveGemsCollected;
				secretsCollected = saveSecretsCollected;
			}
			
			return;
		} else {
			entity->alpha = 255;
		}

		if (spawnParticlesAtPlayer) {
			for (int i = 0; i < 20; i++) {
				Entity *particle = EntityCreate(NULL, entity->x + entity->w / 2, entity->y + entity->h / 2, 0);
				particle->dx = ((int8_t) GetRandomByte() / 128.0f);
				particle->dy = ((int8_t) GetRandomByte() / 128.0f);
				particle->texture = TextureSub(tileset, 10, 80, 3, 3);
				particle->layer = -1;
				particle->timer = 50;
				particle->alpha = 128;
				particle->message = Particle;
			}

			spawnParticlesAtPlayer = false;
		}

		// boosting

		Entity *boost = EntityFind(entity->x, entity->y, entity->w, entity->h, 0, FLAG_BOOST, NULL);
		int oldBoosting = boosting;

		if (boost && boost->tag == TAG_BOOST_UP) {
			player->dy = -6;
			boosting = 20;
		} else if (boost && boost->tag == TAG_BOOST_DOWN) {
			player->dy = 6;
			boosting = 20;
		} else if (boost && boost->tag == TAG_BOOST_LEFT) {
			player->dx = -6;
			player->dy = 0;
			boosting = 20;
		} else if (boost && boost->tag == TAG_BOOST_RIGHT) {
			player->dx = 6;
			player->dy = 0;
			boosting = 20;
		}

		if (oldBoosting < 15 && boosting == 20) {
			PLAY_SOUND("audio/sfx_boost.wav", false, 0.3);
		}

		if (boosting > 0) {
			boosting--;
		}

		if (boosting <= 0) {
			boosting = 0;
			entity->dx *= 0.9f;
		}

		if (entity->dx) {
			if (EntityFind(entity->x + entity->dx, entity->y, entity->w, entity->h, 0, FLAG_SOLID, NULL)) {
				entity->dx = 0;
				boosting = 0;
			}
		}

		// horizontal movement

		int oldX = entity->x;
		int speed = heliTimer ? 3 : 2;

		if (!boosting || !entity->dx) {
			if (keysHeld[KEY_LEFT] && !EntityFind(entity->x - speed, entity->y, entity->w, entity->h, 0, FLAG_SOLID, NULL)) {
				entity->x -= speed;
				entity->dx = 0;

				if (!heliTimer) {
					player->texture = TextureSub(tileset, 16, 16, 16, 16);
					player->texOffX = -4;
					player->texOffY = -8;
				}
			}

			if (keysHeld[KEY_RIGHT] && !EntityFind(entity->x + speed, entity->y, entity->w, entity->h, 0, FLAG_SOLID, NULL)) {
				entity->x += speed;
				entity->dx = 0;

				if (!heliTimer) {
					player->texture = TextureSub(tileset, 0, 16, 16, 16);
					player->texOffX = -5;
					player->texOffY = -8;
				}
			}
		}

		// jumping and gravity

		if (heliTimer) {
			entity->dy = 0;

			if (keysHeld[KEY_UP] && !EntityFind(entity->x, entity->y - speed, entity->w, entity->h, 0, FLAG_SOLID, NULL)) {
				entity->y -= speed;
			}

			if (keysHeld[KEY_DOWN] && !EntityFind(entity->x, entity->y + speed, entity->w, entity->h, 0, FLAG_SOLID, NULL)) {
				entity->y += speed;
			}
		} else {
			entity->dy += boosting && entity->dx ? 0.1f : keysHeld[KEY_DOWN] ? 0.35f : 0.25f;
			int maxDY = keysHeld[KEY_DOWN] ? 6 : 4;
			if (boosting) maxDY += 2;
			if (entity->dy > maxDY) entity->dy = maxDY;

			if (!(keysHeld[KEY_UP] || keysHeld['X'])) {
				keyPressUp = true;
			}

			bool onGround = EntityFind(entity->x, entity->y + 3, entity->w, entity->h, 0, FLAG_SOLID, NULL);
			playerOnGround = onGround;

			if (keyPressUp && (keysHeld[KEY_UP] || keysHeld['X'])
					&& (onGround
						|| EntityFind(oldX, entity->y + 3, entity->w, entity->h, 0, FLAG_SOLID, NULL)
						|| (oldX != entity->x && EntityFind(2 * oldX - entity->x, entity->y + 3, 
								entity->w + 1, entity->h, 0, FLAG_SOLID, NULL))
						/* coyote time + secret wall jump!! */
					)) {
				entity->dy = -__builtin_sqrtf(2.0f * gemsCollected + 8.0f);
				if (roomX == 2 && roomY == 5 && gemsCollected == 16) { entity->dy--; }
				keyPressUp = false;
				PLAY_SOUND("audio/sfx_jump.wav", false, 0.1);
			} else if (keyPressUp && (keysHeld[KEY_UP] || keysHeld['X']) && playerCanDoubleJump) {
				entity->dy = -__builtin_sqrtf(2.0f * gemsCollected + 8.0f) * 0.9f;
				playerCanDoubleJump = false;
				keyPressUp = false;
				PLAY_SOUND("audio/sfx_jump.wav", false, 0.1);
			}

			if (onGround) {
				playerCanDoubleJump = false;
			}

			if (!(keysHeld[KEY_UP] || keysHeld['X']) && entity->dy < -1 && !boosting) {
				entity->dy *= 0.5f;
			}

			Entity *solid = EntityFind(entity->x, entity->y + entity->dy, entity->w, entity->h, 0, FLAG_SOLID, NULL);

			if (solid) {
				if (entity->dy < 0) {
					entity->y = solid->y + solid->h;
				} else if (entity->dy > 0) {
					entity->y = solid->y - entity->h;
				}

				entity->dy = 0;
			}
		}

		// room switching

		if (entity->x < -4) {
			roomX--;
			entity->x += GAME_WIDTH - 4;
			loadRoom = true;
		} else if (entity->x + entity->w > GAME_WIDTH + 4) {
			roomX++;
			entity->x -= GAME_WIDTH - 4;
			loadRoom = true;
		}

		if (entity->y < -4) {
			roomY--;
			entity->y += GAME_HEIGHT - 4;
			loadRoom = true;
		} else if (entity->y + entity->h > GAME_HEIGHT + 4) {
			roomY++;
			entity->y -= GAME_HEIGHT - 4;
			loadRoom = true;
		}

		if (loadRoom) return;

		// collisions

		Entity *gem = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_GEM, 0, NULL);

		if (gem) {
			EntityDestroy(gem);
			PLAY_SOUND("audio/sfx_gem.wav", false, 0.6);

			for (uintptr_t i = 0; i < sizeof(gemList) / sizeof(gemList[0]); i++) {
				if (gemList[i] == gem->uid) {
					break;
				} else if (!gemList[i]) {
					gemList[i] = gem->uid;
					gemsCollected++;
					break;
				}
			}
		}

		Entity *bigGem = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_BIG_GEM, 0, NULL);

		if (bigGem) {
			PLAY_SOUND("audio/sfx_gem.wav", false, 0.6);
			PLAY_SOUND("audio/boss.opus", false, 0.0);
			endTimer2 = 3000;
			showingEndScreen = true;
			WriteNewGame();
			return;
		}

		Entity *secret = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_SECRET, 0, NULL);

		if (secret) {
			EntityDestroy(secret);
			PLAY_SOUND("audio/sfx_secret.wav", false, 0.7);
			if (roomX == 3 && roomY == 1) { secretsCollected |= (1 << 0); }
			if (roomX == 2 && roomY == 3) { secretsCollected |= (1 << 1); }
			if (roomX == 0 && roomY == 1) { secretsCollected |= (1 << 2); }
			if (roomX == 2 && roomY == 1) { secretsCollected |= (1 << 3); }
		}

		Entity *save = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_SAVE, 0, NULL);

		if (save) {
			if (!insideSave) {
				didSaveTimer = 60;
				saveX = player->x, saveY = player->y;
				saveRoomX = roomX, saveRoomY = roomY;
				saveGemsCollected = gemsCollected;
				saveSecretsCollected = secretsCollected;
				memcpy(&saveGemList[0], &gemList[0], sizeof(gemList));

				uint32_t pos = 0;
				pos += WriteU32InHex(&saveBuffer[pos], (uint32_t) saveX | (hardMode ? 0x10000 : 0));
				pos += WriteU32InHex(&saveBuffer[pos], saveY);
				pos += WriteU32InHex(&saveBuffer[pos], saveRoomX);
				pos += WriteU32InHex(&saveBuffer[pos], saveRoomY);
				pos += WriteU32InHex(&saveBuffer[pos], saveGemsCollected);
				pos += WriteU32InHex(&saveBuffer[pos], saveSecretsCollected);
				pos += WriteU32InHex(&saveBuffer[pos], runFrameCount);
				pos += WriteU32InHex(&saveBuffer[pos], bestRunFrameCount);
				pos += WriteU32InHex(&saveBuffer[pos], deathCount);
				pos += WriteU32InHex(&saveBuffer[pos], bestDeathCount);
				pos += WriteU32InHex(&saveBuffer[pos], roomsVisited & 0xFFFFFFFF);
				pos += WriteU32InHex(&saveBuffer[pos], roomsVisited >> 32);
				for (uintptr_t i = 0; i < sizeof(saveGemList) / sizeof(saveGemList[0]); i++) {
					pos += WriteU32InHex(&saveBuffer[pos], saveGemList[i]);
				}
				pos += WriteU32InHex(&saveBuffer[pos], FNV1a(&saveBuffer[0], pos));
				if (pos >= sizeof(saveBuffer)) Panic();
				SaveGameToCookie(saveBuffer, pos);

				PLAY_SOUND("audio/sfx_save.wav", false, 0.4);
			}

			insideSave = true;
		} else {
			insideSave = false;
		}

		Entity *death = EntityFind(entity->x, entity->y, entity->w, entity->h, 0, FLAG_DEATH, NULL);

		if (death 
				|| (bossRoomDirection == 1 && entity->x + entity->w / 2 < GAME_WIDTH * bossRoomPosition)
				|| (bossRoomDirection == 2 && entity->y + entity->h / 2 > GAME_HEIGHT - GAME_HEIGHT * bossRoomPosition)
				|| (bossRoomDirection == 3 && entity->x + entity->w / 2 > GAME_WIDTH - GAME_WIDTH * bossRoomPosition)) {
			DoDeath();
		}

		Entity *doubleJump = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_DOUBLE_JUMP, 0, NULL);

		if (doubleJump && doubleJump->timer == 0) {
			doubleJump->timer = 180;
			playerCanDoubleJump = true;
		}

		if (playerCanDoubleJump) {
			for (int i = 0; i < 3; i++) {
				Entity *particle = EntityCreate(NULL, entity->x + entity->w / 2, entity->y + entity->h / 2, 0);
				particle->dx = ((int8_t) GetRandomByte() / 128.0f);
				particle->dy = ((int8_t) GetRandomByte() / 128.0f);
				particle->texture = TextureSub(tileset, 0, 80, 3, 3);
				particle->layer = 4;
				particle->timer = 10;
				particle->message = Particle;
			}
		}

		Entity *heli = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_HELI, 0, NULL);

		if (heli && !heliTimer) {
			PLAY_SOUND("audio/sfx_heli2.ogg", false, 0.5);
			heliTimer = 1.0f;
			entity->texture = TextureSub(tileset, 96, 16, 16, 16);
			entity->frameCount = 2;
			entity->frameRow = 2;
			entity->stepsPerFrame = 4;
			player->texOffX = -4;
			player->texOffY = -4;
		} else if (heliTimer) {
			heliTimer -= 0.35f / GAME_FPS;

			if (heliTimer < 0) {
				heliTimer = 0;
				entity->texture = TextureSub(tileset, 0, 16, 16, 16);
				entity->frameCount = 1;
				player->texOffX = -5;
				player->texOffY = -8;
			}
		}

		Entity *switch_ = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_SWITCH, 0, NULL);

		if (switch_) {
			EntityDestroy(switch_);
			PLAY_SOUND("audio/sfx_switch.wav", false, 0.7);

			for (int i = 0; i < MAX_ENTITIES; i++) {
				Entity *e = &entities[i];

				if (e->slot == SLOT_USED && e->tag == TAG_DOOR) {
					e->tag = TAG_DOOR_INVERSE;
					e->flags = 0;
					e->texture = TextureSub(tileset, 48, 32, 16, 16);
				} else if (e->slot == SLOT_USED && e->tag == TAG_DOOR_INVERSE) {
					e->tag = TAG_DOOR;
					e->flags = FLAG_SOLID;
					e->texture = TextureSub(tileset, 16, 32, 16, 16);
				}
			}
		}

		Entity *exit = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_EXIT, 0, NULL);

		if (exit) {
			EntityDestroy(exit);
			endOfGame = true;
			endTimer = 0.0f;
			endTimer2 = 0;
			player->texture = TextureSub(tileset, 0, 16, 16, 16);
			player->texOffX = -5;
			player->texOffY = -8;
			PLAY_SOUND("audio/bgm.opus", false, 0.0);
			PLAY_SOUND("audio/sfx_win.opus", false, 0.5);
		}
	}
}

void GameInitialise(const char *saveData) {
	{
		memcpy(saveBuffer, saveData, sizeof(saveBuffer));
		uint32_t hash, index = (12 + 16) * 8;
		ReadU32InHex(&saveBuffer[index], &hash);

		if (hash == (uint32_t) FNV1a(&saveBuffer[0], index)) {
			uint32_t pos = 0;
			uint32_t _saveX, _saveY;
			uint32_t roomsVisitedLow, roomsVisitedHi;
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &_saveX);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &_saveY);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &saveRoomX);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &saveRoomY);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &saveGemsCollected);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &saveSecretsCollected);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &runFrameCount);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &bestRunFrameCount);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &deathCount);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &bestDeathCount);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &roomsVisitedLow);
			pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &roomsVisitedHi);

			for (uintptr_t i = 0; i < sizeof(saveGemList) / sizeof(saveGemList[0]); i++) {
				pos += ReadU32InHex(&saveBuffer[pos], (uint32_t *) &saveGemList[i]);
			}

			hardMode = _saveX & 0x10000;
			saveX = _saveX & 0xFFFF;
			saveY = _saveY;
			insideSave = true;
			gemsCollected = saveGemsCollected;
			secretsCollected = saveSecretsCollected;
			roomsVisited = (uint64_t) roomsVisitedLow | ((uint64_t) roomsVisitedHi << 32);
			memcpy(&gemList[0], &saveGemList[0], sizeof(gemList));
			spawnParticlesAtPlayer = true;
		} else {
			hardMode = false;
			saveX = 206;
			saveY = 200;
			saveRoomX = 2;
			saveRoomY = 2;
			saveGemsCollected = 0;
			saveSecretsCollected = 0;
			bestRunFrameCount = 99999 * 60;
			bestDeathCount = 99999;
			memset(&saveGemList[0], 0, sizeof(saveGemList));
		}
	}

	tileset = TextureCreate(tileset_png, tileset_pngBytes);
	endingText = TextureCreate(ending_png, ending_pngBytes);

	templateBlock.flags = FLAG_SOLID;
	templateBlock.texture = TextureSub(tileset, 32, 48, 16, 16);
	templateBlock.message = Block;

	templateSecretBlock.texture = TextureSub(tileset, 32, 48, 16, 16);
	templateSecretBlock.message = Block;

	templateSecret.texture = TextureSub(tileset, 32, 0, 16, 16);
	templateSecret.message = Secret;
	templateSecret.w = 10;
	templateSecret.h = 11;
	templateSecret.tag = TAG_SECRET;

	templateUpBlock.flags = FLAG_SOLID;
	templateUpBlock.texture = TextureSub(tileset, 96, 0, 16, 16);
	templateUpBlock.message = UpBlock;

	templateGem.tag = TAG_GEM;
	templateGem.texture = TextureSub(tileset, 48, 0, 16, 16);
	templateGem.message = Gem;

	templateSave.tag = TAG_SAVE;
	templateSave.texture = TextureSub(tileset, 64, 0, 16, 16);
	templateSave.message = SaveM;

	templateSaveEasy.tag = TAG_SAVE;
	templateSaveEasy.texture = TextureSub(tileset, 112, 32, 16, 16);
	templateSaveEasy.message = SaveEasyM;

	templateDeath.texture = TextureSub(tileset, 16, 0, 16, 16);
	templateDeath.message = Death;
	templateDeath.flags = FLAG_DEATH;

	templateDeathHard.texture = TextureSub(tileset, 0, 48, 16, 16);
	templateDeathHard.message = DeathHard;
	templateDeathHard.flags = FLAG_DEATH;

	templateDeathHorz.texture = TextureSub(tileset, 32, 16, 16, 16);
	templateDeathHorz.message = DeathHorz;
	templateDeathHorz.flags = FLAG_DEATH;

	templateDeathVert.texture = TextureSub(tileset, 64, 16, 16, 16);
	templateDeathVert.message = DeathVert;
	templateDeathVert.flags = FLAG_DEATH;

	templateHeli.texture = TextureSub(tileset, 96, 16, 16, 16);
	templateHeli.tag = TAG_HELI;
	templateHeli.message = Heli;

	templateBoostUp.texture = TextureSub(tileset, 0, 32, 16, 16);
	templateBoostUp.tag = TAG_BOOST_UP;
	templateBoostUp.flags = FLAG_BOOST;

	templateBoostDown.texture = TextureSub(tileset, 64, 32, 16, 16);
	templateBoostDown.tag = TAG_BOOST_DOWN;
	templateBoostDown.flags = FLAG_BOOST;

	templateBoostLeft.texture = TextureSub(tileset, 80, 32, 16, 16);
	templateBoostLeft.tag = TAG_BOOST_LEFT;
	templateBoostLeft.flags = FLAG_BOOST;

	templateBoostRight.texture = TextureSub(tileset, 96, 32, 16, 16);
	templateBoostRight.tag = TAG_BOOST_RIGHT;
	templateBoostRight.flags = FLAG_BOOST;

	templateJumpHelper.texture = TextureSub(tileset, 16, 48, 16, 16);
	templateJumpHelper.tag = TAG_BOOST_UP;
	templateJumpHelper.flags = FLAG_BOOST;
	templateJumpHelper.message = JumpHelper;

	templateDoor.texture = TextureSub(tileset, 16, 32, 16, 16);
	templateDoor.tag = TAG_DOOR;
	templateDoor.flags = FLAG_SOLID;

	templateExit.texture = TextureSub(tileset, 80, 0, 16, 16);
	templateExit.tag = TAG_EXIT;

	templateDoorInverse.texture = TextureSub(tileset, 48, 32, 16, 16);
	templateDoorInverse.tag = TAG_DOOR_INVERSE;
	
	templateSwitch.texture = TextureSub(tileset, 32, 32, 16, 16);
	templateSwitch.tag = TAG_SWITCH;

	templateDoubleJump.tag = TAG_DOUBLE_JUMP;
	templateDoubleJump.texture = TextureSub(tileset, 112, 0, 16, 16);
	templateDoubleJump.message = DoubleJump;

	player = EntityCreate(NULL, 16, 16, 0);
	player->message = Player;
	player->w = 7;
	player->h = 7;
	player->texture = TextureSub(tileset, 0, 16, 16, 16);
	player->x = saveX;
	player->y = saveY;
	player->texOffX = -5;
	player->texOffY = -8;
	player->layer = 1;

	roomX = saveRoomX;
	roomY = saveRoomY;

	loadRoom = true;

	PLAY_SOUND("audio/bgm.opus", true, 0.5);

	// Preload sound effects.
	PLAY_SOUND("audio/sfx_boost.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_death.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_gem.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_secret.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_heli2.ogg", false, 0.0);
	PLAY_SOUND("audio/sfx_jump.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_save.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_switch.wav", false, 0.0);
}

void GameUpdate() {
	globalTimer++;

	if (keysHeld['M']) {
		keysHeld['M'] = 0;
		menuOpen = !menuOpen;
		menuHighlighted = 0;
		return;
	}

	if (menuOpen) {
		if (!endOfGame) {
			runFrameCount++;
		}

		if (keysHeld[KEY_LEFT]) {
			keysHeld[KEY_LEFT] = false;
			menuHighlighted = (menuHighlighted + 2) % 3;
		}

		if (keysHeld[KEY_RIGHT]) {
			keysHeld[KEY_RIGHT] = false;
			menuHighlighted = (menuHighlighted + 1) % 3;
		}

		if (keysHeld['X']) {
			keysHeld['X'] = 0;

			if (menuHighlighted == 0) {
				menuOpen = false;
			} else if (menuHighlighted == 1) {
				gemsCollected = 0;
				secretsCollected = 0;
				playerCanDoubleJump = false;
				insideSave = true;
				saveX = 206;
				saveY = 200;
				saveRoomX = 2;
				saveRoomY = 2;
				saveGemsCollected = 0;
				saveSecretsCollected = 0;
				runFrameCount = 0;
				deathCount = 0;
				memset(&saveGemList[0], 0, sizeof(saveGemList));
				roomX = 2; 
				roomY = 2;
				spawnParticlesAtPlayer = true;
				memset(&gemList[0], 0, sizeof(gemList));
				heliTimer = 0.0f;
				backgroundMotion = 0;
				boosting = 0;
				deathTimer = 0.0f;
				endOfGame = false;
				endTimer = 0.0f;
				endTimer2 = 0;
				menuOpen = false;
				player->timer = 0;
				player->x = saveX;
				player->y = saveY;
				roomsVisited = 0;
				showingEndScreen = false;
				bossRoomDirection = 0;
				LoadRoom();
			} else if (menuHighlighted == 2) {
				hardMode = !hardMode;
			}
		}

		return;
	}

	if (endTimer2 >= 1500) {
		showingEndScreen = true;

		if (endTimer2 == 1500) {
			if (secretsCollected != 0xF) {
				WriteNewGame();
			} else {
				roomX = 0;
				roomY = 6;
				player->x = 32;
				player->y = 32;
				endTimer = 0;
				endTimer2 = 0;
				endOfGame = false;
				gemsCollected = 0;
				memset(&gemList[0], 0, sizeof(gemList));
				showingEndScreen = false;
				LoadRoom();
				PLAY_SOUND("audio/boss.opus", true, 0.5);
				return;
			}
		}

		endTimer2++;
		return;
	}

	if (loadRoom) {
		loadRoom = false;
		LoadRoom();
	}

	if (!editMode) {
		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity *entity = entities + i;

			if (entity->slot == SLOT_USED) {
				entity->x += entity->dx;
				entity->y += entity->dy;
				entity->message(entity, MSG_STEP);
				entity->tick++;
			}
		}

#ifdef UI_LINUX
		if (luigiKeys[UI_KEYCODE_LETTER('P')]) {
			editMode = true;
			endOfGame = false;
			luigiKeys[UI_KEYCODE_LETTER('P')] = false;
		} else if (luigiKeys[UI_KEYCODE_LETTER('G')]) {
			gemsCollected += 1;
			luigiKeys[UI_KEYCODE_LETTER('G')] = false;
		} else if (luigiKeys[UI_KEYCODE_LETTER('H')]) {
			gemsCollected += 4;
			luigiKeys[UI_KEYCODE_LETTER('H')] = false;
		}
#endif
	} else {
#ifdef UI_LINUX
		if (mouseLeft && mouseX >= 0 && mouseY >= 0 && mouseX < GAME_WIDTH && mouseY < GAME_HEIGHT) {
			roomData[mouseY / 16][mouseX / 16] = selectedEntityIndex;
		} else if (mouseRight && mouseX >= 0 && mouseY >= 0 && mouseX < GAME_WIDTH && mouseY < GAME_HEIGHT) {
			roomData[mouseY / 16][mouseX / 16] = 0;
		}

		if (luigiKeys[UI_KEYCODE_DIGIT('1')]) selectedEntityIndex = 1;
		if (luigiKeys[UI_KEYCODE_DIGIT('2')]) selectedEntityIndex = 2;
		if (luigiKeys[UI_KEYCODE_DIGIT('3')]) selectedEntityIndex = 3;
		if (luigiKeys[UI_KEYCODE_DIGIT('4')]) selectedEntityIndex = 4;
		if (luigiKeys[UI_KEYCODE_DIGIT('5')]) selectedEntityIndex = 5;
		if (luigiKeys[UI_KEYCODE_DIGIT('6')]) selectedEntityIndex = 6;
		if (luigiKeys[UI_KEYCODE_DIGIT('7')]) selectedEntityIndex = 7;
		if (luigiKeys[UI_KEYCODE_DIGIT('8')]) selectedEntityIndex = 8;
		if (luigiKeys[UI_KEYCODE_DIGIT('9')]) selectedEntityIndex = 9;
		if (luigiKeys[UI_KEYCODE_FKEY(1)]) selectedEntityIndex = 10;
		if (luigiKeys[UI_KEYCODE_FKEY(2)]) selectedEntityIndex = 11;
		if (luigiKeys[UI_KEYCODE_FKEY(3)]) selectedEntityIndex = 12;
		if (luigiKeys[UI_KEYCODE_FKEY(4)]) selectedEntityIndex = 13;
		if (luigiKeys[UI_KEYCODE_FKEY(5)]) selectedEntityIndex = 14;
		if (luigiKeys[UI_KEYCODE_FKEY(6)]) selectedEntityIndex = 15;
		if (luigiKeys[UI_KEYCODE_FKEY(7)]) selectedEntityIndex = 16;
		if (luigiKeys[UI_KEYCODE_FKEY(8)]) selectedEntityIndex = 17;
		if (luigiKeys[UI_KEYCODE_FKEY(9)]) selectedEntityIndex = 18;
		if (luigiKeys[UI_KEYCODE_FKEY(10)]) selectedEntityIndex = 19;
		if (luigiKeys[UI_KEYCODE_FKEY(11)]) selectedEntityIndex = 20;
		if (luigiKeys[UI_KEYCODE_FKEY(12)]) selectedEntityIndex = 21;

		if (luigiKeys[UI_KEYCODE_LETTER('S')]) {
			char buf[64];
			snprintf(buf, sizeof(buf), "embed/r%d_%d.dat", roomX, roomY);
			FILE *f = fopen(buf, "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);
		} else if (luigiKeys[UI_KEYCODE_LETTER('P')]) {
			char buf[64];
			snprintf(buf, sizeof(buf), "embed/r%d_%d.dat", roomX, roomY);
			FILE *f = fopen(buf, "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);

			editMode = false;
			loadRoom = true;
			luigiKeys[UI_KEYCODE_LETTER('P')] = false;
			saveX = player->x = mouseX - player->w / 2;
			saveY = player->y = mouseY - player->h / 2;
			saveRoomX = roomX;
			saveRoomY = roomY;
			memset(saveGemList, 0, sizeof(saveGemList));
			gemsCollected = 0;
			memset(gemList, 0, sizeof(gemList));
		}

		if (luigiKeys[UI_KEYCODE_LEFT]) {
			char buf[64];
			snprintf(buf, sizeof(buf), "embed/r%d_%d.dat", roomX, roomY);
			FILE *f = fopen(buf, "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);

			luigiKeys[UI_KEYCODE_LEFT] = false;
			roomX--;
			loadRoom = true;
		} else if (luigiKeys[UI_KEYCODE_RIGHT]) {
			char buf[64];
			snprintf(buf, sizeof(buf), "embed/r%d_%d.dat", roomX, roomY);
			FILE *f = fopen(buf, "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);

			luigiKeys[UI_KEYCODE_RIGHT] = false;
			roomX++;
			loadRoom = true;
		} else if (luigiKeys[UI_KEYCODE_UP]) {
			char buf[64];
			snprintf(buf, sizeof(buf), "embed/r%d_%d.dat", roomX, roomY);
			FILE *f = fopen(buf, "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);

			luigiKeys[UI_KEYCODE_UP] = false;
			roomY--;
			loadRoom = true;
		} else if (luigiKeys[UI_KEYCODE_DOWN]) {
			char buf[64];
			snprintf(buf, sizeof(buf), "embed/r%d_%d.dat", roomX, roomY);
			FILE *f = fopen(buf, "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);

			luigiKeys[UI_KEYCODE_DOWN] = false;
			roomY++;
			loadRoom = true;
		}
#endif
	}

	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = entities + i;

		if ((entity->slot & SLOT_USED) && (entity->slot & SLOT_DESTROYING)) {
			entity->slot &= ~SLOT_USED;
		}
	}

	backgroundMotion = (backgroundMotion + 1) % 64;

	if (bossRoomDirection) {
		if (roomX == 2 && roomY == 6) {
			bossRoomTimer += 0.5f;
		} else if ((roomX == 4 && roomY == 6) || (roomX == 6 && roomY == 5)) {
			bossRoomTimer += 0.7f;
		} else {
			bossRoomTimer += 1.0f;
		}
		
		if (bossRoomTimer > 70.0f) {
			bossRoomPosition = LinearMap(70.0f, 360.0f, 0, 1, bossRoomTimer);
		} else {
			bossRoomPosition = 0;
		}
	}
}

Texture Digit(int i) {
	i %= 10;
	return TextureSub(tileset, (i % 8) * 16, 128 - 16 - (i / 8) * 16, 16, 16);
}

void DrawTime(int frameCount, int x, int y) {
	int s = frameCount / 60;
	int m = s / 60;
	if (m > 99) m = 99;
	s %= 60;

	Blend(Digit(m / 10), x + 16 * 0, y, 0xFF);
	Blend(Digit(m /  1), x + 16 * 1, y, 0xFF);
	Blend(TextureSub(tileset, 32, 96, 16, 16), x + 16 * 2, y, 0xE0);
	Blend(Digit(s / 10), x + 16 * 3, y, 0xFF);
	Blend(Digit(s /  1), x + 16 * 4, y, 0xFF);
}

void GameRender() {
	if (showingEndScreen) {
		Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x000000);
		Blend(TextureSub(endingText, 15, 27, 81, 70), 80, 80, 0xFF);

		if (runFrameCount < bestRunFrameCount) { bestRunFrameCount = runFrameCount; }
		if (deathCount < bestDeathCount) { bestDeathCount = deathCount; }

		DrawTime(runFrameCount, 160, 79);
		DrawTime(bestRunFrameCount, 160, 96);

		Blend(Digit(deathCount / 10000), 160 + 16 * 0, 79 + 17 * 2, 0xFF);
		Blend(Digit(deathCount /  1000), 160 + 16 * 1, 79 + 17 * 2, 0xFF);
		Blend(Digit(deathCount /   100), 160 + 16 * 2, 79 + 17 * 2, 0xFF);
		Blend(Digit(deathCount /    10), 160 + 16 * 3, 79 + 17 * 2, 0xFF);
		Blend(Digit(deathCount /     1), 160 + 16 * 4, 79 + 17 * 2, 0xFF);

		Blend(Digit(bestDeathCount / 10000), 160 + 16 * 0, 79 + 17 * 3, 0xFF);
		Blend(Digit(bestDeathCount /  1000), 160 + 16 * 1, 79 + 17 * 3, 0xFF);
		Blend(Digit(bestDeathCount /   100), 160 + 16 * 2, 79 + 17 * 3, 0xFF);
		Blend(Digit(bestDeathCount /    10), 160 + 16 * 3, 79 + 17 * 3, 0xFF);
		Blend(Digit(bestDeathCount /     1), 160 + 16 * 4, 79 + 17 * 3, 0xFF);

		if (secretsCollected == 0xF) {
			Blend(TextureSub(endingText, 303, 6, 76, 14), GAME_WIDTH - 76 - 80, GAME_HEIGHT - 80, 0xFF);
		}

		if (deathCount < 9) {
			Blend(TextureSub(endingText, 2, 2, 34, 13), GAME_WIDTH - 76 - 80, GAME_HEIGHT - 80 + 20, 0xFF);
		}

		return;
	}

	Texture backgroundTile = TextureSub(tileset, 
			32 + 16 * colorOfRoom[roomY >= 0 && roomY < 10 ? roomY : 0][roomX >= 0 && roomX < 10 ? roomX : 0], 64, 16, 16);

	for (int j = -1; j <= GAME_HEIGHT / 16 + 1; j++) {
		for (int i = -1; i <= GAME_WIDTH / 16 + 1; i++) {
			Blit(backgroundTile, i * 16 + backgroundMotion / 4, j * 16 + backgroundMotion / 4);
		}
	}

	if (!editMode) {
		for (int layer = -1; layer <= 4; layer++) {
			for (int i = 0; i < MAX_ENTITIES; i++) {
				Entity *entity = entities + i;
				if ((~entity->slot & SLOT_USED) || (entity->layer != layer)) continue;

				int x = entity->x + entity->texOffX;
				int y = entity->y + entity->texOffY;

				if (entity->texture.width && entity->texture.height) {
					uint32_t frame = entity->stepsPerFrame >= 0 
						? ((entity->tick / entity->stepsPerFrame) % entity->frameCount) 
						: (-entity->stepsPerFrame - 1);
					Texture texture = entity->texture;
					texture.bits += (frame % entity->frameRow) * texture.width;
					texture.bits += (frame / entity->frameRow) * texture.height * texture.stride;
					bool noEndAlpha = (entity->message == Particle || entity->message == Player);
					Blend(texture, x, y, entity->alpha * (noEndAlpha ? 1.0f : 1.0f - endTimer));
				}

				entity->message(entity, MSG_DRAW);
			}
		}

		if (bossRoomDirection == 1) {
			Fill(0, 0, GAME_WIDTH * bossRoomPosition, GAME_HEIGHT, 0x000000);
		} else if (bossRoomDirection == 2) {
			Fill(0, GAME_HEIGHT - GAME_HEIGHT * bossRoomPosition, GAME_WIDTH, GAME_HEIGHT, 0x000000);
		} else if (bossRoomDirection == 3) {
			Fill(GAME_WIDTH - GAME_WIDTH * bossRoomPosition, 0, GAME_WIDTH, GAME_HEIGHT, 0x000000);
		}

		if (didSaveTimer > 0) {
			didSaveTimer--;
			Blend(TextureSub(tileset, 80, 96, 32, 16), player->x - 12, player->y - 20, 4 * didSaveTimer);
		}

		Blend(TextureSub(tileset, 48, 0, 16, 16), 0, 0, 0xE0 * (1.0f - endTimer));
		Blend(TextureSub(tileset, 112, 96, 16, 16), 16, 0, 0xE0 * (1.0f - endTimer));
		Blend(Digit(gemsCollected / 10), 32, 0, 0xE0 * (1.0f - endTimer));
		Blend(Digit(gemsCollected % 10), 48, 0, 0xE0 * (1.0f - endTimer));

		int secretsCollectedCount = 0;

		for (int i = 0; i < 8; i++) {
			if (secretsCollected & (1 << i)) {
				secretsCollectedCount++;
			}
		}

		if (secretsCollectedCount == 4 && endTimer2 > 0) {
			float k = endTimer2 / 300.0f;
			if (k > 1) { k = 1; }
			float tr = globalTimer / 311.0f;
			float r = 30.0f + 10.0f * CheapSineApproximation01(tr - __builtin_floorf(tr));

			for (int i = 0; i < secretsCollectedCount; i++) {
				float ts = globalTimer / 140.0f + i * 0.25f;
				float tc = ts + 0.25f;
				Blend(TextureSub(tileset, 0, 0, 16, 16), 
						(64 + i * 16) * (1.0f - k) 
						+ k * (player->x + player->texOffX + r * CheapSineApproximation01(ts - __builtin_floorf(ts))), 
						k * (player->y + player->texOffY + r * CheapSineApproximation01(tc - __builtin_floorf(tc))), 0xE0);
			}
		} else {
			for (int i = 0; i < secretsCollectedCount; i++) {
				float t = globalTimer / 70.0f + i / 0.3f;
				Blend(TextureSub(tileset, 0, 0, 16, 16), 64 + i * 16, 
						secretsCollectedCount == 4 ? 2 * CheapSineApproximation01(t - __builtin_floorf(t)) : 0, 0xE0);
			}
		}

		if (heliTimer) {
			Fill(GAME_WIDTH - 110, 10, 100, 10, 0xFF0000);
			Fill(GAME_WIDTH - 110, 10, 100 * heliTimer, 10, 0x00FF00);
		}

#define DO_ENDING_TEXT(startTime, endTime, x, y, width, height) \
		Blend(TextureSub(endingText, x, y, width, height), x, y, FadeInOut(LinearMap(startTime, endTime, 0, 1, endTimer2)) * 255.0f);
		DO_ENDING_TEXT(120, 300, 161, 4, 91, 20);
		DO_ENDING_TEXT(260, 440, 191, 24, 195, 35);
		DO_ENDING_TEXT(400, 580, 182, 61, 120, 18);
		DO_ENDING_TEXT(460, 640, 194, 80, 199, 36);
		DO_ENDING_TEXT(600, 780, 182, 116, 207, 50);
		DO_ENDING_TEXT(740, 920, 163, 171, 235, 34);
		DO_ENDING_TEXT(880, 1060, 166, 211, 226, 34);

		if (secretsCollectedCount < 4) {
			DO_ENDING_TEXT(1060, 1120, 113, 139, 54, 21);
			DO_ENDING_TEXT(1120, 1300, 194, 245, 176, 19);
			DO_ENDING_TEXT(1260, 1440, 222, 264, 118, 20);
		} else {
			Blend(TextureSub(endingText, 12, 196, 141, 36), 194 + GetRandomByte() % 2, 120 + GetRandomByte() % 2, FadeInOut(LinearMap(1120, 1300, 0, 1, endTimer2)) * 255.0f);
			Blend(TextureSub(endingText, 29, 243, 106, 20), 222 + GetRandomByte() % 2, 199 + GetRandomByte() % 2, FadeInOut(LinearMap(1260, 1440, 0, 1, endTimer2)) * 255.0f);
		}

		if (endTimer2 > 1440 && endTimer2 < 1500) {
			FillBlend(0, 0, GAME_WIDTH, GAME_HEIGHT, (uint32_t) (LinearMap(1440, 1500, 0, 1, endTimer2) * 255.0f) << 24);
		}
	} else {
#ifdef UI_LINUX
		for (int j = 0; j < GAME_HEIGHT / 16; j++) {
			for (int i = 0; i < GAME_WIDTH / 16; i++) {
				if (roomData[j][i]) {
					if (templateList[roomData[j][i]] == &templateJumpHelper) {
						Fill(i * 16 - 100, j * 16, 200 + 16, 2, 0xFF00FF);
						Blend(templateList[roomData[j][i]]->texture, i * 16, j * 16, 0xFF);
					} else if (templateList[roomData[j][i]] == &templateSecretBlock) {
						Blend(templateList[roomData[j][i]]->texture, i * 16, j * 16, 0x7F);
					} else {
						Blend(templateList[roomData[j][i]]->texture, i * 16, j * 16, 0xFF);

					}
				}
			}
		}

		if (!mouseRight) {
			Blend(templateList[selectedEntityIndex]->texture, mouseX & ~15, mouseY & ~15, 0x80);
		}

		Blend(Digit(roomX), 0, 0, 0xE0);
		Blend(TextureSub(tileset, 112, 96, 16, 16), 16, 0, 0xE0);
		Blend(Digit(roomY), 32, 0, 0xE0);
#endif
	}

	if (menuOpen) {
		FillBlend(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x80000000);
		DrawTime(runFrameCount, 160, 79);
		int x[] = { 40, 110, 200 };
		Blend(TextureSub(tileset, 26, 83, 36, 9), x[0], 150, 0xFF);
		Blend(TextureSub(tileset, 68, 83, 58, 9), x[1], 150, 0xFF);
		Blend(TextureSub(tileset, 52, 99, 25, 9), x[2], 150, 0xFF);
		Fill(x[menuHighlighted], 160, 50, 3, 0xFFFF00);
		Blend(TextureSub(tileset, hardMode ? 0 : 16, 64, 16, 16), x[2] + 34, 147, 0xFF);

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (roomsVisited & ((uint64_t) 1 << (i * 8 + j))) {
					FillBlend(GAME_WIDTH - 8 * 4 - 20 + j * 4, 20 + i * 4, 3, 3, roomX == j && roomY == i ? 0xFFFFFFFF : 0x80FFFFFF);
				}
			}
		}
	}
}
