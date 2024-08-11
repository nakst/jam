// working title: celeste clone no. 9748658148

// TIME REMAINING: 0 seconds

#define SAVE_KEY "TriJam282_9cd70f87-8f12-471d-8ac6-54f081ac7e48"

#define SLOT_USED       (1 << 0)
#define SLOT_DESTROYING (1 << 1)

#define MSG_STEP      (1)
#define MSG_DRAW      (2)
#define MSG_CREATE    (3)

#define TAG_PLAYER (1)

#define FLAG_SOLID (1 << 0)
#define FLAG_CANWJ (1 << 1)
#define FLAG_RESTORE (1 << 2)

typedef struct TilesetRegion {
	uint16_t x, y, w, h;
} TilesetRegion;

typedef struct Entity {
	uint8_t slot;
	int8_t layer;
	uint16_t tag;
	uint16_t flags;
	uint8_t alpha;
	uint8_t extraData;
	int16_t timer;
	int8_t i8[2];
	uint32_t tick;
	int x, y, w, h;
	float dx, dy;
	float residualMoveX, residualMoveY;
	int8_t frameCount, stepsPerFrame, frameRow, texOffX, texOffY;
	uint8_t v8[3];
	int u32[2];
	TilesetRegion texture;
	void (*message)(struct Entity *entity, int message);
} Entity;

typedef struct SaveData {
	int byteCount;
	int x, y;
	int time;
	bool finished;
} SaveData;

typedef struct GameData {
	int globalTimer;
	bool lastDirection;
	bool canDash;
	int loadWorld;
	bool consumedJump;
	int framesSinceOnGround;
	int inWallJump;
	int jumpRequest;
	int inDash;
	bool dashRequest;
	int freezeFrames;
	int justFinishedDash;
	bool canDoubleJump;
	int deathTimer;
	bool holdingDash;
	int holdReset;
	bool didReset;
} GameData;

typedef struct WorldData {
#define ROOM_Y (18)
#define WORLD_HEIGHT (500)
#define WORLD_WIDTH (18)
	uint8_t tiles[WORLD_HEIGHT][WORLD_WIDTH];
} WorldData;

static WorldData worldData;
static Texture tileset;

#define MAX_ENTITIES (800)
static Entity entities[MAX_ENTITIES];
static int maxUsedEntity;
static Entity *player;
static GameData gd;
static SaveData sd;

#ifdef UI_LINUX
static bool editMode = false;
#else
static bool editMode = false;
#endif
static int editTile = 1;
static int editY = 19;

static void WriteSaveData(int x, int y) {
	sd.x = x;
	sd.y = y;
	WriteSave(SAVE_KEY, strlen(SAVE_KEY), (const char *) &sd, sizeof(sd));
}

static void InitialiseSaveData() {
	memset(&sd, 0, sizeof(sd));
	sd.y = 20 * 8 * ROOM_Y - 16;
	sd.x = 16;
}

static Texture TilesetRegionToTexture(TilesetRegion r) {
	return TextureSub(tileset, r.x, r.y, r.w, r.h);
}

static TilesetRegion TilesetRegionMake(int x, int y, int w, int h) {
	TilesetRegion r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return r;
}

static void NullMessage(Entity *entity, int message) {}

static Entity *EntityCreate(const Entity *templateEntity, float x, float y) {
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
		if (!entity->w) entity->w = entity->texture.w;
		if (!entity->h) entity->h = entity->texture.h;
		entity->alpha = 0xFF;
		if (!entity->message) entity->message = NullMessage;
		entity->message(entity, MSG_CREATE);
		if (maxUsedEntity <= i) maxUsedEntity = i + 1;
		return entity;
	}
	
	static Entity fake = {};
	PRINT("too many entities!\n");
	return &fake;
}

static void EntityDestroy(Entity *entity) {
	entity->slot |= SLOT_DESTROYING;
}

