#define SLOT_USED       (1 << 0)
#define SLOT_DESTROYING (1 << 1)

#define MSG_STEP   (1)
#define MSG_DRAW   (2)
#define MSG_CREATE (3)

#define TAG_SAVEPOINT (1)
#define TAG_FLIP (2)
#define TAG_PLAYER (3)

#define FLAG_SOLID (1 << 0)
#define FLAG_DEATH (1 << 1)

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

typedef struct SaveData {
	int playerX, playerY;
	int roomX, roomY;
	uint32_t hash;
	bool gravityFlipped;
	int bucket; // 0 = not got, 1 = empty, 2 = milk
	int sack; // 0 = not got, 1 = empty, 2 = flour
	int egg; // 0 = not got, 1 = got
	int grain; // 0 = not got, 1 = got
	int cake; // 0 = not got, 1 = got
} SaveData;

typedef struct GameData {
	bool loadRoom;
	int globalTimer;
	int onFloor;
	int lastDeathTimer;
	int justRespawnedTimer;
	int area;
	bool xHeldPrevious;
	char message[180];
	int messageTimer;
	bool justSaved;
} GameData;

#define MAX_ENTITIES (300)
Entity entities[MAX_ENTITIES];
int maxUsedEntity;
Texture tileset;
Entity *player;
GameData gd;
SaveData sd;
SaveData lastCheckPoint;
char saveBuffer[2048];

Entity templateBlock, templateDeath, templateSavePoint, templateFlip,
       templateArea1, templateArea2, templateUpBlock,
       templateBucket, templateCow, templateChicken, templateSack, templateGrain, templateMill, templateOven;
#define TILES_X (GAME_WIDTH / 16)
#define TILES_Y (GAME_HEIGHT / 16)
int roomData[20 * TILES_Y][20 * TILES_X];

#ifdef UI_LINUX
bool editMode = true;
#else
bool editMode = false;
#endif
bool fullMapMode;
int selectedEntityIndex = 1;

Entity *templateList[] = {
	NULL,
	&templateBlock,
	&templateDeath,
	&templateSavePoint,
	&templateArea1,
	&templateArea2,
	&templateUpBlock,
	&templateBucket, &templateCow, &templateChicken, &templateSack, &templateGrain, &templateMill,
	&templateFlip,
	&templateOven,
};

uint64_t FNV1a(const void *key, size_t keyBytes) {
	uint64_t hash = 0xCBF29CE484222325;

	for (uintptr_t i = 0; i < keyBytes; i++) {
		hash = (hash ^ ((uint8_t *) key)[i]) * 0x100000001B3;
	}

	return hash;
}

void WriteSaveData(int x, int y) {
	sd.playerX = x;
	sd.playerY = y;
	lastCheckPoint = sd;
	sd.hash = 0;
	sd.hash = FNV1a(&sd, sizeof(sd));

	if (sizeof(sd) > sizeof(saveBuffer) / 2) Panic();
	for (uintptr_t i = 0; i < sizeof(sd); i++) {
		saveBuffer[i * 2 + 0] = 'A' + (((const uint8_t *) &sd)[i] & 15);
		saveBuffer[i * 2 + 1] = 'A' + (((const uint8_t *) &sd)[i] >> 4);
	}
//	SaveGameToCookie(saveBuffer, sizeof(sd) * 2);
}

void RestoreSaveData() {
	sd = lastCheckPoint;
	player->x = sd.playerX;
	player->y = sd.playerY;
	player->dx = player->dy = 0;
	gd.loadRoom = true;
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
		if (maxUsedEntity <= i) maxUsedEntity = i + 1;
		return entity;
	}
	
	static Entity fake = {};
#ifdef UI_LINUX
	printf("too many entities!\n");
#endif
	return &fake;
}

void EntityDestroy(Entity *entity) {
	entity->slot |= SLOT_DESTROYING;
}

