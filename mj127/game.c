#define SLOT_USED       (1 << 0)
#define SLOT_DESTROYING (1 << 1)

#define MSG_STEP   (1)
#define MSG_DRAW   (2)
#define MSG_CREATE (3)

#define FLAG_SOLID (1 << 0)
#define FLAG_HIDE  (1 << 1)
#define FLAG_DEATH (1 << 2)
#define FLAG_GUY_SOLID (1 << 3)

#define TAG_SAVE (1)
#define TAG_GEM (2)
#define TAG_TELEPORT (3)
#define TAG_MINIGEM (4)
#define TAG_DOUBLE_JUMP (5)
#define TAG_SWITCH (6)
#define TAG_DOOR_ON (7)
#define TAG_DOOR_OFF (8)
#define TAG_BOOST_UP (9)
#define TAG_LOCKED_DOOR (10)
#define TAG_WINGS (11)
#define TAG_GUY (12)
#define TAG_ROCKET (13)
#define TAG_EXCLAIM_BLOCK (14)
#define TAG_HELI (15)
#define TAG_PUZZLE_A (16)
#define TAG_EXIT (17)
#define TAG_PUZZLE_B (18)

#define UPGRADE_JUMP_HEIGHT (1 << 0)
#define UPGRADE_TELEPORT (1 << 1)
#define UPGRADE_TOOTH_HALF_OF_KEY (1 << 2)
#define UPGRADE_LOOP_HALF_OF_KEY (1 << 3)
#define UPGRADE_BIG_OL_SWITCH (1 << 4)

#ifndef REPAINT_WORLD_OVERVIEW_CANVAS
#define REPAINT_WORLD_OVERVIEW_CANVAS()
#endif

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
	int hasUpgrade, hasKey, hasTeleporter;
	int deathCount;
	uint32_t gemList[80];
	uint32_t mapSeen[20];
	uint32_t hash;
} SaveData;

typedef struct GameData {
	bool loadRoom;
	bool lastDirection;
	int onGround;
	int jumpBuffer;
	bool holdingJump;
	int insideSave;
	int insideTeleport;
	int wallJumped;
	int justSaved;
	int unlockTimer;
	int justUnlocked;
	bool canDoubleJump;
	int boosting;
	float wingsTimer;
	float rocketTimer;
	int horizontallyBouncing;
	int globalTimer;
	float deathTimer;
	bool spawnParticlesAtPlayer;
	float rocketSpeed;
	int rocketDirection;
	float heliTimer;
	uint32_t puzzleSolution;
	int puzzleCount;
	float teleportTimer;
	int teleportToX;
	int teleportToY;
	int teleportToRX;
	int teleportToRY;
	int afterTeleport;
	bool endOfGame;
	float endTimer;
	int endTimer2;
	bool menuOpen;
	int menuIndex;
	bool menuConfirm;
} GameData;

#define MAX_ENTITIES (300)
Entity entities[MAX_ENTITIES];
int maxUsedEntity;
Texture tileset;
Entity *player;
GameData gd;
SaveData sd;
SaveData lastCheckPoint, lastTeleport;

Entity templateBlock, templateSave, templateDeath, templateGem, templateTeleport, templateMiniGem, templateDoubleJump, templateUpBlock,
       templateDoorOn, templateDoorOff, templateSwitch, templateLoopHalfOfKey, templateToothHalfOfKey, templateBoostUp, templateLockedDoor,
       templateWings, templateGuyHorizontal, templateGuyWall, templateGuyCircle, templateFallingBlock, templateVerticalDeath,
       templateRocket, templateExclaimBlock, templateHeli, templateBigOlSwitch, templatePuzzleA, templatePuzzleB, templateExit;
#define TILES_X (GAME_WIDTH / 16)
#define TILES_Y (GAME_HEIGHT / 16)
int roomData[20 * TILES_Y][20 * TILES_X];

uint8_t recolor[144 * 128];
int paletteIndex;
bool disablePaletteSwaps;
uint32_t palettes[][7] = {
	{ 0, 0x4a234d, 0x5d71d4, 0x86b4f3, 0xd6dbe4, 0xeaebef, 0xffffff },
	{ 0, 0x0a0012, 0x013de1, 0xc0d8ff, 0x0c0550, 0xd0e8ff, 0xffffff },
	{ 0, 0x4a234d, 0xd9ab77, 0xf5d2b1, 0x6e55bf, 0x7f83e5, 0xffffff },
	{ 0, 0x550d1e, 0xaa406e, 0xfaaed1, 0xfecdf7, 0xfecdf7, 0xffffff },
	{ 0, 0x3e000c, 0x780cc8, 0x33b8f8, 0x673126, 0x9d7066, 0xe8f1ff },
	{ 0, 0x13000a, 0x473c4b, 0x5c676c, 0x56aa8e, 0x7dcdab, 0xe4e5ea },
};
uint8_t paletteForRoom[20][20] = {};
uint32_t uidAssign[5] = {
	0xa0a100c, // teleport upgrade
	0xb0a110d, // loop half of key
	0x809100d, // tooth half of key
	0xb09040c, // jump height
	0xa070c10, // big ol' switch
};

#ifdef LEVEL_EDITOR
bool editMode = true;
#else
bool editMode = false;
#endif
bool renderMapMode;
int selectedEntityIndex = 1;
bool clickToChooseStartPosition;
int clickToChooseUIDAssignment;
bool showCollisionShapes;

typedef struct { 
	uint32_t version;  
	int roomData[20 * TILES_Y][20 * TILES_X];
	uint8_t paletteForRoom[20][20];
	uint32_t uidAssign[5];
} UserLevelSetLayout;
bool loadingUserLevelSet;
UserLevelSetLayout *userLevelSet;
bool usingUserLevelSet;
// char saveBuffer[2048];

Entity *templateList[] = {
	NULL,
	&templateBlock,
	&templateSave,
	&templateDeath,
	&templateGem,
	&templateTeleport,
	&templateMiniGem,
	&templateDoubleJump,
	&templateUpBlock,
	&templateDoorOn, 
	&templateDoorOff, 
	&templateSwitch,
	&templateLoopHalfOfKey,
	&templateToothHalfOfKey,
	&templateBoostUp,
	&templateLockedDoor,
	&templateWings,
	&templateGuyHorizontal,
	&templateGuyWall,
	&templateGuyCircle,
	&templateFallingBlock,
	&templateVerticalDeath,
	&templateRocket,
	&templateExclaimBlock,
	&templateHeli,
	&templateBigOlSwitch,
	&templatePuzzleA,
	&templateExit,
	&templatePuzzleB,
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

#if 0
	if (sizeof(sd) > sizeof(saveBuffer) / 2) Panic();
	for (uintptr_t i = 0; i < sizeof(sd); i++) {
		saveBuffer[i * 2 + 0] = 'A' + (((const uint8_t *) &sd)[i] & 15);
		saveBuffer[i * 2 + 1] = 'A' + (((const uint8_t *) &sd)[i] >> 4);
	}
	SaveGameToCookie(saveBuffer, sizeof(sd) * 2);
#endif
}

void RestoreSaveData() {
	int oldDeathCount = sd.deathCount;
	sd = lastCheckPoint;
	sd.deathCount = oldDeathCount;
	player->x = sd.playerX;
	player->y = sd.playerY;
	player->dx = player->dy = 0;
	gd.loadRoom = true;
	gd.insideSave = 20;
	gd.wingsTimer = 0;
	gd.heliTimer = 0;
	gd.rocketTimer = 0;
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
#ifdef LEVEL_EDITOR
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

void SaveM(Entity *entity, int message) {
	if (message == MSG_STEP) {
		entity->alpha = gd.insideSave ? 0x40 : 0xFF;
	}
}

void Particle(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (!entity->timer--) {
			EntityDestroy(entity);
		} else {
			entity->x += entity->dx;
			entity->y += entity->dy;
			entity->dy += entity->extraData / 20.0f;
			if (entity->timer < 8) entity->alpha /= 2;
		}
	}
}

void Teleport(Entity *entity, int message) {
	if (message == MSG_STEP) {
		entity->alpha = (sd.hasUpgrade & UPGRADE_TELEPORT) ? 0xFF : 0x40;
	}
}

void Death(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		int tx = entity->x / 16, ty = entity->y / 16;

		if (ty < GAME_HEIGHT / 16 - 1 && templateList[roomData[sd.roomY * TILES_Y + ty + 1][sd.roomX * TILES_X + tx]] == &templateBlock) {
			entity->texture = TextureSub(tileset, 112, 0, 16, 16);
		} else if (ty > 0 && templateList[roomData[sd.roomY * TILES_Y + ty - 1][sd.roomX * TILES_X + tx]] == &templateBlock) {
			entity->texture = TextureSub(tileset, 96, 0, 16, 16);
		} else if (tx > 0 && templateList[roomData[sd.roomY * TILES_Y + ty][sd.roomX * TILES_X + tx - 1]] == &templateBlock) {
			entity->texture = TextureSub(tileset, 64, 0, 16, 16);
		} else if (tx < GAME_WIDTH / 16 - 1 && templateList[roomData[sd.roomY * TILES_Y + ty][sd.roomX * TILES_X + tx + 1]] == &templateBlock) {
			entity->texture = TextureSub(tileset, 80, 0, 16, 16);
		}

		entity->w -= 6;
		entity->h -= 6;
		entity->x += 3;
		entity->y += 3;
		entity->texOffX -= 3;
		entity->texOffY -= 3;
	}
}