static Entity *EntityFind(int x, int y, int w, int h, uint8_t tag, uint8_t flags, Entity *exclude) {
	if (tag == TAG_PLAYER && gd.deathTimer) {
		return NULL;
	}

	int upto = tag == TAG_PLAYER ? 1 : maxUsedEntity;

	for (int i = 0; i < upto; i++) {
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

static Entity *EntityFindAt(Entity *at, int dx, int dy, uint8_t tag, uint8_t flags, Entity *exclude) {
	return EntityFind(at->x + dx, at->y + dy, at->w, at->h, tag, flags, exclude);
}

static Entity *ActorMoveX(Entity *entity, float d, Entity *exclude) {
	entity->residualMoveX += d;
	int i = __builtin_floorf(entity->residualMoveX + 0.5f);
	int s = i > 0 ? 1 : -1;

	while (i) {
		Entity *collidedWith = EntityFindAt(entity, s, 0, 0, FLAG_SOLID, exclude);

		if (collidedWith) {
			entity->residualMoveX = 0;
			return collidedWith;
		}

		entity->x += s;
		i -= s;
		entity->residualMoveX -= s;
	}

	return NULL;
}

static Entity *ActorMoveY(Entity *entity, float d, Entity *exclude) {
	entity->residualMoveY += d;
	int i = __builtin_floorf(entity->residualMoveY + 0.5f);
	int s = i > 0 ? 1 : -1;

	while (i) {
		Entity *collidedWith = EntityFindAt(entity, 0, s, 0, FLAG_SOLID, exclude);

		if (collidedWith) {
			entity->residualMoveY = 0;
			return collidedWith;
		}

		entity->y += s;
		i -= s;
		entity->residualMoveY -= s;
	}

	return NULL;
}

static void BasicMove(Entity *entity, float dx, float dy) {
	entity->residualMoveX += dx;
	int ix = __builtin_floorf(entity->residualMoveX + 0.5f);
	entity->x += ix;
	entity->residualMoveX -= ix;
	entity->residualMoveY += dy;
	int iy = __builtin_floorf(entity->residualMoveY + 0.5f);
	entity->y += iy;
	entity->residualMoveY -= iy;
}

static void Block(Entity *entity, int message) {
	if (message == MSG_CREATE) {
		if (entity->y >= 8 && worldData.tiles[entity->y / 8 - 1][entity->x / 8] != 1) {
			entity->texture = TilesetRegionMake(16, 0, 8, 8);
		}
	} else if (message == MSG_DRAW) {
		if (entity->y >= WORLD_HEIGHT * 8 - 8 || worldData.tiles[entity->y / 8 + 1][entity->x / 8] != 1) {
			Fill(entity->x, entity->y + 7, 8, 1, 0xff9d9d9d);
		}

		if (entity->x >= GAME_WIDTH - 8 || worldData.tiles[entity->y / 8][entity->x / 8 + 1] != 1) {
			Fill(entity->x + 7, entity->y, 1, 8, 0xff9d9d9d);
		}

		if (entity->x < 8 || worldData.tiles[entity->y / 8][entity->x / 8 - 1] != 1) {
			Fill(entity->x, entity->y, 1, 8, 0xff9d9d9d);
		}
	}
}

static void RestoreDash(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (!gd.canDash && !entity->u32[0] && EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
			gd.canDash = true;
			entity->u32[0] = 120;
		}

		entity->texture = TilesetRegionMake(16, 8 + (entity->u32[0] ? 8 : 0), 8, 8);

		if (entity->u32[0]) {
			entity->u32[0]--;
		}
	}
}

static void RestoreJump(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (!gd.canDoubleJump && !entity->u32[0] && EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
			gd.canDoubleJump = true;
			entity->u32[0] = 120;
		}

		entity->texture = TilesetRegionMake(24, 8 + (entity->u32[0] ? 8 : 0), 8, 8);

		if (entity->u32[0]) {
			entity->u32[0]--;
		}
	}
}

static void BreakBlock(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (entity->u32[1]) {
			entity->u32[1]--;

			if (entity->u32[1] == 0) {
				if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
					entity->u32[1] = 1;
				} else {
					entity->flags |= FLAG_SOLID;
					entity->texture = TilesetRegionMake(8, 16, 8, 8);
				}
			}
		} else if (entity->u32[0]) {
			entity->u32[0]--;
			entity->texOffX = GetRandomByte() % 3 - 1;
			entity->texOffY = GetRandomByte() % 3 - 1;

			if (entity->u32[0] == 0) {
				entity->flags &= ~FLAG_SOLID;
				entity->texture = TilesetRegionMake(0, 16, 8, 8);
				entity->u32[1] = 120;
				entity->texOffX = 0;
				entity->texOffY = 0;
			}
		} else {
			if (EntityFindAt(entity, 0, -1, TAG_PLAYER, 0, 0) && player->dy > 0) {
				entity->u32[0] = 20;
			}
		}
	}
}