Entity *EntityFind(float x, float y, float w, float h, uint8_t tag, uint8_t flags, Entity *exclude) {
	for (int i = 0; i < maxUsedEntity; i++) {
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

Entity *EntityFindAt(Entity *at, float dx, float dy, uint8_t tag, uint8_t flags, Entity *exclude) {
	return EntityFind(at->x + dx, at->y + dy, at->w, at->h, tag, flags, exclude);
}

void Death(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		entity->x += 3;
		entity->y += 3;
	}
}

void SavePoint(Entity *entity, int message) {
	if (message == MSG_STEP) {
		bool on = lastCheckPoint.playerX == entity->x && lastCheckPoint.playerY == entity->y 
			&& sd.roomX == lastCheckPoint.roomX && sd.roomY == lastCheckPoint.roomY;
		entity->texture = TextureSub(tileset, on ? 64 : 80, 0, 16, 16);
	}
}

void Area1(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		if (gd.area != 1) { PLAY_SOUND("audio/area1.opus", true, 0.5); }
		gd.area = 1;
		entity->alpha = 0;
	} else if (message == MSG_STEP && !sd.cake) {
		for (int y = 0; y < TILES_Y; y++) {
			for (int x = 0; x < TILES_X; x++) {
				Blit(TextureSub(tileset, 16, 0, 16, 16), x * 16, y * 16);
			}
		}
	}
}

void Area2(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		if (gd.area != 2) { PLAY_SOUND("audio/area2.opus", true, 0.5); }
		gd.area = 2;
		entity->alpha = 0;
	} else if (message == MSG_STEP && !sd.cake) {
		for (int y = 0; y < TILES_Y; y++) {
			for (int x = 0; x < TILES_X; x++) {
				Blit(TextureSub(tileset, 16, 16, 16, 16), x * 16, y * 16);
			}
		}
	}
}

void Block(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (sd.cake == 1) {
			entity->texture = TextureSub(tileset, 128, 96, 16, 16);
		} else {
			entity->texture = TextureSub(tileset, 0, 16 * gd.area - 16, 16, 16);
		}
	}
}

void UpBlock(Entity *entity, int message) {
	if (message == MSG_STEP) {
		entity->flags &= ~FLAG_SOLID;

		if (entity->y > player->y && player->dy > 0) {
			entity->flags |= FLAG_SOLID;
		}
	}
}

void CollectBucket(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (sd.bucket) {
			EntityDestroy(entity);
		}

		if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
			sd.bucket = 1;
			gd.messageTimer = 300;
			strcpy(gd.message, "you~got~the~bucket~~~it~can~store~milk");
			PLAY_SOUND("audio/itm.wav", false, 0.6);
		}
	}
}

void CollectSack(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (sd.sack) {
			EntityDestroy(entity);
		}

		if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
			sd.sack = 1;
			gd.messageTimer = 300;
			strcpy(gd.message, "you~got~the~sack~~~it~can~store~flour");
			PLAY_SOUND("audio/itm.wav", false, 0.6);
		}
	}
}

void CollectGrain(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (sd.grain) {
			EntityDestroy(entity);
		}

		if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
			sd.grain = 1;
			gd.messageTimer = 300;
			strcpy(gd.message, "you~got~the~grain~~~take~it~to~the~windmill~with~a~sack~to~get~flour");
			PLAY_SOUND("audio/itm.wav", false, 0.6);
		}
	}
}

void CollectFlour(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
			if (sd.grain == 0) {
				gd.messageTimer = 300;
				strcpy(gd.message, "i~must~find~the~grain~first~to~turn~into~flour");
			} else if (sd.sack == 0) {
				gd.messageTimer = 300;
				strcpy(gd.message, "maybe~i~should~get~a~sack~to~put~the~flour~in");
			} else if (sd.sack == 1 && sd.grain == 1) {
				sd.sack = 2;
				gd.messageTimer = 300;
				strcpy(gd.message, "you~put~the~grain~in~the~windmill~and~fill~up~the~sack~with~flour");
				PLAY_SOUND("audio/itm.wav", false, 0.6);
			}
		}
	}
}

void CollectCake(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
			if (sd.cake == 1) {
				// Already done.
			} else if (sd.egg != 1 || sd.bucket != 2 || sd.sack != 2) {
				gd.messageTimer = 300;
				strcpy(gd.message, "to~bake~a~cake~i~need~an~egg~and~some~flour~and~some~milk");
			} else {
				sd.cake = 1;
				gd.messageTimer = 300;
				strcpy(gd.message, "you~bake~a~nice~cake~~~congratulation");
				PLAY_SOUND("audio/itm.wav", false, 0.6);
			}
		}
	}
}

void CollectEgg(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0) && !sd.egg) {
			sd.egg = 1;
			gd.messageTimer = 300;
			strcpy(gd.message, "you~find~a~very~small~ostrich~egg~under~the~chickecn");
			PLAY_SOUND("audio/itm.wav", false, 0.6);
		}
	}
}