void Puzzle(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		entity->texture = TextureSub(tileset, 48 + 16 * entity->timer, 48, 16, 16);
		entity->uid = gd.puzzleCount;
		gd.puzzleCount++;
	} else if (message == MSG_STEP) {
		if (EntityFindAt(entity, 0, 0, 0, 0, NULL) == player) {
			if (!entity->extraData) {
				entity->timer = 1 - entity->timer;
				entity->texture = TextureSub(tileset, 48 + 16 * entity->timer, 48, 16, 16);
				entity->extraData = 1;
			}
		} else {
			entity->extraData = 0;
		}

		if (entity->tag == TAG_PUZZLE_B ? !entity->timer : entity->timer) {
			gd.puzzleSolution |= 1 << entity->uid;
		} else {
			gd.puzzleSolution &= ~(1 << entity->uid);
		}
	}
}

void GotGem(Entity *entity) {
	if (entity->uid == uidAssign[4]) {
		entity->extraData = 1;
		entity->texture = TextureSub(tileset, 112, 48, 16, 16);
	} else {
		EntityDestroy(entity);
	}
}

void Gem(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		for (uintptr_t i = 0; i < sizeof(sd.gemList) / sizeof(sd.gemList[0]); i++) {
			if (sd.gemList[i] == entity->uid) {
				GotGem(entity);
				break;
			}
		}
	} else if (message == MSG_STEP && !entity->extraData) {
		Entity *particle = EntityCreate(NULL, entity->x + entity->w / 2, entity->y + entity->h / 2, 0);
		particle->dx = ((int8_t) GetRandomByte() / 128.0f);
		particle->dy = ((int8_t) GetRandomByte() / 128.0f) - 1;
		particle->extraData = 1;
		particle->texture = TextureSub(tileset, 134, 112, 5, 5);
		particle->frameRow = 2;
		particle->frameCount = 2;
		particle->stepsPerFrame = 3;
		particle->timer = 50;
		particle->alpha = GetRandomByte();
		particle->layer = -1;
		particle->message = Particle;
	}
}

void DoubleJump(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (gd.onGround == 0 && entity->timer > 2) {
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

void Heli(Entity *entity, int message) {
	if (message == MSG_STEP) {
		entity->alpha = gd.heliTimer ? 0x40 : 0xFF;
	}
}

void Wings(Entity *entity, int message) {
	if (message == MSG_STEP) {
		entity->alpha = gd.wingsTimer ? 0x40 : 0xFF;
	}
}

void Rocket(Entity *entity, int message) {
	if (message == MSG_STEP) {
		entity->alpha = gd.rocketTimer ? 0x40 : 0xFF;
	}
}

void Guy(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (EntityFindAt(entity, entity->dx, entity->dy, 0, FLAG_GUY_SOLID, NULL)
				|| EntityFindAt(entity, entity->dx, entity->dy, 0, FLAG_SOLID, NULL)) {
			entity->dx = -entity->dx;
			entity->dy = -entity->dy;
		} else {
			entity->x += entity->dx;
			entity->y += entity->dy;
		}
	}
}

void GuyCircle(Entity *entity, int message) {
	if (message == MSG_STEP) {
		entity->x += entity->dx * 0.6f;
		entity->y += entity->dy * 0.6f;
		entity->dy -= entity->dx * 0.04f;
		entity->dx += entity->dy * 0.04f;
	}
}

void LockedDoor(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if ((sd.hasUpgrade & UPGRADE_TOOTH_HALF_OF_KEY) && (sd.hasUpgrade & UPGRADE_LOOP_HALF_OF_KEY)) {
			if (entity->alpha) {
				entity->alpha -= 3;
				Entity *particle = EntityCreate(NULL, entity->x + entity->w / 2, entity->y + entity->h / 2, 0);
				particle->dx = ((int8_t) GetRandomByte() / 128.0f) / 2;
				particle->dy = ((int8_t) GetRandomByte() / 128.0f) + 0.5f;
				particle->texture = TextureSub(tileset, 134, 112, 5, 5);
				particle->layer = -1;
				particle->timer = 30;
				particle->message = Particle;
				particle->frameRow = 2;
				particle->frameCount = 2;
				particle->alpha = 120;
				particle->stepsPerFrame = 3;
				entity->flags &= ~FLAG_SOLID;
			}
		}
	}
}

void ExclaimBlock(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (sd.hasUpgrade & UPGRADE_BIG_OL_SWITCH) {
			entity->flags &= ~FLAG_SOLID;
			entity->texture = TextureSub(tileset, 128, 48, 16, 16);
		}
	}
}

void Block(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		if (!(FNV1a(&entity->uid, 4) & 0x415)) { entity->texture = TextureSub(tileset, 128, 64, 16, 16); }
		if (!(FNV1a(&entity->uid, 4) & 0x868)) { entity->texture = TextureSub(tileset, 112, 80, 16, 16); }
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

void FallingBlock(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		entity->h = 1;
	} else if (message == MSG_STEP) {
		if (entity->extraData) {
			entity->dy += 0.3f;
			entity->y += entity->dy;
			entity->flags &= ~FLAG_SOLID;
			entity->timer++;

#if 0
			if (entity->timer == 120) {
				entity->extraData = 0;
				entity->timer = 0;
				entity->y = (entity->uid & 0xFF) * 16;
				entity->dy = 0.0f;
			}
#endif
		} else {
			if (player->dy >= 0 && player->y + player->h - 1 < entity->y) {
				entity->flags |= FLAG_SOLID;
			} else {
				entity->flags &= ~FLAG_SOLID;
			}

			if (entity->x < player->x + player->w 
					&& player->x < entity->x + entity->w
					&& entity->y - 4 < player->y + player->h 
					&& player->y < entity->y
					&& !entity->timer
					&& player->dy >= 0.0f) {
				entity->timer = 10;
			}

			if (entity->timer) {
				entity->timer--;

				if (!entity->timer) {
					entity->extraData = 1;
				}
			}
		}
	}
}

void MiniGem(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		for (uintptr_t i = 0; i < sizeof(sd.gemList) / sizeof(sd.gemList[0]); i++) {
			if (sd.gemList[i] == entity->uid) {
				EntityDestroy(entity);
			}
		}
	}
}

float /* -1..1 */ CheapSineApproximation01(float x /* 0..1 */) {
	return ((((-56.8889f * x + 142.2222f) * x - 103.1111f) * x + 12.4444f) * x + 5.3333f) * x;
}

void DoDeath() {
	for (int i = 0; i < 20; i++) {
		Entity *particle = EntityCreate(NULL, player->x + player->w / 2, player->y + player->h / 2, 0);
		particle->dx = ((int8_t) GetRandomByte() / 128.0f);
		particle->dy = ((int8_t) GetRandomByte() / 128.0f);
		particle->texture = TextureSub(tileset, 114, 112, 5, 5);
		particle->layer = 4;
		particle->timer = 20;
		particle->message = Particle;
		particle->frameRow = 2;
		particle->frameCount = 2;
		particle->stepsPerFrame = 3;
	}

	if (gd.heliTimer) {
		PLAY_SOUND("audio/sfx_heli2.ogg", false, 0.0);
	}

	if (gd.rocketTimer) {
		PLAY_SOUND("audio/sfx_rocket.ogg", false, 0.0);
	}

	sd.deathCount++;
	gd.deathTimer = 1.0f;
	player->dx = 0;
	player->dy = -1;
	PLAY_SOUND("audio/sfx_death.wav", false, 0.5);
}