static void Spikes(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
			gd.freezeFrames = 5;
			gd.deathTimer = 20;
			PLAY_SOUND("audio/die.wav", false, 0.5);
		}
	} else if (message == MSG_CREATE) {
		int tx = entity->x / 8;
		int ty = entity->y / 8;

		if (ty < WORLD_HEIGHT - 1 && worldData.tiles[entity->y / 8 + 1][entity->x / 8] == 1) {
			entity->h = 2;
			entity->y += 6;
			entity->texOffY -= 6;
		} else if (ty > 0 && worldData.tiles[entity->y / 8 - 1][entity->x / 8] == 1) {
			entity->h = 2;
			entity->texture = TilesetRegionMake(48, 0, 8, 8);
		} else if (tx < WORLD_WIDTH - 1 && worldData.tiles[entity->y / 8][entity->x / 8 + 1] == 1) {
			entity->w = 2;
			entity->x += 6;
			entity->texOffX -= 6;
			entity->texture = TilesetRegionMake(40, 0, 8, 8);
		} else if (tx > 0 && worldData.tiles[entity->y / 8][entity->x / 8 - 1] == 1) {
			entity->w = 2;
			entity->texture = TilesetRegionMake(56, 0, 8, 8);
		} else {
			entity->h = 2;
			entity->y += 6;
			entity->texOffY -= 6;
		}
	}
}

static void Checkpoint(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (sd.x == entity->x && sd.y == entity->y) {
			entity->texture = TilesetRegionMake(32, 8, 8, 8);
		} else {
			entity->texture = TilesetRegionMake(40, 8, 8, 8);

			// (entity->y < sd.y || !sd.y) && 

			if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
				WriteSaveData(entity->x, entity->y);
				PLAY_SOUND("audio/checkpt.wav", false, 0.5);
			}
		}
	}
}

static void Summit(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (EntityFindAt(entity, 0, 0, TAG_PLAYER, 0, 0)) {
			sd.finished = true;
		}
	} else if (message == MSG_CREATE) {
		entity->texture = TilesetRegionMake(0, 0, 0, 0);
	} else if (message == MSG_DRAW) {
		if (sd.finished) {
			Blend(TextureSub(tileset, 0, 27, 51, 17), GAME_WIDTH / 2 - 51 / 2, GAME_HEIGHT / 2 - 17 + cameraY, 255);

			int f = sd.time % 60;
			int s = (sd.time / 60) % 60;
			int m = sd.time / 3600;

			int x = GAME_WIDTH / 2 - 51 / 2 + 48 + 4;
			int y = GAME_HEIGHT / 2 - 17 + cameraY + 12;

			x -= 4; Blend(TextureSub(tileset, 4 * (f % 10), 45, 3, 5), x, y, 255); f /= 10;
			x -= 4; Blend(TextureSub(tileset, 4 * (f % 10), 45, 3, 5), x, y, 255); f /= 10;
			x -= 2; Blend(TextureSub(tileset, 42, 45, 1, 5), x, y, 255);
			x -= 4; Blend(TextureSub(tileset, 4 * (s % 10), 45, 3, 5), x, y, 255); s /= 10;
			x -= 4; Blend(TextureSub(tileset, 4 * (s % 10), 45, 3, 5), x, y, 255); s /= 10;
			x -= 2; Blend(TextureSub(tileset, 40, 45, 1, 5), x, y, 255);
			do { x -= 4; Blend(TextureSub(tileset, 4 * (m % 10), 45, 3, 5), x, y, 255); m /= 10; } while (m);
		}
	}
}

static void Particle(Entity *entity, int message) {
	if (message == MSG_STEP) {
		if (--entity->i8[0] == 0) {
			EntityDestroy(entity);
		} else {
			BasicMove(entity, entity->dx, entity->dy);
		}
	}
}