void CollectMilk(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0) && sd.bucket == 0) {
			gd.messageTimer = 300;
			strcpy(gd.message, "maybe~i~should~get~a~bucket~to~put~the~cows~milk~in");
		} else if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0) && sd.bucket == 1) {
			sd.bucket = 2;
			gd.messageTimer = 300;
			strcpy(gd.message, "you~fill~up~the~bucket~with~milk");
			PLAY_SOUND("audio/itm.wav", false, 0.6);
		}
	}
}

float /* -1..1 */ CheapSineApproximation01(float x /* 0..1 */) {
	return ((((-56.8889f * x + 142.2222f) * x - 103.1111f) * x + 12.4444f) * x + 5.3333f) * x;
}

void Player(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (gd.lastDeathTimer) {
			gd.lastDeathTimer--;
			player->texture = TextureSub(tileset, 96, 0, 16, 16);

			if (!gd.lastDeathTimer) {
				sd = lastCheckPoint;
				player->x = sd.playerX;
				player->y = sd.playerY;
				gd.loadRoom = true;
				gd.justRespawnedTimer = 30;
			}

			return;
		}

		if (gd.justRespawnedTimer) {
			gd.justRespawnedTimer--;
			player->alpha = (gd.justRespawnedTimer & 4) ? 64 : 255;
		} else {
			player->alpha = 255;
		}

		player->texture = TextureSub(tileset, 32, 0, 16, 16);

		float flip = sd.gravityFlipped ? -1.0f : 1.0f;

		Entity *other = NULL;

		entity->dy += 0.2f * flip;
		if (entity->dy * flip > 5) entity->dy = 5 * flip;

		other = EntityFindAt(entity, 0, entity->dy, 0, FLAG_SOLID, 0);

		if (!other) {
			entity->y += entity->dy;
		} else if (other->y > entity->y) {
			entity->y = other->y - entity->h;
			if (flip) { entity->dy *= 0.7f; } else { entity->dy = 0; }
		} else {
			entity->y = other->y + other->h;
			if (flip) { entity->dy = 0; } else { entity->dy *= 0.7f; }
		}

		if (EntityFindAt(entity, 0, 1 * flip, 0, FLAG_SOLID, 0)) {
			gd.onFloor = 0;
		} else {
			gd.onFloor++;
		}

		if (gd.onFloor < 10 && keysHeld['X'] && !gd.xHeldPrevious) {
			gd.onFloor = 10;
			entity->dy = -5.5f * flip;
			PLAY_SOUND("audio/jmp.wav", false, 0.2);
		}

		gd.xHeldPrevious = keysHeld['X'];

		if (entity->dy * flip < 0 * flip && !keysHeld['X']) {
			entity->dy *= 0.756f;
		}

		if (keysHeld[KEY_LEFT]) {
			entity->dx -= 1.0f;
			if (entity->dx < -4.0f) entity->dx = -4.0f;
		} else if (keysHeld[KEY_RIGHT]) {
			entity->dx += 1.0f;
			if (entity->dx > 4.0f) entity->dx = 4.0f;
		} else {
			entity->dx *= 0.5f;
		}

		if (entity->dx < 0.1f && entity->dx > -0.1f) {
			entity->dx = 0.0f;
		}

		other = EntityFindAt(entity, entity->dx, 0, 0, FLAG_SOLID, 0);

		if (!other) {
			entity->x += entity->dx;
		} else if (other->x > entity->x) {
			entity->x = other->x - entity->w;
			entity->dx = 0;
		} else {
			entity->x = other->x + other->w;
			entity->dx = 0;
		}

		other = EntityFindAt(entity, 0, 0, TAG_SAVEPOINT, 0, 0);
		
		if (other) {
			if (!gd.justSaved) {
				WriteSaveData(other->x, other->y);
				PLAY_SOUND("audio/sav.wav", false, 0.6);
			}

			gd.justSaved = true;
		} else {
			gd.justSaved = false;
		}

		other = EntityFindAt(entity, 0, 0, 0, FLAG_DEATH, 0);
		
		if (other) {
			gd.lastDeathTimer = 30;
			gd.messageTimer = 0;
			PLAY_SOUND("audio/die.wav", false, 0.6);
			return;
		}

		if (player->x < -7         ) { player->x += GAME_WIDTH ; sd.roomX--; gd.loadRoom = true; return; }
		if (player->y < -7         ) { player->y += GAME_HEIGHT; sd.roomY--; gd.loadRoom = true; return; }
		if (player->x > GAME_WIDTH ) { player->x -= GAME_WIDTH ; sd.roomX++; gd.loadRoom = true; return; }
		if (player->y > GAME_HEIGHT) { player->y -= GAME_HEIGHT; sd.roomY++; gd.loadRoom = true; return; }

		other = EntityFindAt(entity, 0, 0, TAG_FLIP, 0, 0);
		
		if (other) {
			sd.gravityFlipped = !sd.gravityFlipped;
			EntityDestroy(other);
			player->dy = -player->dy;
			PLAY_SOUND("audio/flp.wav", false, 0.6);
		}
	} else if (message == MSG_DRAW) {
	}
}