void NormalMovement() {
	float leftAcceleration = gd.horizontallyBouncing ? 0.05f : gd.wingsTimer ? 0.2f : gd.wallJumped && player->dx > 0 ? 0.05f : 0.45f;
	float rightAcceleration = gd.horizontallyBouncing ? 0.05f : gd.wingsTimer ? 0.2f : gd.wallJumped && player->dx < 0 ? 0.05f : 0.45f;
	player->dx *= gd.wingsTimer ? 0.995f : 0.85f;
	if (keysHeld[KEY_LEFT]) { player->dx += (-2.4f - player->dx) * leftAcceleration; gd.lastDirection = true; }
	if (keysHeld[KEY_RIGHT]) { player->dx += (2.4f - player->dx) * rightAcceleration; gd.lastDirection = false; }
	if (player->dx >= -0.5f && player->dx <= 0.5f && !keysHeld[KEY_LEFT] && !keysHeld[KEY_RIGHT]) { player->dx = 0; }
	if (player->dx < -5.0f) player->dx = -5.0f;
	if (player->dx > 5.0f) player->dx = 5.0f;
	player->dy += 0.2f;

	bool pushingAgainstLeftWall = keysHeld[KEY_LEFT] && EntityFindAt(player, -1, 0, 0, FLAG_SOLID, NULL);
	bool pushingAgainstRightWall = keysHeld[KEY_RIGHT] && EntityFindAt(player, 1, 0, 0, FLAG_SOLID, NULL);
	bool pushingAgainstWall = pushingAgainstLeftWall || pushingAgainstRightWall;
	float maxDY = 6 + (keysHeld[KEY_DOWN] ? 2 : 0) + (pushingAgainstWall ? -4.5f : 0);
	if (player->dy > maxDY) player->dy = maxDY;

	if (pushingAgainstWall && player->dy > 0.0f) {
		Entity *particle = EntityCreate(NULL, player->x + player->w / 2, player->y + player->h / 2, 0);
		particle->dx = ((int8_t) GetRandomByte() / 128.0f) / 2.0f;
		particle->dy = (((int8_t) GetRandomByte() / 128.0f) - 1) / 2.0f;
		particle->extraData = 1;
		particle->texture = TextureSub(tileset, 134, 112, 5, 5);
		particle->frameRow = 2;
		particle->frameCount = 2;
		particle->stepsPerFrame = 3;
		particle->timer = 20;
		particle->alpha = GetRandomByte();
		particle->layer = 0;
		particle->message = Particle;
	}

	if (EntityFindAt(player, 0, 3, 0, FLAG_SOLID, NULL)) { gd.onGround = 0, gd.canDoubleJump = false; } 
	else if (gd.onGround < 100) { gd.onGround++; }
	bool canJumpOffGround = gd.onGround < 5;
	if (gd.wallJumped) gd.wallJumped--;

	bool jump = keysHeld[KEY_UP] || keysHeld['X'];
	if (jump && !gd.holdingJump) { gd.jumpBuffer = 4; }
	else if (gd.jumpBuffer) { gd.jumpBuffer--; }
	gd.holdingJump = jump;

	if (gd.jumpBuffer > 0) {
		if (canJumpOffGround || gd.wingsTimer) {
			float jumpHeight = -4;
			if (sd.hasUpgrade & UPGRADE_JUMP_HEIGHT) jumpHeight = -5.18f;
			player->dy = jumpHeight;
			gd.onGround = 100;
			gd.jumpBuffer = 0;

			if (gd.wingsTimer) {
				PLAY_SOUND("audio/sfx_flap.wav", false, 0.1);
			} else {
				PLAY_SOUND("audio/sfx_jump.wav", false, 0.1);
			}
		} else if (gd.canDoubleJump) {
			gd.canDoubleJump = false;
			player->dy = -4;
			gd.onGround = 100;
			gd.jumpBuffer = 0;
			PLAY_SOUND("audio/sfx_jump.wav", false, 0.1);
		} else if (pushingAgainstWall) {
			float jumpHeight = -4;
			if (sd.hasUpgrade & UPGRADE_JUMP_HEIGHT) jumpHeight = -5;
			jumpHeight /= 1.6f;
			player->dx = pushingAgainstRightWall ? -6.0f : 6.0f;
			gd.wallJumped = 15;
			player->dy = jumpHeight;
			gd.onGround = 100;
			gd.jumpBuffer = 0;
			PLAY_SOUND("audio/sfx_jump.wav", false, 0.1);
		}
	}

	if (!jump && player->dy < -2 && !gd.boosting) { player->dy *= 0.7f; }
}