static void NormalMovement() {
	bool canWallJump = false;

	float maxDX = 1.45;

	if (gd.inWallJump) {
	} else if (keysHeld[KEY_LEFT] && keysHeld[KEY_RIGHT]) {
		player->dx = 0;
	} else if (keysHeld[KEY_LEFT]) {
		if (player->dx > -maxDX) {
			player->dx = -maxDX;
			gd.lastDirection = true;
		}

		if (EntityFindAt(player, -1, 0, 0, FLAG_CANWJ, 0)) {
			if (player->dy > 0.5) {
				player->dy = 0.5;
			}

			canWallJump = true;
		}
	} else if (keysHeld[KEY_RIGHT]) {
		if (player->dx < maxDX) {
			player->dx = maxDX;
			gd.lastDirection = false;
		}

		if (EntityFindAt(player, 1, 0, 0, FLAG_CANWJ, 0)) {
			if (player->dy > 0.5) {
				player->dy = 0.5;
			}

			canWallJump = true;
		}
	} else {
		if (EntityFindAt(player, 0, 1, 0, FLAG_SOLID, 0)) {
			player->dx *= 0.4;
		} else {
			player->dx *= 0.7;
		}
	}

	if (player->dx < -maxDX) {
		player->dx += 0.2;
	}

	if (player->dx > maxDX) {
		player->dx -= 0.2;
	}

	player->dy += 0.12;

	ActorMoveX(player, player->dx, NULL);

	if (ActorMoveY(player, player->dy, NULL)) {
		player->dy = 0;
	}

	if (EntityFindAt(player, 0, 2, 0, FLAG_SOLID, 0)) {
		gd.framesSinceOnGround = 0;
		gd.canDoubleJump = false;
	}

	if (!gd.justFinishedDash && EntityFindAt(player, 0, 1, 0, FLAG_RESTORE, 0)) {
		gd.canDash = true;
	}

	if (keysHeld['X'] && !gd.consumedJump) {
		gd.jumpRequest = 5;
		gd.consumedJump = true;
	}

	if (gd.jumpRequest) {
		if (gd.framesSinceOnGround < 8) {
			if (player->dy > -2) { PLAY_SOUND("audio/jump.wav", false, 0.5); }
			player->dy = -2.5;
		} else if (canWallJump) {
			player->dy = -2.4;
			player->dx = keysHeld[KEY_LEFT] ? 1.3 : -1.3;
			gd.inWallJump = 10;
			PLAY_SOUND("audio/jump.wav", false, 0.5);
		} else if (gd.canDoubleJump) {
			player->dy = -2.2;
			gd.canDoubleJump = false;
			PLAY_SOUND("audio/jump.wav", false, 0.5);
		}
	}

	if (!keysHeld['X']) {
		gd.consumedJump = false;
	}

	if (gd.inWallJump) {
		gd.inWallJump--;
	}

	if (gd.jumpRequest) {
		gd.jumpRequest--;
	}

	gd.framesSinceOnGround++;

	if (!keysHeld['X'] && player->dy < 0) {
		player->dy += 0.25;
	}

	float maxDY = keysHeld[KEY_DOWN] ? 5 : 2.4;

	if (player->dy > maxDY) {
		player->dy = maxDY;
	}

	if (keysHeld['Z']) {
		if (gd.canDash && !gd.holdingDash) {
			gd.dashRequest = true;
			gd.freezeFrames = 2;
			gd.canDash = false;
		}

		gd.holdingDash = true;
	} else {
		gd.holdingDash = false;
	}

	if (gd.justFinishedDash) {
		gd.justFinishedDash--;
	}

	if (gd.canDoubleJump) {
		Entity *e = EntityCreate(NULL, player->x + player->w / 2, player->y + player->h / 2);
		e->message = Particle;
		float a = GetRandomByte() / 255.0f * 6.24f;
		e->dx = Cos(a) * 0.5;
		e->dy = Sin(a) * Sin(a);
		e->i8[0] = 10;
		e->layer = 1;
		e->texture = TilesetRegionMake(50, 8, 2, 2);
	}
}