void InitialiseSaveData() {
	memset(&sd, 0, sizeof(sd));
	player->x = 100;
	player->y = 100;
	sd.roomX = 10;
	sd.roomY = 10;
}

void GameInitialise(const char *saveData) {
	for (uintptr_t i = 0; i < sizeof(sd); i++) {
		((uint8_t *) &sd)[i] = (saveData[i * 2 + 0] - 'A') | ((saveData[i * 2 + 1] - 'A') << 4);
	}
	uint32_t oldHash = sd.hash;
	sd.hash = 0;
	sd.hash = FNV1a(&sd, sizeof(sd));
	bool saveValid = sd.hash == oldHash;
	if (!saveValid) memset(&sd, 0, sizeof(sd));

#ifdef UI_LINUX
	FILE *f = fopen("embed/world.dat", "rb");
	fread(&roomData[0][0], 1, sizeof(roomData), f);
	fclose(f);
#else
	memcpy(roomData, world_dat, sizeof(roomData));
#endif

	tileset = TextureCreate(tileset_png, sizeof(tileset_png));

	templateBlock.flags = FLAG_SOLID;
	templateBlock.texture = TextureSub(tileset, 0, 0, 16, 16);
	templateBlock.message = Block;

	templateUpBlock.flags = FLAG_SOLID;
	templateUpBlock.texture = TextureSub(tileset, 112, 0, 16, 16);
	templateUpBlock.message = UpBlock;
	templateUpBlock.h = 2;

	templateDeath.flags = FLAG_DEATH;
	templateDeath.texture = TextureSub(tileset, 48, 0, 16, 16);
	templateDeath.message = Death;
	templateDeath.w = 10;
	templateDeath.h = 10;
	templateDeath.texOffX = -3;
	templateDeath.texOffY = -3;

	templateBucket.texture = TextureSub(tileset, 32, 96, 16, 16); 
	templateBucket.message = CollectBucket;
	templateCow.texture = TextureSub(tileset, 32, 80, 32, 16);
	templateCow.message = CollectMilk;
	templateChicken.texture = TextureSub(tileset, 64, 80, 16, 16);
	templateChicken.message = CollectEgg;
	templateSack.texture = TextureSub(tileset, 80, 96, 16, 16);
	templateSack.message = CollectSack;
	templateGrain.texture = TextureSub(tileset, 112, 96, 16, 16);
	templateGrain.message = CollectGrain;
	templateMill.texture = TextureSub(tileset, 112, 48, 16 * 2, 16 * 3);
	templateMill.message = CollectFlour;
	templateOven.texture = TextureSub(tileset, 80, 80, 16, 16);
	templateOven.message = CollectCake;

	templateFlip.texture = TextureSub(tileset, 32, 16, 16, 16);
	templateFlip.tag = TAG_FLIP;

	templateSavePoint.texture = TextureSub(tileset, 80, 0, 16, 16);
	templateSavePoint.message = SavePoint;
	templateSavePoint.tag = TAG_SAVEPOINT;

	templateArea1.texture = TextureSub(tileset, 128, 0, 16, 16);
	templateArea1.layer = -1;
	templateArea1.message = Area1;

	templateArea2.texture = TextureSub(tileset, 128, 16, 16, 16);
	templateArea2.layer = -1;
	templateArea2.message = Area2;

	player = EntityCreate(NULL, 16, 16, 0);
	player->message = Player;
	player->w = 12;
	player->h = 12;
	player->texture = TextureSub(tileset, 32, 0, 16, 16);
	player->texOffX = -2;
	player->texOffY = -4;
	player->layer = 2;
	player->tag = TAG_PLAYER;

	gd.loadRoom = true;

	if (!saveValid) {
		InitialiseSaveData();
	} else {
		player->x = sd.playerX;
		player->y = sd.playerY;
	}

	lastCheckPoint = sd;

	// Preload sound effects.
	PLAY_SOUND("audio/jmp.wav", false, 0.0);
	PLAY_SOUND("audio/flp.wav", false, 0.0);
	PLAY_SOUND("audio/itm.wav", false, 0.0);
	PLAY_SOUND("audio/sav.wav", false, 0.0);
	PLAY_SOUND("audio/die.wav", false, 0.0);
}