void Player(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (gd.endOfGame) {
			if (gd.endTimer < 1.0f) {
				gd.endTimer += 0.005f;
				entity->x += 0.2f;
				entity->y -= 0.09f;
			}

			if (gd.endTimer > 1.0f) {
				gd.endTimer = 1.0f;
			}

			gd.endTimer2++;

			float t = (float) gd.endTimer2 / 160.0f;
			entity->y += CheapSineApproximation01(t - __builtin_floorf(t)) / 11.0f;

			{
				Entity *particle = EntityCreate(NULL, entity->x + entity->w / 2, entity->y + entity->h / 2, 0);
				particle->dx = ((int8_t) GetRandomByte() / 128.0f);
				particle->dy = ((int8_t) GetRandomByte() / 128.0f);
				particle->texture = TextureSub(tileset, 134, 112, 5, 5);
				particle->timer = 50;
				particle->alpha = 128;
				particle->message = Particle;
				particle->layer = 1;
			}

			return;
		} else {
			gd.endTimer = 0;
			gd.endTimer2 = 0;
		}

		if (player->x < -7         ) { player->x += GAME_WIDTH ; sd.roomX--; gd.loadRoom = true; return; }
		if (player->y < -7         ) { player->y += GAME_HEIGHT; sd.roomY--; gd.loadRoom = true; return; }
		if (player->x > GAME_WIDTH ) { player->x -= GAME_WIDTH ; sd.roomX++; gd.loadRoom = true; return; }
		if (player->y > GAME_HEIGHT) { player->y -= GAME_HEIGHT; sd.roomY++; gd.loadRoom = true; return; }

		if (gd.deathTimer > 0.0f) {
			gd.deathTimer -= 1.0f / 20.0f;
			entity->y += entity->dy;
			entity->alpha = 255 * gd.deathTimer;

			if (gd.deathTimer <= 0.0f) {
				gd.deathTimer = 0.0f;
				gd.spawnParticlesAtPlayer = true;
				RestoreSaveData();
			}
			
			return;
		} else {
			entity->alpha = 255;
		}

		if (gd.teleportTimer > 0.0f) {
			gd.teleportTimer -= 1.0f / 40.0f;
			entity->y += -0.1f;
			entity->alpha = 255 * gd.teleportTimer;

			if (gd.teleportTimer <= 0.0f) {
				gd.teleportTimer = 0.0f;
				gd.afterTeleport = 3;
				gd.spawnParticlesAtPlayer = true;

				player->x = gd.teleportToX;
				player->y = gd.teleportToY;
				sd.roomX = gd.teleportToRX;
				sd.roomY = gd.teleportToRY;
				gd.loadRoom = true;
				WriteSaveData(player->x, player->y);
				lastTeleport = lastCheckPoint;
				gd.justSaved = 60;
				gd.spawnParticlesAtPlayer = true;
			}
			
			return;
		} else {
			entity->alpha = 255;
		}

		if (gd.afterTeleport) {
			gd.afterTeleport--;
		}

		if (gd.spawnParticlesAtPlayer) {
			for (int i = 0; i < 20; i++) {
				Entity *particle = EntityCreate(NULL, entity->x + entity->w / 2, entity->y + entity->h / 2, 0);
				particle->dx = ((int8_t) GetRandomByte() / 128.0f);
				particle->dy = ((int8_t) GetRandomByte() / 128.0f);
				particle->texture = TextureSub(tileset, 134, 112, 5, 5);
				particle->layer = -1;
				particle->timer = 50;
				particle->alpha = 128;
				particle->message = Particle;
				particle->frameRow = 2;
				particle->frameCount = 2;
				particle->stepsPerFrame = 3;
			}

			gd.spawnParticlesAtPlayer = false;
		}

		if (gd.rocketTimer) {
			if (keysHeld[KEY_LEFT]) {
				keysHeld[KEY_LEFT] = 0;
				gd.rocketDirection = (gd.rocketDirection + 7) % 8;
			} else if (keysHeld[KEY_RIGHT]) {
				keysHeld[KEY_RIGHT] = 0;
				gd.rocketDirection = (gd.rocketDirection + 1) % 8;
			}

			if (gd.rocketDirection == 0) {
				player->dx = 0.0f;
				player->dy = -gd.rocketSpeed;
			} else if (gd.rocketDirection == 1) {
				player->dx = gd.rocketSpeed * 0.8f;
				player->dy = -gd.rocketSpeed * 0.8f;
			} else if (gd.rocketDirection == 2) {
				player->dx = gd.rocketSpeed;
				player->dy = 0.0f;
			} else if (gd.rocketDirection == 3) {
				player->dx = gd.rocketSpeed * 0.8f;
				player->dy = gd.rocketSpeed * 0.8f;
			} else if (gd.rocketDirection == 4) {
				player->dx = 0.0f;
				player->dy = gd.rocketSpeed;
			} else if (gd.rocketDirection == 5) {
				player->dx = -gd.rocketSpeed * 0.8f;
				player->dy = gd.rocketSpeed * 0.8f;
			} else if (gd.rocketDirection == 6) {
				player->dx = -gd.rocketSpeed;
				player->dy = 0.0f;
			} else if (gd.rocketDirection == 7) {
				player->dx = -gd.rocketSpeed * 0.8f;
				player->dy = -gd.rocketSpeed * 0.8f;
			}

			{
				Entity *particle = EntityCreate(NULL, entity->x + entity->w / 2, entity->y + entity->h / 2, 0);
				particle->dx = ((int8_t) GetRandomByte() / 128.0f);
				particle->dy = ((int8_t) GetRandomByte() / 128.0f);
				particle->texture = TextureSub(tileset, 134, 112, 5, 5);
				particle->layer = -1;
				particle->timer = 20;
				particle->alpha = 128;
				particle->message = Particle;
				particle->frameRow = 2;
				particle->frameCount = 2;
				particle->stepsPerFrame = 3;
			}
		} else if (gd.heliTimer) {
			player->dx *= 0.5f;
			player->dy *= 0.5f;
			if (keysHeld[KEY_LEFT]) { player->dx += (-3 - player->dx) * 0.8f; } 
			if (keysHeld[KEY_RIGHT]) { player->dx += (3 - player->dx) * 0.8f; }
			if (keysHeld[KEY_UP]) { player->dy += (-3 - player->dy) * 0.8f; } 
			if (keysHeld[KEY_DOWN]) { player->dy += (3 - player->dy) * 0.8f; }
		} else {
			NormalMovement();
		}

		Entity *collide = NULL;

		collide = EntityFindAt(player, player->dx, 0, 0, FLAG_SOLID, NULL);

		if (collide) { 
			if (player->dx > 0) { player->x = collide->x - player->w; }
			if (player->dx < 0) { player->x = collide->x + collide->w; }
			player->dx = gd.wingsTimer ? -1.1f * player->dx : 0; 

			if (gd.wingsTimer) {
				gd.horizontallyBouncing = 30;
				player->dy -= 0.4f;
			}
		} else {
			player->x += player->dx;
		}

		if (gd.horizontallyBouncing) {
			gd.horizontallyBouncing--;
		}

		collide = EntityFindAt(player, 0, player->dy, 0, FLAG_SOLID, NULL);

		if (collide) { 
			if (player->dy > 0) { player->y = collide->y - player->h; player->dy = 0; }
			if (player->dy < 0) { player->y = collide->y + collide->h; player->dy *= 0.6f; }
		} else {
			player->y += player->dy;
		}

		if (gd.rocketTimer) {
			player->texture = TextureSub(tileset, gd.rocketDirection * 16, 64, 16, 16);
			player->texOffX = -4;
			player->texOffY = -5;
			player->frameCount = 1;
		} else if (gd.heliTimer) {
			player->texture = TextureSub(tileset, 80, 32, 16, 16);
			player->frameCount = 2;
			player->frameRow = 2;
			player->stepsPerFrame = 4;
			player->texOffX = -4;
			player->texOffY = -4;
		} else {
			player->frameCount = 1;

			if (gd.lastDirection) {
				player->texture = TextureSub(tileset, 16, 0, 16, 16);
				player->texOffX = -4;
				player->texOffY = -8;
			} else {
				player->texture = TextureSub(tileset, 0, 0, 16, 16);
				player->texOffX = -5;
				player->texOffY = -8;
			}
		}

		collide = EntityFindAt(player, 0, 0, TAG_SAVE, 0, NULL);

		if (collide) {
			if (!gd.insideSave) {
				gd.justSaved = 60;
				WriteSaveData(collide->x, collide->y);
				PLAY_SOUND("audio/sfx_save.wav", false, 0.4);
			}

			gd.insideSave = 20;
		} else if (gd.insideSave) {
			gd.insideSave--;
		}

		if (gd.justSaved) gd.justSaved--;

		collide = EntityFindAt(player, 0, 0, TAG_TELEPORT, 0, NULL);

		if (collide) {
			if ((sd.hasUpgrade & UPGRADE_TELEPORT) && keysHeld['T']) {
				gd.teleportTimer = 1.0f;
				PLAY_SOUND("audio/sfx_tele.wav", false, 0.8);
				keysHeld['T'] = 0;
				bool foundSelf = false;

				for (int k = 0; k < 2; k++) {
					for (int y = 0; y < 20 * TILES_Y; y++) {
						for (int x = 0; x < 20 * TILES_X; x++) {
							int rx = x / TILES_X, ry = y / TILES_Y;
							int i = x % TILES_X, j = y % TILES_Y;
							uint32_t uid = (rx << 24) | (ry << 16) | (i << 8) | (j << 0);

							if (foundSelf && (sd.mapSeen[ry] & (1 << rx)) 
									&& templateList[roomData[y][x]] == &templateTeleport) {
								gd.teleportToX = i * 16;
								gd.teleportToY = j * 16;
								gd.teleportToRX = rx;
								gd.teleportToRY = ry;
								return;
							} else if (uid == collide->uid) {
								foundSelf = true;
							}
						}
					}
				}
			}

			gd.insideTeleport = 1;
		} else if (gd.insideTeleport) {
			gd.insideTeleport--;
		}

		collide = EntityFindAt(player, 0, 0, TAG_MINIGEM, 0, NULL);

		if (collide) {
			for (uintptr_t i = 0; i < sizeof(sd.gemList) / sizeof(sd.gemList[0]); i++) {
				if (sd.gemList[i] == 0) {
					sd.gemList[i] = collide->uid;
					break;
				}
			}

			PLAY_SOUND("audio/sfx_minigem.wav", false, 0.6);
			EntityDestroy(collide);
		}

		if (gd.unlockTimer) gd.unlockTimer--;

		collide = EntityFindAt(player, 0, 0, TAG_GEM, 0, NULL);

		if (collide && !collide->extraData) {
			gd.unlockTimer = 420;

#ifdef LEVEL_EDITOR
			printf("%x\n", collide->uid);
#endif

			if (collide->uid == uidAssign[0]) {
				gd.justUnlocked = 1;
				sd.hasUpgrade |= UPGRADE_TELEPORT;
			} else if (collide->uid == uidAssign[1]) {
				gd.justUnlocked = 3;
				sd.hasUpgrade |= UPGRADE_LOOP_HALF_OF_KEY;
			} else if (collide->uid == uidAssign[2]) {
				gd.justUnlocked = 2;
				sd.hasUpgrade |= UPGRADE_TOOTH_HALF_OF_KEY;
			} else if (collide->uid == uidAssign[3]) {
				gd.justUnlocked = 0;
				sd.hasUpgrade |= UPGRADE_JUMP_HEIGHT;
			} else if (collide->uid == uidAssign[4]) {
				gd.justUnlocked = 4;
				sd.hasUpgrade |= UPGRADE_BIG_OL_SWITCH;
			} else {
				gd.justUnlocked = -1;
			}

			for (uintptr_t i = 0; i < sizeof(sd.gemList) / sizeof(sd.gemList[0]); i++) {
				if (sd.gemList[i] == 0) {
					sd.gemList[i] = collide->uid;
					break;
				}
			}

			PLAY_SOUND("audio/sfx_gem.wav", false, 0.6);
			GotGem(collide);
		}

		if (gd.unlockTimer) gd.unlockTimer--;

		collide = EntityFindAt(player, 0, 0, 0, FLAG_DEATH, NULL);

		if (collide) {
			DoDeath();
			return;
		}

		collide = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_DOUBLE_JUMP, 0, NULL);

		if (collide && collide->timer == 0) {
			collide->timer = 180;
			gd.canDoubleJump = true;
		}

		if (gd.canDoubleJump) {
			Entity *particle = EntityCreate(NULL, entity->x + entity->w / 2, entity->y + entity->h / 2, 0);
			particle->dx = ((int8_t) GetRandomByte() / 128.0f) / 2;
			particle->dy = ((int8_t) GetRandomByte() / 128.0f) + 0.5f;
			particle->texture = TextureSub(tileset, 124, 112, 5, 5);
			particle->layer = -1;
			particle->timer = 30;
			particle->message = Particle;
			particle->alpha = 120;
			particle->frameRow = 2;
			particle->frameCount = 2;
			particle->stepsPerFrame = 3;
		}

		collide = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_SWITCH, 0, NULL);

		if (collide) {
			EntityDestroy(collide);
			PLAY_SOUND("audio/sfx_switch.wav", false, 0.7);

			for (int i = 0; i < maxUsedEntity; i++) {
				Entity *e = &entities[i];

				if (e->slot == SLOT_USED && e->tag == TAG_DOOR_ON) {
					e->tag = TAG_DOOR_OFF;
					e->flags = 0;
					e->texture = TextureSub(tileset, 96, 16, 16, 16);
				} else if (e->slot == SLOT_USED && e->tag == TAG_DOOR_OFF) {
					e->tag = TAG_DOOR_ON;
					e->flags = FLAG_SOLID;
					e->texture = TextureSub(tileset, 80, 16, 16, 16);
				}
			}
		}
		
		collide = EntityFindAt(entity, 0, 0, TAG_BOOST_UP, 0, NULL);

		if (collide) {
			if (player->dy > -3) {
				PLAY_SOUND("audio/sfx_boost.wav", false, 0.3);
			}

			player->dy = -6;
			gd.boosting = 20;
		}

		if (gd.boosting) {
			gd.boosting--;
		}

		collide = EntityFindAt(entity, 0, 0, TAG_WINGS, 0, NULL);

		if (collide && !gd.wingsTimer) {
			gd.wingsTimer = 1.0f;
		} else if (gd.wingsTimer) {
			gd.wingsTimer -= 0.35f / GAME_FPS;

			if (gd.wingsTimer < 0) {
				gd.wingsTimer = 0;
			}
		}

		collide = EntityFindAt(entity, 0, 0, TAG_ROCKET, 0, NULL);

		if (collide && !gd.rocketTimer) {
			gd.rocketTimer = 1.0f;
			gd.rocketSpeed = 0.0f;
			gd.rocketDirection = 0;

			PLAY_SOUND("audio/sfx_rocket.ogg", false, 0.5);

			keysHeld[KEY_LEFT] = 0;
			keysHeld[KEY_RIGHT] = 0;

			if (player->dx < -0.5f) {
				if (player->dy < -0.5f) {
					gd.rocketDirection = 7;
				} else if (player->dy > 0.5f) {
					gd.rocketDirection = 5;
				} else {
					gd.rocketDirection = 6;
				}
			} else if (player->dx > 0.5f) {
				if (player->dy < -0.5f) {
					gd.rocketDirection = 1;
				} else if (player->dy > 0.5f) {
					gd.rocketDirection = 3;
				} else {
					gd.rocketDirection = 2;
				}
			} else {
				if (player->dy < -0.5f) {
					gd.rocketDirection = 0;
				} else if (player->dy > 0.5f) {
					gd.rocketDirection = 4;
				} else {
					gd.rocketDirection = 0;
				}
			}
		} else if (gd.rocketTimer) {
			gd.rocketTimer -= 0.2f / GAME_FPS;
			gd.rocketSpeed += (2.3f - gd.rocketSpeed) * 0.05f;
			if (gd.rocketTimer < 0) { gd.rocketTimer = 0; }
		}

		collide = EntityFindAt(entity, 0, 0, TAG_GUY, 0, NULL);

		if (collide) {
			if (!EntityFindAt(entity, 0, -2.0f - entity->dy, TAG_GUY, 0, NULL)) {
				entity->dy = -5.0f;
				EntityDestroy(collide);
				PLAY_SOUND("audio/sfx_guy.wav", false, 0.6);

				for (int i = 0; i < 10; i++) {
					Entity *particle = EntityCreate(NULL, entity->x + entity->w / 2, entity->y + entity->h / 2, 0);
					particle->dx = ((int8_t) GetRandomByte() / 128.0f) / 2.0f;
					particle->dy = ((int8_t) GetRandomByte() / 128.0f) - 1;
					particle->extraData = 1;
					particle->texture = TextureSub(tileset, 114, 112, 5, 5);
					particle->frameRow = 2;
					particle->frameCount = 2;
					particle->stepsPerFrame = 3;
					particle->timer = 50;
					particle->alpha = GetRandomByte();
					particle->layer = -1;
					particle->message = Particle;
				}
			} else {
				DoDeath();
				return;
			}
		}

		collide = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_HELI, 0, NULL);

		if (collide && !gd.heliTimer) {
			PLAY_SOUND("audio/sfx_heli2.ogg", false, 0.5);
			gd.heliTimer = 1.0f;
		} else if (gd.heliTimer) {
			gd.heliTimer -= 0.25f / GAME_FPS;
			if (gd.heliTimer < 0) { gd.heliTimer = 0; }
		}

		collide = EntityFind(entity->x, entity->y, entity->w, entity->h, TAG_EXIT, 0, NULL);

		if (collide) {
			EntityDestroy(collide);
			gd.endOfGame = true;
			gd.endTimer = 0.0f;
			gd.endTimer2 = 0;
			player->texture = TextureSub(tileset, 0, 0, 16, 16);
			player->texOffX = -5;
			player->texOffY = -8;
			PLAY_SOUND("audio/bgm.opus", false, 0.0);

			if (usingUserLevelSet) {
				gd.endTimer2 = 3600;
			} else {
				PLAY_SOUND("audio/sfx_win.opus", false, 0.8);
			}
		}
	} else if (message == MSG_DRAW) {
		if (gd.wingsTimer) {
			Blend(templateWings.texture, 
					gd.lastDirection ? player->x + 8 : player->x - 14, 
					player->y - 16 + 3.0f * CheapSineApproximation01((gd.globalTimer % 60) / 60.0f), 255);
		}
	}
}