static void DashMovement() {
	if (gd.dashRequest) {
		gd.inDash = 8;
		gd.dashRequest = false;

		if (keysHeld[KEY_UP]) {
			player->dy = -1;
		} else if (keysHeld[KEY_DOWN]) {
			player->dy = 1;
		} else {
			player->dy = 0;
		}

		if (keysHeld[KEY_LEFT]) {
			player->dx = -1;
		} else if (keysHeld[KEY_RIGHT]) {
			player->dx = 1;
		} else if (!player->dy) {
			player->dx = gd.lastDirection ? -1 : 1;
		} else {
			player->dx = 0;
		}

		float sf = player->dx && player->dy ? 2.5 : 3.2;
		player->dx *= sf;
		player->dy *= sf;

		PLAY_SOUND("audio/dash.wav", false, 0.5);
	}

	gd.inDash--;

	if (ActorMoveX(player, player->dx, NULL)) {
		if (player->dy) {
		} else if (!EntityFindAt(player, player->dx, -1, 0, FLAG_SOLID, 0)) {
			player->y -= 1;
		} else if (!EntityFindAt(player, player->dx, 1, 0, FLAG_SOLID, 0)) {
			player->y += 1;
		} else if (!EntityFindAt(player, player->dx, -2, 0, FLAG_SOLID, 0)) {
			player->y -= 2;
		} else if (!EntityFindAt(player, player->dx, 2, 0, FLAG_SOLID, 0)) {
			player->y += 2;
		}
	}

	if (ActorMoveY(player, player->dy, NULL)) {
		if (player->dx) {
		} else if (!EntityFindAt(player, -1, player->dy, 0, FLAG_SOLID, 0)) {
			player->x -= 1;
		} else if (!EntityFindAt(player, 1, player->dy, 0, FLAG_SOLID, 0)) {
			player->x += 1;
		} else if (!EntityFindAt(player, -2, player->dy, 0, FLAG_SOLID, 0)) {
			player->x -= 2;
		} else if (!EntityFindAt(player, 2, player->dy, 0, FLAG_SOLID, 0)) {
			player->x += 2;
		} else if (!EntityFindAt(player, -3, player->dy, 0, FLAG_SOLID, 0)) {
			player->x -= 3;
		} else if (!EntityFindAt(player, 3, player->dy, 0, FLAG_SOLID, 0)) {
			player->x += 3;
		}
	}

	if (!gd.inDash) {
		gd.justFinishedDash = 10;
	}
}

static void Player(Entity *entity, int message) {
	if (message == MSG_STEP) {
		entity->texture = TilesetRegionMake(gd.lastDirection ? 8 : 0, gd.canDash ? 8 : 0, 8, 8);

		if (gd.deathTimer) {
			gd.deathTimer--;
			BasicMove(player, 0, -0.2f);

			if (!gd.deathTimer) {
				player->x = sd.x;
				player->y = sd.y;
				player->dx = 0;
				player->dy = 0;
				gd.inDash = 0;
				gd.jumpRequest = 0;
				gd.loadWorld = -1;
				gd.holdingDash = true;
			} else {
				Entity *e = EntityCreate(NULL, player->x + player->w / 2, player->y + player->h / 2);
				e->message = Particle;
				float a = GetRandomByte() / 255.0f * 6.24f;
				e->dx = Cos(a);
				e->dy = Sin(a);
				e->i8[0] = 10;
				e->layer = 2;
				e->texture = TilesetRegionMake(48, 8, 2, 2);
			}
		} else if (gd.inDash || gd.dashRequest) {
			DashMovement();
		} else {
			NormalMovement();
		}
	}
}

typedef struct TemplateEntity {
	Entity entity;
	const char *name;
} TemplateEntity;

static const TemplateEntity templateList[] = {
	{},
	{ {	.flags = FLAG_SOLID | FLAG_CANWJ | FLAG_RESTORE,
		  .texture = { 24, 0, 8, 8 },
		  .message = Block,
	  }, "Block", },
	{ {	.texture = { 16, 8, 8, 8 },
		  .message = RestoreDash,
	  }, "RestoreDash", },
	{ {	.texture = { 24, 8, 8, 8 },
		  .message = RestoreJump,
	  }, "RestoreJump", },
	{ {	.texture = { 8, 16, 8, 8 },
		  .message = BreakBlock,
		  .flags = FLAG_SOLID | FLAG_CANWJ,
	  }, "BreakBlock", },
	{ {	.texture = { 32, 0, 8, 8 },
		  .message = Spikes,
	  }, "Spikes", },
	{ {	.texture = { 32, 8, 8, 8 },
		  .message = Checkpoint,
	  }, "Checkpoint", },
	{ {	.texture = { 32, 16, 8, 8 },
		  .message = Summit,
	  }, "Summit", },
};