void GameUpdate() {
	gd.globalTimer++;

	if (gd.loadRoom) {
		if (sd.roomX < 1 || sd.roomY < 1 || sd.roomX > 18 || sd.roomY > 18) {
			RestoreSaveData();
		}

		gd.loadRoom = false;

		for (int i = 0; i < maxUsedEntity; i++) {
			if (&entities[i] != player) {
				entities[i].slot = 0;
			}
		}

		if (player != &entities[0]) Panic();
		maxUsedEntity = 1;

		for (int j = -1; j < TILES_Y + 1; j++) {
			for (int i = -1; i < TILES_X + 1; i++) {
				if (roomData[sd.roomY * TILES_Y + j][sd.roomX * TILES_X + i]) {
					EntityCreate(templateList[roomData[sd.roomY * TILES_Y + j][sd.roomX * TILES_X + i]], 
							i * 16, j * 16, (sd.roomX << 24) | (sd.roomY << 16) | (i << 8) | (j << 0));
				}
			}
		}
	}

	if (!editMode) {
		for (int i = 0; i < maxUsedEntity; i++) {
			Entity *entity = entities + i;

			if (entity->slot == SLOT_USED) {
				entity->message(entity, MSG_STEP);
				entity->tick++;
			}
		}

#ifdef UI_LINUX
		if (luigiKeys[UI_KEYCODE_LETTER('P')]) {
			editMode = true;
			luigiKeys[UI_KEYCODE_LETTER('P')] = false;
		}
#endif
	} else {
#ifdef UI_LINUX
		if (mouseLeft && mouseX >= 0 && mouseY >= 0 && mouseX < GAME_WIDTH && mouseY < GAME_HEIGHT) {
			roomData[sd.roomY * TILES_Y + mouseY / 16][sd.roomX * TILES_X + mouseX / 16] = selectedEntityIndex;
		} else if (mouseRight && mouseX >= 0 && mouseY >= 0 && mouseX < GAME_WIDTH && mouseY < GAME_HEIGHT) {
			roomData[sd.roomY * TILES_Y + mouseY / 16][sd.roomX * TILES_X + mouseX / 16] = 0;
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
		if (luigiKeys[UI_KEYCODE_LETTER('Q')]) selectedEntityIndex = 22;
		if (luigiKeys[UI_KEYCODE_LETTER('W')]) selectedEntityIndex = 23;
		if (luigiKeys[UI_KEYCODE_LETTER('E')]) selectedEntityIndex = 24;
		if (luigiKeys[UI_KEYCODE_LETTER('R')]) selectedEntityIndex = 25;
		if (luigiKeys[UI_KEYCODE_LETTER('T')]) selectedEntityIndex = 26;
		if (luigiKeys[UI_KEYCODE_LETTER('Y')]) selectedEntityIndex = 27;
		if (luigiKeys[UI_KEYCODE_LETTER('U')]) selectedEntityIndex = 28;
		if (selectedEntityIndex >= sizeof(templateList) / sizeof(templateList[0])) selectedEntityIndex = 1;

		if (luigiKeys[UI_KEYCODE_LETTER('S')]) {
			FILE *f = fopen("embed/world.dat", "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);
		} else if (luigiKeys[UI_KEYCODE_LETTER('F')]) {
			fullMapMode = !fullMapMode;
			luigiKeys[UI_KEYCODE_LETTER('F')] = false;
		} else if (luigiKeys[UI_KEYCODE_LETTER('P')]) {
			FILE *f = fopen("embed/world.dat", "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);

			editMode = false;
			player->x = mouseX - player->w / 2;
			player->y = mouseY - player->h / 2;
			gd.loadRoom = true;
			WriteSaveData(player->x, player->y);
			luigiKeys[UI_KEYCODE_LETTER('P')] = false;
		}

		if (luigiKeys[UI_KEYCODE_LEFT]) {
			FILE *f = fopen("embed/world.dat", "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);

			luigiKeys[UI_KEYCODE_LEFT] = false;
			sd.roomX--;
			gd.loadRoom = true;
		} else if (luigiKeys[UI_KEYCODE_RIGHT]) {
			FILE *f = fopen("embed/world.dat", "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);

			luigiKeys[UI_KEYCODE_RIGHT] = false;
			sd.roomX++;
			gd.loadRoom = true;
		} else if (luigiKeys[UI_KEYCODE_UP]) {
			FILE *f = fopen("embed/world.dat", "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);

			luigiKeys[UI_KEYCODE_UP] = false;
			sd.roomY--;
			gd.loadRoom = true;
		} else if (luigiKeys[UI_KEYCODE_DOWN]) {
			FILE *f = fopen("embed/world.dat", "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);

			luigiKeys[UI_KEYCODE_DOWN] = false;
			sd.roomY++;
			gd.loadRoom = true;
		}

		// To prevent out of bounds when loading surrounding edges...
		if (sd.roomX < 1) sd.roomX = 1;
		if (sd.roomY < 1) sd.roomY = 1;
		if (sd.roomX > 18) sd.roomX = 18;
		if (sd.roomY > 18) sd.roomY = 18;
#endif
	}

	for (int i = 0; i < maxUsedEntity; i++) {
		Entity *entity = entities + i;

		if ((entity->slot & SLOT_USED) && (entity->slot & SLOT_DESTROYING)) {
			entity->slot &= ~SLOT_USED;
		}
	}

	if (gd.messageTimer) {
		gd.messageTimer--;
	}
}

void GameRender() {
	if (!editMode) {
		for (int layer = -1; layer <= 4; layer++) {
			float alpha = 1.0f;

			for (int i = 0; i < maxUsedEntity; i++) {
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
					Blend(texture, x, y, entity->alpha * alpha);
				}

				entity->message(entity, MSG_DRAW);
			}
		}

		if (gd.messageTimer) {
			int x = 10;
			int y = 0;

			for (int i = 0; gd.message[i]; i++) {
				Fill(x, GAME_HEIGHT / 2 - 8 - 2 + y, 9, 8 + 4, 0xFFFFFFFF);
				Blend(TextureSub(tileset, (gd.message[i] - 'a') % 18 * 8 + 0, (gd.message[i] - 'a') / 18 * 8 + 112, 8, 8), x, GAME_HEIGHT / 2 - 8 + y, 255);
				if (GAME_WIDTH - x - 30 < 0) { x = 10; y += 10; }
				else { x += 9; }
			}
		}

		{
			FillBlend(1, 1, 16 * 4, 16, 0x7FFFFFFF);
			if (sd.bucket == 1) { Blend(TextureSub(tileset, 32, 96, 16, 16), 1 + 16 * 0, 1, 255); }
			if (sd.bucket == 2) { Blend(TextureSub(tileset, 48, 96, 16, 16), 1 + 16 * 0, 1, 255); }
			if (sd.sack == 1) { Blend(TextureSub(tileset, 80, 96, 16, 16), 1 + 16 * 1, 1, 255); }
			if (sd.sack == 2) { Blend(TextureSub(tileset, 96, 96, 16, 16), 1 + 16 * 1, 1, 255); }
			if (sd.egg == 1) { Blend(TextureSub(tileset, 64, 96, 16, 16), 1 + 16 * 2, 1, 255); }
			if (sd.grain == 1 && sd.sack != 2) { Blend(TextureSub(tileset, 112, 96, 16, 16), 1 + 16 * 3, 1, 255); }
		}
#ifdef UI_LINUX
	} else if (fullMapMode) {
		Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x808080);

		for (int j = 0; j < TILES_Y * 20; j++) {
			for (int i = 0; i < TILES_X * 20; i++) {
				if ((i % TILES_X) == 0 || (j % TILES_Y) == 0) {
					FillBlend(i, j, 1, 1, 0x20000000);
				}

				if (i / TILES_X == sd.roomX && j / TILES_Y == sd.roomY) {
					FillBlend(i, j, 1, 1, 0x20000000);
				}

				if (roomData[j][i]) {
					Blend(TextureSub(templateList[roomData[j][i]]->texture, 4, 8, 1, 1), i * 1, j * 1, 0xFF);
				}
			}
		}
	} else {
		Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x808080);

		for (int j = 0; j < TILES_Y; j++) {
			for (int i = 0; i < TILES_X; i++) {
				if (roomData[sd.roomY * TILES_Y + j][sd.roomX * TILES_X + i]) {
					Blend(templateList[roomData[sd.roomY * TILES_Y + j][sd.roomX * TILES_X + i]]->texture, i * 16, j * 16, 0xFF);
				}
			}
		}

		if (!mouseRight) {
			Blend(templateList[selectedEntityIndex]->texture, mouseX & ~15, mouseY & ~15, 0x80);
		}
#endif
	}
}