void InitialiseSaveData() {
	memset(&sd, 0, sizeof(sd));
	player->x = 100;
	player->y = 100;
	sd.roomX = 10;
	sd.roomY = 10;
}

void ApplyPalette() {
	paletteIndex = !disablePaletteSwaps && sd.roomX >= 0 && sd.roomX < 15 && sd.roomY >= 0 && sd.roomY < 15 
		? paletteForRoom[sd.roomY][sd.roomX] : 0;
	if (paletteIndex == 9) paletteIndex = 0;

	for (int i = 0; i < tileset.width * tileset.height; i++) {
		tileset.bits[i] = (tileset.bits[i] & 0xFF000000) | palettes[paletteIndex][recolor[i]];
	}
}

void LoadRoom() {
	if (gd.loadRoom) {
		if (sd.roomX < 1 || sd.roomY < 1 || sd.roomX > 18 || sd.roomY > 18) {
			RestoreSaveData();
		}

		gd.loadRoom = false;
		gd.puzzleCount = 0;

		sd.mapSeen[sd.roomY] |= 1 << sd.roomX;

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

		ApplyPalette();

		REPAINT_WORLD_OVERVIEW_CANVAS();
	}
}

void GameRender();

void GameInitialise() {
	userLevelSet = (UserLevelSetLayout *) Allocate(sizeof(UserLevelSetLayout));
#if 0
	for (uintptr_t i = 0; i < sizeof(sd); i++) {
		((uint8_t *) &sd)[i] = (saveData[i * 2 + 0] - 'A') | ((saveData[i * 2 + 1] - 'A') << 4);
	}
	uint32_t oldHash = sd.hash;
	sd.hash = 0;
	sd.hash = FNV1a(&sd, sizeof(sd));
	bool saveValid = sd.hash == oldHash;
	if (!saveValid) memset(&sd, 0, sizeof(sd));
#else
	bool saveValid = false;
#endif

#ifdef LEVEL_EDITOR
	FILE *f = fopen("embed/world.dat", "rb");
	fread(&roomData[0][0], 1, sizeof(roomData), f);
	fclose(f);
#else
	memcpy(roomData, world_dat, sizeof(roomData));
	memcpy(paletteForRoom, palettes_dat, sizeof(paletteForRoom));
#endif

	tileset = TextureCreate(tileset_png, sizeof(tileset_png));

	templateBlock.flags = FLAG_SOLID;
	templateBlock.texture = TextureSub(tileset, 32, 0, 16, 16);
	templateBlock.message = Block;

	templateUpBlock.flags = FLAG_SOLID | FLAG_GUY_SOLID;
	templateUpBlock.texture = TextureSub(tileset, 64, 16, 16, 16);
	templateUpBlock.message = UpBlock;

	templateFallingBlock.flags = FLAG_SOLID | FLAG_GUY_SOLID;
	templateFallingBlock.texture = TextureSub(tileset, 16, 48, 16, 16);
	templateFallingBlock.message = FallingBlock;

	templateDoorOn.flags = FLAG_SOLID;
	templateDoorOn.tag = TAG_DOOR_ON;
	templateDoorOn.texture = TextureSub(tileset, 80, 16, 16, 16);
	templateDoorOff.tag = TAG_DOOR_OFF;
	templateDoorOff.texture = TextureSub(tileset, 96, 16, 16, 16);
	templateSwitch.tag = TAG_SWITCH;
	templateSwitch.texture = TextureSub(tileset, 112, 16, 16, 16);

	templateToothHalfOfKey.tag = TAG_GEM;
	templateToothHalfOfKey.texture = TextureSub(tileset, 32, 32, 16, 16);
	templateToothHalfOfKey.message = Gem;
	templateLoopHalfOfKey.tag = TAG_GEM;
	templateLoopHalfOfKey.texture = TextureSub(tileset, 16, 32, 16, 16);
	templateLoopHalfOfKey.message = Gem;
	templateBigOlSwitch.tag = TAG_GEM;
	templateBigOlSwitch.texture = TextureSub(tileset, 96, 48, 16, 16);
	templateBigOlSwitch.message = Gem;

	templateExit.tag = TAG_EXIT;
	templateExit.texture = TextureSub(tileset, 96, 80, 16, 16);

	templatePuzzleA.tag = TAG_PUZZLE_A;
	templatePuzzleA.texture = TextureSub(tileset, 48, 48, 16, 16);
	templatePuzzleA.message = Puzzle;

	templatePuzzleB.tag = TAG_PUZZLE_B;
	templatePuzzleB.texture = TextureSub(tileset, 48 + 16, 48, 16, 16);
	templatePuzzleB.message = Puzzle;

	templateSave.tag = TAG_SAVE;
	templateSave.texture = TextureSub(tileset, 48, 0, 16, 16);
	templateSave.message = SaveM;

	templateDeath.texture = TextureSub(tileset, 112, 0, 16, 16);
	templateDeath.message = Death;
	templateDeath.flags = FLAG_DEATH;

	templateGem.tag = TAG_GEM;
	templateGem.texture = TextureSub(tileset, 0, 16, 16, 16);
	templateGem.message = Gem;

	templateMiniGem.tag = TAG_MINIGEM;
	templateMiniGem.texture = TextureSub(tileset, 32, 16, 16, 16);
	templateMiniGem.message = MiniGem;

	templateTeleport.tag = TAG_TELEPORT;
	templateTeleport.texture = TextureSub(tileset, 16, 16, 16, 16);
	templateTeleport.message = Teleport;

	templateDoubleJump.tag = TAG_DOUBLE_JUMP;
	templateDoubleJump.texture = TextureSub(tileset, 48, 16, 16, 16);
	templateDoubleJump.message = DoubleJump;

	templateBoostUp.tag = TAG_BOOST_UP;
	templateBoostUp.texture = TextureSub(tileset, 48, 32, 16, 16);

	templateExclaimBlock.tag = TAG_EXCLAIM_BLOCK;
	templateExclaimBlock.flags = FLAG_SOLID;
	templateExclaimBlock.texture = TextureSub(tileset, 80, 48, 16, 16);
	templateExclaimBlock.message = ExclaimBlock;

	templateLockedDoor.tag = TAG_LOCKED_DOOR;
	templateLockedDoor.flags = FLAG_SOLID;
	templateLockedDoor.texture = TextureSub(tileset, 64, 32, 16, 16);
	templateLockedDoor.message = LockedDoor;

	templateWings.tag = TAG_WINGS;
	templateWings.texture = TextureSub(tileset, 112, 32, 16, 16);
	templateWings.message = Wings;

	templateRocket.tag = TAG_ROCKET;
	templateRocket.texture = TextureSub(tileset, 0, 64, 16, 16);
	templateRocket.message = Rocket;

	templateHeli.tag = TAG_HELI;
	templateHeli.texture = TextureSub(tileset, 80, 32, 16, 16);
	templateHeli.message = Heli;

	templateGuyHorizontal.tag = TAG_GUY;
	templateGuyHorizontal.texture = TextureSub(tileset, 0, 48, 16, 16);
	templateGuyHorizontal.message = Guy;
	templateGuyHorizontal.dx = -2;

	templateVerticalDeath.flags = FLAG_DEATH;
	templateVerticalDeath.texture = TextureSub(tileset, 32, 48, 16, 16);
	templateVerticalDeath.message = Guy;
	templateVerticalDeath.dy = -1.2f;

	templateGuyCircle.tag = TAG_GUY;
	templateGuyCircle.texture = TextureSub(tileset, 0, 48, 16, 16);
	templateGuyCircle.message = GuyCircle;
	templateGuyCircle.dx = -2;

	templateGuyWall.flags = FLAG_GUY_SOLID;
	templateGuyWall.texture = TextureSub(tileset, 16, 96, 16, 16);
	templateGuyWall.layer = -100;

	player = EntityCreate(NULL, 16, 16, 0);
	player->message = Player;
	player->w = 7;
	player->h = 7;
	player->texture = TextureSub(tileset, 0, 0, 16, 16);
	player->texOffX = -5;
	player->texOffY = -8;
	player->layer = 2;

	gd.loadRoom = true;

	if (!saveValid) {
		InitialiseSaveData();
	} else {
		player->x = sd.playerX;
		player->y = sd.playerY;
		gd.insideSave = 20;
	}

	lastCheckPoint = sd;
	lastTeleport = sd;

	PLAY_SOUND("audio/bgm.opus", true, 0.5);

	// Preload sound effects.
	PLAY_SOUND("audio/sfx_boost.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_death.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_flap.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_gem.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_guy.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_heli2.ogg", false, 0.0);
	PLAY_SOUND("audio/sfx_jump.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_minigem.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_rocket.ogg", false, 0.0);
	PLAY_SOUND("audio/sfx_save.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_secret.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_switch.wav", false, 0.0);
	PLAY_SOUND("audio/sfx_tele.wav", false, 0.0);

	if (sizeof(recolor) != tileset.width * tileset.height) {
		Panic();
	}

#ifndef LEVEL_EDITOR
	disablePaletteSwaps = true;
#endif

	for (int i = 0; i < tileset.width * tileset.height; i++) {
		for (int j = 0; j < 7; j++) {
			if (palettes[0][j] == (tileset.bits[i] & 0xFFFFFF)) {
				recolor[i] = j;
			}
		}
	}

#ifdef RENDER_MAP
	uint32_t *bigImage = (uint32_t *) calloc(GAME_WIDTH * 20 * GAME_HEIGHT * 20, 4);

	for (int x = 1; x < 19; x++) {
		for (int y = 1; y < 19; y++) {
			renderMapMode = true;
			editMode = false;
			gd.loadRoom = true;
			sd.roomX = x;
			sd.roomY = y;
			disablePaletteSwaps = false;
			LoadRoom();
			GameRender();

			for (int i = 0; i < GAME_HEIGHT; i++) {
				for (int j = 0; j < GAME_WIDTH; j++) {
					bigImage[(i + y * GAME_HEIGHT) * GAME_WIDTH * 20 + (j + x * GAME_WIDTH)] = imageData[i * GAME_WIDTH + j];
				}
			}
		}
	}

	stbi_write_jpg("bin/map.jpg", GAME_WIDTH * 20, GAME_HEIGHT * 20, 4, bigImage, 60);
#endif
}