static void GameInitialise() {
#ifdef UI_LINUX
	FILE *f = fopen("embed/world.dat", "rb");
	if (f) {
		fread(&worldData, 1, sizeof(worldData), f);
		fclose(f);
	}
#else
#ifdef world_dat
	memcpy(&worldData, world_dat, sizeof(worldData));
#endif
#endif

	tileset = TextureCreate(tileset_png, tileset_pngBytes);

	player = EntityCreate(NULL, 8, 8);
	player->message = Player;
	player->w = 6;
	player->h = 5;
	player->texOffX = -1;
	player->texOffY = -3;
	player->layer = 2;
	player->tag = TAG_PLAYER;

	gd.loadWorld = -1;

	// Load the saved game.
	ReadSave(SAVE_KEY, strlen(SAVE_KEY), (char *) &sd, sizeof(sd));
	if (sd.byteCount == 0) { InitialiseSaveData(); }
	sd.byteCount = sizeof(sd);
	player->x = sd.x;
	player->y = sd.y;

	PLAY_SOUND("audio/bgm.opus", true, 0.6);

	// Preload sound effects.
	PLAY_SOUND("audio/die.wav", false, 0.0);
	PLAY_SOUND("audio/checkpt.wav", false, 0.0);
	PLAY_SOUND("audio/jump.wav", false, 0.0);
	PLAY_SOUND("audio/dash.wav", false, 0.0);
}

static void GameUpdate() {
	if (sd.time < 1000000000 && !sd.finished) {
		sd.time++;
	}

	if (gd.freezeFrames) {
		gd.freezeFrames--;
		return;
	}

	if (!keysHeld['R']) {
		gd.didReset = false;
	}

	if (keysHeld['R'] && !gd.didReset) {
		gd.holdReset++;

		if (gd.holdReset == 60) {
			gd.holdReset = 0;
			InitialiseSaveData();
			player->x = sd.x;
			player->y = sd.y;
			gd.inDash = 0;
			gd.jumpRequest = 0;
			gd.loadWorld = -1;
			gd.holdingDash = true;
			gd.didReset = true;
		}
	} else {
		gd.holdReset = 0;
	}

	gd.globalTimer++;

	if (gd.loadWorld != player->y / GAME_HEIGHT) {
		for (int i = 1; i < maxUsedEntity; i++) {
			entities[i].slot = 0;
		}

		maxUsedEntity = 1;

		int from = player->y / GAME_HEIGHT * ROOM_Y - ROOM_Y;
		int to = player->y / GAME_HEIGHT * ROOM_Y + ROOM_Y * 2;

		for (int y = from > 0 ? from : 0; y < WORLD_HEIGHT && y < to; y++) {
			for (int x = 0; x < WORLD_WIDTH; x++) {
				if (worldData.tiles[y][x]) {
					EntityCreate(&templateList[worldData.tiles[y][x]].entity, x * 8, y * 8);
				}
			}
		}

		{ Entity *e = EntityCreate(&templateList[1].entity, -8, 0); e->h = WORLD_HEIGHT * 8; e->flags &= ~FLAG_CANWJ; }
		{ Entity *e = EntityCreate(&templateList[1].entity, GAME_WIDTH, 0); e->h = WORLD_HEIGHT * 8; e->flags &= ~FLAG_CANWJ; }

		gd.loadWorld = player->y / GAME_HEIGHT;
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
		int mouseOverTileX = mouseX / 8;
		int mouseOverTileY = mouseY / 8 + editY * ROOM_Y;

		if (mouseLeft && mouseOverTileX >= 0 && mouseOverTileY >= 0 && mouseOverTileX < WORLD_WIDTH && mouseOverTileY < WORLD_HEIGHT) {
			worldData.tiles[mouseOverTileY][mouseOverTileX] = editTile;
		} else if (mouseRight && mouseOverTileX >= 0 && mouseOverTileY >= 0 && mouseOverTileX < WORLD_WIDTH && mouseOverTileY < WORLD_HEIGHT) {
			worldData.tiles[mouseOverTileY][mouseOverTileX] = 0;
		}

		if (luigiKeys[UI_KEYCODE_LETTER('S')]) {
			FILE *f = fopen("embed/world.dat", "wb");
			fwrite(&worldData, 1, sizeof(worldData), f);
			fclose(f);
		} else if (luigiKeys[UI_KEYCODE_LETTER('P')]) {
			FILE *f = fopen("embed/world.dat", "wb");
			fwrite(&worldData, 1, sizeof(worldData), f);
			fclose(f);

			editMode = false;
			player->x = mouseX;
			player->y = mouseY + editY * GAME_HEIGHT;
			WriteSaveData(player->x, player->y);
			luigiKeys[UI_KEYCODE_LETTER('P')] = false;
			gd.loadWorld = -1;
		} else if (luigiKeys[UI_KEYCODE_DIGIT('1')]) {
			editTile = 1;
		} else if (luigiKeys[UI_KEYCODE_DIGIT('2')]) {
			editTile = 2;
		} else if (luigiKeys[UI_KEYCODE_DIGIT('3')]) {
			editTile = 3;
		} else if (luigiKeys[UI_KEYCODE_DIGIT('4')]) {
			editTile = 4;
		} else if (luigiKeys[UI_KEYCODE_DIGIT('5')]) {
			editTile = 5;
		} else if (luigiKeys[UI_KEYCODE_DIGIT('6')]) {
			editTile = 6;
		} else if (luigiKeys[UI_KEYCODE_DIGIT('7')]) {
			editTile = 7;
		} else if (luigiKeys[UI_KEYCODE_DIGIT('8')]) {
			editTile = 8;
		} else if (luigiKeys[UI_KEYCODE_UP]) {
			if (editY) { editY--; }
			luigiKeys[UI_KEYCODE_UP] = false;
		} else if (luigiKeys[UI_KEYCODE_DOWN]) {
			if (editY != 27) { editY++; }
			luigiKeys[UI_KEYCODE_DOWN] = false;
		}
#endif
	}

	for (int i = 0; i < maxUsedEntity; i++) {
		Entity *entity = entities + i;

		if ((entity->slot & SLOT_USED) && (entity->slot & SLOT_DESTROYING)) {
			entity->slot &= ~SLOT_USED;
		}
	}
}