void GameUpdate() {
	if (loadingUserLevelSet) {
		loadingUserLevelSet = LoadUserLevelSet((uint8_t *) userLevelSet, sizeof(UserLevelSetLayout));

		if (!loadingUserLevelSet && userLevelSet->version == 1) {
			usingUserLevelSet = true;
			memcpy(roomData, &userLevelSet->roomData, sizeof(roomData));
			memcpy(paletteForRoom, &userLevelSet->paletteForRoom, sizeof(paletteForRoom));
			memcpy(uidAssign, &userLevelSet->uidAssign, sizeof(uidAssign));
		}

		return;
	}

	if (keysHeld['M']) {
		keysHeld['M'] = 0;
		gd.menuOpen = !gd.menuOpen;
		gd.menuIndex = 0;
		gd.menuConfirm = false;
	}

	if (gd.menuOpen) {
		if (gd.menuConfirm) {
			if (keysHeld['Y']) {
				keysHeld['Y'] = 0;
				gd.menuOpen = false;
				gd.menuConfirm = false;

				if (gd.menuIndex == 1) {
					RestoreSaveData();
				} else if (gd.menuIndex == 2) {
					lastCheckPoint = lastTeleport;
					RestoreSaveData();
				} else if (gd.menuIndex == 3) {
					memset(&gd, 0, sizeof(gd));
					RestoreSaveData();
					InitialiseSaveData();
					usingUserLevelSet = false;
					userLevelSet->version = 0;

					if (LoadUserLevelSet((uint8_t *) userLevelSet, sizeof(UserLevelSetLayout))) {
						loadingUserLevelSet = true;
					}
				}
			} else if (keysHeld['X'] || keysHeld['N']) {
				gd.menuOpen = false;
				gd.menuConfirm = false;
			}
		} else {
			if (keysHeld[KEY_UP]) {
				keysHeld[KEY_UP] = 0;
				gd.menuIndex--;
			}

			if (keysHeld[KEY_DOWN]) {
				keysHeld[KEY_DOWN] = 0;
				gd.menuIndex++;
			}

			if (gd.menuIndex < 0) {
				gd.menuIndex = 0;
			}

			if (gd.menuIndex > 4) {
				gd.menuIndex = 4;
			}

			if (keysHeld['X']) {
				keysHeld['X'] = 0;

				if (gd.menuIndex == 0) {
					gd.menuOpen = false;
					gd.menuConfirm = false;
				} else if (gd.menuIndex == 4) {
					gd.menuOpen = false;
					gd.menuConfirm = false;
					disablePaletteSwaps = !disablePaletteSwaps;
					ApplyPalette();
				} else {
					gd.menuConfirm = true;
				}
			}
		}

		return;
	}

	gd.globalTimer++;

	LoadRoom();

	if (!editMode) {
		for (int i = 0; i < maxUsedEntity; i++) {
			Entity *entity = entities + i;

			if (entity->slot == SLOT_USED) {
				entity->message(entity, MSG_STEP);
				entity->tick++;
			}
		}

#ifdef LEVEL_EDITOR
		if (luigiKeys[UI_KEYCODE_LETTER('P')]) {
			editMode = true;
			luigiKeys[UI_KEYCODE_LETTER('P')] = false;
		}
#endif
	} else {
#ifdef LEVEL_EDITOR
		if (!clickToChooseStartPosition && !clickToChooseUIDAssignment && mouseLeft && mouseX >= 0 && mouseY >= 0 && mouseX < GAME_WIDTH && mouseY < GAME_HEIGHT) {
			roomData[sd.roomY * TILES_Y + mouseY / 16][sd.roomX * TILES_X + mouseX / 16] = selectedEntityIndex;
			REPAINT_WORLD_OVERVIEW_CANVAS();
		} else if (mouseRight && mouseX >= 0 && mouseY >= 0 && mouseX < GAME_WIDTH && mouseY < GAME_HEIGHT) {
			roomData[sd.roomY * TILES_Y + mouseY / 16][sd.roomX * TILES_X + mouseX / 16] = 0;
			REPAINT_WORLD_OVERVIEW_CANVAS();
		}

		if (luigiKeys[UI_KEYCODE_LETTER('S')]) {
			FILE *f = fopen("embed/world.dat", "wb");
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fclose(f);
			f = fopen("embed/palettes.dat", "wb");
			fwrite(&paletteForRoom[0][0], 1, sizeof(paletteForRoom), f);
			fclose(f);
			f = fopen("embed/uid_assign.dat", "wb");
			fwrite(&uidAssign[0], 1, sizeof(uidAssign), f);
			fclose(f);
			luigiKeys[UI_KEYCODE_LETTER('S')] = false;
			f = fopen("bin/level_set.dat", "wb");
			uint32_t version = 1;
			fwrite(&version, 1, sizeof(version), f);
			fwrite(&roomData[0][0], 1, sizeof(roomData), f);
			fwrite(&paletteForRoom[0][0], 1, sizeof(paletteForRoom), f);
			fwrite(&uidAssign[0], 1, sizeof(uidAssign), f);
			fclose(f);
		} else if (luigiKeys[UI_KEYCODE_LETTER('P')] || (clickToChooseStartPosition && mouseLeft)) {
			luigiKeys[UI_KEYCODE_LETTER('S')] = true;
			editMode = false;
			player->x = mouseX - player->w / 2;
			player->y = mouseY - player->h / 2;
			gd.loadRoom = true;
			gd.wingsTimer = 0;
			gd.heliTimer = 0;
			gd.rocketTimer = 0;
			WriteSaveData(player->x, player->y);
			luigiKeys[UI_KEYCODE_LETTER('P')] = false;
			clickToChooseStartPosition = false;
			mouseLeft = false;
		} else if (clickToChooseUIDAssignment && mouseLeft) {
			int i = mouseX >> 4, j = mouseY >> 4;
			uidAssign[clickToChooseUIDAssignment - 1] = (sd.roomX << 24) | (sd.roomY << 16) | (i << 8) | (j << 0);
			clickToChooseUIDAssignment = 0;
			mouseLeft = false;
		}

		if (luigiKeys[UI_KEYCODE_LEFT]) {
			luigiKeys[UI_KEYCODE_LETTER('S')] = true;
			luigiKeys[UI_KEYCODE_LEFT] = false;
			sd.roomX--;
			gd.loadRoom = true;
		} else if (luigiKeys[UI_KEYCODE_RIGHT]) {
			luigiKeys[UI_KEYCODE_LETTER('S')] = true;
			luigiKeys[UI_KEYCODE_RIGHT] = false;
			sd.roomX++;
			gd.loadRoom = true;
		} else if (luigiKeys[UI_KEYCODE_UP]) {
			luigiKeys[UI_KEYCODE_LETTER('S')] = true;
			luigiKeys[UI_KEYCODE_UP] = false;
			sd.roomY--;
			gd.loadRoom = true;
		} else if (luigiKeys[UI_KEYCODE_DOWN]) {
			luigiKeys[UI_KEYCODE_LETTER('S')] = true;
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

	if ((gd.puzzleSolution == 0 || gd.puzzleSolution == (1 << gd.puzzleCount) - 1) && gd.puzzleCount) {
		for (int i = 0; i < maxUsedEntity; i++) {
			Entity *e = &entities[i];

			if (e->slot == SLOT_USED && e->tag == TAG_DOOR_ON) {
				e->tag = TAG_DOOR_OFF;
				e->flags = 0;
				e->texture = TextureSub(tileset, 96, 16, 16, 16);
			}
		}
	}

	for (int i = 0; i < maxUsedEntity; i++) {
		Entity *entity = entities + i;

		if ((entity->slot & SLOT_USED) && (entity->slot & SLOT_DESTROYING)) {
			entity->slot &= ~SLOT_USED;
		}
	}
}

void DrawString(const char *cString, int x, int y, bool center, int alpha, float rippleAmount) {
	if (center) {
		for (int i = 0; cString[i]; i++) {
			x -= 4;
		}
	}

	for (int i = 0; cString[i]; i++) {
		char c = cString[i];
		Texture t = c >= 'A' && c <= 'R' ? TextureSub(tileset, (c - 'A') * 8, 120, 8, 8) 
			: c >= 'S' && c <= 'Z' ? TextureSub(tileset, (c - 'S') * 8, 112, 8, 8)
			: c >= '0' && c <= '9' ? TextureSub(tileset, (c - '0') * 8 + 32, 104, 8, 8)
			: c == '~' ? TextureSub(tileset, 64, 112, 8, 8) 
			: c == '?' ? TextureSub(tileset, 72, 112, 8, 8) 
			: c == ',' ? TextureSub(tileset, 80, 112, 8, 8)
			: c == '.' ? TextureSub(tileset, 88, 112, 8, 8)
			: TextureSub(tileset, 0, 104, 8, 8);
		float p = i * 0.1f + gd.globalTimer * 0.01f;
		float r = CheapSineApproximation01(p - __builtin_floorf(p));
		Blend(t, x, y + rippleAmount * r, alpha > 255 ? 255 : alpha < 0 ? 0 : alpha);
		x += 8;
	}
}

int GemCount() {
	int gemCount = 0;

	for (uintptr_t i = 0; i < sizeof(sd.gemList) / sizeof(sd.gemList[0]); i++) {
		if (sd.gemList[i]) {
			uint8_t rx = sd.gemList[i] >> 24;
			uint8_t ry = sd.gemList[i] >> 16;
			uint8_t x = sd.gemList[i] >> 8;
			uint8_t y = sd.gemList[i] >> 0;

			Entity *template = templateList[roomData[ry * TILES_Y + y][rx * TILES_X + x]];

			if (template == &templateGem || template == &templateMiniGem) { 					
				gemCount++;
			}
		}
	}

	return gemCount;
}

void GameRender() {
	if (gd.endTimer2 >= 3600) {
		Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, 0);
		char c[13] = "GEM COUNT   ";
		c[10] = (GemCount() / 10) + '0';
		c[11] = (GemCount() % 10) + '0';
		DrawString(c, GAME_WIDTH / 2, GAME_HEIGHT / 2 - 40, true, 255, 0);
		char d[17] = "DEATH COUNT     ";
		int deathCount = sd.deathCount > 9999 ? 9999 : sd.deathCount;
		d[12] = ((deathCount / 1000) % 10) + '0';
		d[13] = ((deathCount /  100) % 10) + '0';
		d[14] = ((deathCount /   10) % 10) + '0';
		d[15] = ((deathCount /    1) % 10) + '0';
		DrawString(d, GAME_WIDTH / 2, GAME_HEIGHT / 2 - 20, true, 255, 0);
		if (deathCount < 9) {
			DrawString("!! SUPER PLAYER !!", GAME_WIDTH / 2, GAME_HEIGHT / 2, true, 255, 0);
		}
		return;
	}

	Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, palettes[paletteIndex][3]);

	for (int y = 0; y < TILES_Y; y++) {
		for (int x = 0; x < TILES_X; x++) {
			uint32_t i = (sd.roomX << 24) | (sd.roomY << 16) | (x << 8) | (y << 0);
			uint32_t hash = FNV1a(&i, 4);

			if ((hash & 30) == 0) {
				if (hash & (1 << 14)) {
					Blit(TextureSub(tileset, 128, 80, 16, 16), x * 16, y * 16);
				} else {
					Blit(TextureSub(tileset, 128, 32, 16, 16), x * 16, y * 16);
				}
			}
		}
	}

	if (!editMode) {
		for (int layer = -1; layer <= 4; layer++) {
			float alpha = 1.0f;

			if (layer < 1 && gd.endOfGame) {
				alpha *= 1.0f - gd.endTimer;
			}

			for (int i = 0; i < maxUsedEntity; i++) {
				Entity *entity = entities + i;
				if ((~entity->slot & SLOT_USED) || (entity->layer != layer)) continue;
				if (renderMapMode && entity == player) continue;

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

				if (showCollisionShapes) {
					FillBlend(entity->x, entity->y, entity->w, 1, 0x80FF00FF);
					FillBlend(entity->x, entity->y, 1, entity->h, 0x80FF00FF);
					FillBlend(entity->x, entity->y + entity->h - 1, entity->w, 1, 0x80FF00FF);
					FillBlend(entity->x + entity->w - 1, entity->y, 1, entity->h, 0x80FF00FF);
				}
			}
		}

		if (!renderMapMode) {
			if (gd.justSaved) {
				DrawString("SAVED", player->x + player->w / 2, player->y - 16, true, 255 / 40 * gd.justSaved, 0);
			} else if (gd.insideTeleport) {
				if (sd.hasUpgrade & UPGRADE_TELEPORT) {
					DrawString("PRESS T", player->x + player->w / 2, player->y - 16, true, 255, 0);
				} else {
					DrawString("???", player->x + player->w / 2, player->y - 16, true, 255, 0);
				}
			}

			if (gd.unlockTimer) {
				Fill(0, 84, GAME_WIDTH, 70, palettes[paletteIndex][2]);
				Fill(0, 84, GAME_WIDTH, 1, palettes[paletteIndex][6]);
				Fill(0, 84 + 70, GAME_WIDTH, 1, palettes[paletteIndex][1]);
				DrawString(gd.justUnlocked == 3 || gd.justUnlocked == 2 ? "CAT HAS MADE ACQUISITION"
						: gd.justUnlocked == 4 ? "BIG OL SWITCH PRESSED"
						: "POWER OF THE GEM AWAKENS A GREAT STRENGTH", 
						GAME_WIDTH / 2, 100, true, 255, 0);
				DrawString(gd.justUnlocked == 0 ? "~THE MIGHTY LEAP~ INCREASE OF JUMP" 
					: gd.justUnlocked == 1 ? "~CONTINUUM WARP~ TELEPORT ACCESS" 
					: gd.justUnlocked == 2 ? "TOOTH HALF OF KEY" 
					: gd.justUnlocked == 3 ? "LOOP HALF OF KEY" 
					: gd.justUnlocked == 4 ? "BEEP HAS TURNED INTO BOOP"
					: "NO UID ASSIGN", GAME_WIDTH / 2, 130, true, 255, 0);
			}

			FillBlend(0, 0, 64, 16, (uint32_t) (0x80 * (1.0f - gd.endTimer)) << 24);

			{
				int gemCount = GemCount();
				Blend(TextureSub(tileset, 112, 104, 8, 8), 4, 4, 255 * (1.0f - gd.endTimer));
				char string[4];
				string[0] = 'X';
				string[1] = (gemCount / 10) + '0';
				string[2] = (gemCount % 10) + '0';
				string[3] = 0;
				DrawString(string, 12, 4, false, 255 * (1.0f - gd.endTimer), 0);
			}

			if ((sd.hasUpgrade & UPGRADE_TOOTH_HALF_OF_KEY) && (sd.hasUpgrade & UPGRADE_LOOP_HALF_OF_KEY)) {
				Blend(TextureSub(tileset, 0, 32, 16, 16), 44, 0, 255 * (1.0f - gd.endTimer));
			} else if (sd.hasUpgrade & UPGRADE_TOOTH_HALF_OF_KEY) {
				Blend(TextureSub(tileset, 32, 32, 16, 16), 44, 0, 255 * (1.0f - gd.endTimer));
			} else if (sd.hasUpgrade & UPGRADE_LOOP_HALF_OF_KEY) {
				Blend(TextureSub(tileset, 16, 32, 16, 16), 44, 0, 255 * (1.0f - gd.endTimer));
			}
			
			if (gd.wingsTimer || gd.rocketTimer || gd.heliTimer) {
				Fill(GAME_WIDTH - 110, 10, 100, 10, 0x0000FF);
				Fill(GAME_WIDTH - 110, 10, 100 * (gd.wingsTimer ?: gd.rocketTimer ?: gd.heliTimer), 10, 0x00FF00);
			}

			if (gd.teleportTimer || gd.afterTeleport) {
				FillBlend(0, 0, GAME_WIDTH, GAME_HEIGHT, (((uint32_t) ((1.0f - gd.teleportTimer) * 255.0f)) << 24) | 0xFFFFFF);
			}
		}

#define DO_ENDING_TEXT(startTime, endTime, string, row) \
		DrawString(string, 250, GAME_HEIGHT / 2 - 20 + row * 15, true, \
				FadeInOut(LinearMap(startTime, endTime, 0, 1, gd.endTimer2)) * 255.0f, 1.5f)
		DO_ENDING_TEXT(120, 360, "GOOD EVENING, CAT", 0);
		DO_ENDING_TEXT(340, 680, "ONCE AGAIN, YOU HAVE MADE", 0);
		DO_ENDING_TEXT(340, 680, "A FOOL OUT OF US...", 1);
		DO_ENDING_TEXT(660, 980, "TO SAY WE ARE DISPLEASED WOULD", 0);
		DO_ENDING_TEXT(660, 980, "BE A GREAT UNDERSTATEMENT", 1);
		DO_ENDING_TEXT(960, 1280, "WORDS CANNOT DESCRIBE", 0);
		DO_ENDING_TEXT(960, 1280, "MY ETERNAL RAGE", 1);
		DO_ENDING_TEXT(1260, 1580, "EVERY GEM YOU STEAL BRINGS", 0);
		DO_ENDING_TEXT(1260, 1580, "ANOTHER CURSE UPON OUR WORLD", 1);
		DO_ENDING_TEXT(1560, 1880, "ARE YOU EVEN PREPARED TO", 0);
		DO_ENDING_TEXT(1560, 1880, "ACKNOWLEDGE WHAT YOU HAVE DONE?", 1);
		DrawString("MEOW~", 80, GAME_HEIGHT / 2 - 30, true, FadeInOut(LinearMap(2000, 2050, 0, 1, gd.endTimer2)) * 255.0f, 0);
		DO_ENDING_TEXT(2160, 2480, "YOUR ARROGANCE KNOWS NO ENDS...", 0);
		DO_ENDING_TEXT(2460, 2680, "...", 0);
		DO_ENDING_TEXT(2660, 2980, "VERY WELL.", 0);
		DO_ENDING_TEXT(2960, 3280, "WE WILL MEET AGAIN.", 0);

		if (gd.endTimer2 >= 3400 && gd.endTimer2 <= 3600) {
			FillBlend(0, 0, GAME_WIDTH, GAME_HEIGHT, (uint32_t) (LinearMap(3400, 3600, 0, 1, gd.endTimer2) * 255) << 24);
		}

#ifdef LEVEL_EDITOR
	} else {
		for (int j = 0; j < TILES_Y; j++) {
			for (int i = 0; i < TILES_X; i++) {
				if (roomData[sd.roomY * TILES_Y + j][sd.roomX * TILES_X + i]) {
					Blend(templateList[roomData[sd.roomY * TILES_Y + j][sd.roomX * TILES_X + i]]->texture, i * 16, j * 16, 0xFF);
				}
			}
		}

		if (!mouseRight && !clickToChooseStartPosition && !clickToChooseUIDAssignment) {
			Blend(templateList[selectedEntityIndex]->texture, mouseX & ~15, mouseY & ~15, 0x80);
		}

		if (clickToChooseUIDAssignment) {
			FillBlend(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x30000000);
			DrawString("CLICK TO ASSIGN UID", 40, 40, false, 0xC0, 0);
			FillBlend(mouseX & ~15, mouseY & ~15, 16, 16, 0x80FF00FF);
		}

		if (clickToChooseStartPosition) {
			FillBlend(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x30000000);
			DrawString("CLICK START LOCATION", 40, 40, false, 0xC0, 0);
			Blend(player->texture, mouseX - player->w / 2 + player->texOffX, mouseY - player->h / 2 + player->texOffY, 0x80);
		}

		if (sd.roomX == 10 && sd.roomY == 10) {
			FillBlend(100, 100, 16, 16, 0xFFFFFF00);
			DrawString("ENTRY", 100, 100, false, 0xC0, 0);
		}
#endif
	}

	if (gd.menuOpen) {
		FillBlend(0, 0, GAME_WIDTH, GAME_HEIGHT, 0xA0000000);

		if (gd.menuConfirm) {
			if (gd.menuIndex == 1) {
				DrawString("ARE YOU SURE YOU WANT TO LOAD", 40, 40, false, 255, 0);
				DrawString("THE LAST SAVE?", 40, 60, false, 255, 0);
			} else if (gd.menuIndex == 2) {
				DrawString("ARE YOU SURE YOU WANT TO LOAD", 40, 40, false, 255, 0);
				DrawString("THE LAST SAVE AT A TELEPORTER?", 40, 60, false, 255, 0);
			} else if (gd.menuIndex == 3) {
				DrawString("ARE YOU SURE YOU WANT TO START A NEW GAME?", 40, 40, false, 255, 0);
				DrawString("ALL PROGRESS WILL BE LOST.", 40, 60, false, 255, 0);
			}

			DrawString("  PRESS Y TO CONFIRM", 40, 80, false, 255, 0);
			DrawString("  PRESS X TO CANCEL", 40, 100, false, 255, 0);
		} else {
			DrawString("RESUME GAME", 40, 40, false, 255, 0);
			DrawString("GO BACK TO LAST SAVEPOINT", 40, 60, false, 255, 0);
			DrawString("GO BACK TO LAST TELEPORTER", 40, 80, false, 255, 0);
			DrawString("START A NEW GAME", 40, 100, false, 255, 0);
			DrawString(disablePaletteSwaps ? "ENABLE PALETTE SWAPS" : "DISABLE PALETTE SWAPS", 40, 120, false, 255, 0);
			Blend(TextureSub(tileset, 0, 0, 16, 16), 20, 34 + 20 * gd.menuIndex, 255);
		}
	}
}