static void GameRender() {
	drawTarget.width = drawTarget.stride = GAME_WIDTH;
	drawTarget.height = GAME_HEIGHT;
	drawTarget.bits = imageData;

	cameraY = 0;

	if (!editMode) {
		Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, 0xFF202024);
		cameraY = (player->y / GAME_HEIGHT) * GAME_HEIGHT;

		for (int layer = -2; layer <= 4; layer++) {
			float alpha = 1.0f;

			for (int i = 0; i < maxUsedEntity; i++) {
				Entity *entity = entities + i;
				if ((~entity->slot & SLOT_USED) || (entity->layer != layer)) continue;

				int x = entity->x + entity->texOffX;
				int y = entity->y + entity->texOffY;

				if (entity->texture.w && entity->texture.h) {
					int repeatX = entity->frameCount < 0 ? (entity->w + entity->texture.w - 1) / entity->texture.w : 1;
					int repeatY = entity->frameCount < 0 ? (entity->h + entity->texture.h - 1) / entity->texture.h : 1;
					int frameCount = entity->frameCount > 0 ? entity->frameCount : -entity->frameCount;

					for (int j = 0; j < repeatY; j++) {
						for (int i = 0; i < repeatX; i++) {
							uint32_t frame = entity->stepsPerFrame >= 0 
								? ((entity->tick / entity->stepsPerFrame) % frameCount) 
								: (-entity->stepsPerFrame - 1);
							Texture texture = TilesetRegionToTexture(entity->texture);
							texture.bits += (frame % entity->frameRow) * texture.width;
							texture.bits += (frame / entity->frameRow) * texture.height * texture.stride;
							Blend(texture, x + i * entity->texture.w, y + j * entity->texture.h, entity->alpha * alpha);
						}
					}
				}

				entity->message(entity, MSG_DRAW);
			}
		}
#ifdef UI_LINUX
	} else {
		Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x404040);

		for (int j = 0; j < ROOM_Y; j++) {
			for (int i = 0; i < WORLD_WIDTH; i++) {
				if (worldData.tiles[j + editY * ROOM_Y][i]) {
					Blend(TilesetRegionToTexture(templateList[worldData.tiles[j + editY * ROOM_Y][i]].entity.texture), i * 8, j * 8, 0xFF);
				}
			}
		}

		if (!mouseRight) {
			Blend(TilesetRegionToTexture(templateList[editTile].entity.texture), mouseX & ~7, mouseY & ~7, 0x80);
		}
#endif
	}

	if (gd.holdReset) {
		cameraY = 0;
		Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x404040);
		Blend(TextureSub(tileset, 0, 51, 53, 5), GAME_WIDTH / 2 - 51 / 2, GAME_HEIGHT / 2 - 5, 255);
	}
}
