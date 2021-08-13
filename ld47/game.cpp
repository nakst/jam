// #define DEVBUILD

#define TAG_ALL (-1)
#define TAG_SOLID (-2)
#define TAG_KILL (-3)
#define TAG_WORLD (-4)

#define TILE_SIZE (16)

#ifdef DEVBUILD
bool levelEditing = true;
#else
bool levelEditing = false;
#endif

struct Entity {
	uint8_t tag;
	uint8_t layer;
	bool solid, kill, hide;
	char symbol;
	int8_t frameCount, stepsPerFrame;
	Texture *texture;
	float texOffX, texOffY, w, h;
	int uid;
	
	void (*create)(Entity *);
	void (*destroy)(Entity *);
	void (*stepAlways)(Entity *); // Called even if level editing.
	void (*step)(Entity *);
	void (*draw)(Entity *);
	void (*drawAfter)(Entity *);
	
	float x, y, px, py;
	int stepIndex;
	bool isUsed, isDestroyed;
	
	union {
		struct {
			float cx, cy;
			float th, dth;
			float dths;
			int respawn, respawnGrow, dash;
		} player;
		
		struct {
			int random;
		} block;
		
		struct {
			float vel;
		} moveBlock;
		
		struct {
			float th, cx, cy;
		} spin;
		
		struct {
			float vx, vy, th, dth;
			int life;
			uint32_t col;
		} star;
	};
	
	void Destroy() {
		if (isDestroyed) return;
		isDestroyed = true;
		if (destroy) destroy(this);
	}
};

Texture textureDashMessage, textureKeyMessage, textureKey, textureControlsMessage;

Entity typePlayer, typeBlock, typeCheck, typeMoveH, typeStar, typeMoveV, typePowerup, typeShowMessage, typeKey, typeLock, typeSpin, typeEnd;

// All the entities that can be loaded from the rooms.
Entity *entityTypes[] = { 
	&typeBlock, &typeCheck, &typeMoveH, &typeMoveV, &typePowerup, &typeKey, &typeLock, &typeSpin, &typeEnd,
	nullptr,
};
	
int levelTick;
bool justLoaded = true;

#define MAX_ENTITIES (1000)

struct SaveState {
	float checkX, checkY;
	int roomX, roomY;
	bool hasDash, hasKey;
	int deathCount;
	uint32_t check;
};

struct GameState : SaveState {
	Entity entities[MAX_ENTITIES];
	int world;
	Entity *player;
};

GameState state;

Entity *AddEntity(Entity *templateEntity, float x, float y, int uid = 0) {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (!state.entities[i].isUsed) {
			memcpy(state.entities + i, templateEntity, sizeof(Entity));
			state.entities[i].isUsed = true;
			if (!state.entities[i].frameCount) state.entities[i].frameCount = 1;
			if (!state.entities[i].stepsPerFrame) state.entities[i].stepsPerFrame = 1;
			state.entities[i].x = state.entities[i].px = x;
			state.entities[i].y = state.entities[i].py = y;
			if (!state.entities[i].w) state.entities[i].w = state.entities[i].texture->width - 1;
			if (!state.entities[i].h) state.entities[i].h = state.entities[i].texture->height / state.entities[i].frameCount - 1;
			state.entities[i].uid = uid;
			if (state.entities[i].create) state.entities[i].create(state.entities + i);
			return state.entities + i;
		}
	}
	
	static Entity fake = {};
	return &fake;
}
	
Entity *FindEntity(float x, float y, float w, float h, int tag, Entity *exclude) {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (state.entities[i].isUsed && !state.entities[i].isDestroyed
				&& (state.entities[i].tag == tag 
					|| tag == TAG_ALL 
					|| (tag == TAG_SOLID && state.entities[i].solid)
					|| (tag == TAG_KILL && state.entities[i].kill)
					)
				&& state.entities + i != exclude) {
			if (x <= state.entities[i].x + state.entities[i].w && state.entities[i].x <= x + w 
					&& y <= state.entities[i].y + state.entities[i].h && state.entities[i].y <= y + h) {
				return state.entities + i;
			}
		}
	}
	
	return nullptr;
}

char roomName[16];

void UpdateRoomName() {
	roomName[0] = 'R';
	roomName[1] = (state.roomX / 10) + '0';
	roomName[2] = (state.roomX % 10) + '0';
	roomName[3] = '_';
	roomName[4] = (state.roomY / 10) + '0';
	roomName[5] = (state.roomY % 10) + '0';
	roomName[6] = '.';
	roomName[7] = 'D';
	roomName[8] = 'A';
	roomName[9] = 'T';
	roomName[10] = 0;
}

#define ROOM_ID ((state.roomX - 7) + (state.roomY - 9) * 6)

void LoadRoom() {
	state.world = 0;
	
	// state.visited |= 1ull << ROOM_ID;
	
	UpdateRoomName();
	
#ifdef DEVBUILD
	char n[32];
	memcpy(n, "..\\embed\\", 9);
	memcpy(n + 9, roomName, 11);
	
	char buffer[16384] = {};
	HANDLE h = CreateFile(n, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD w;
	if (h != INVALID_HANDLE_VALUE) {
		ReadFile(h, buffer, sizeof(buffer), &w, 0);
		CloseHandle(h);
	}
#else
	uint8_t *buffer = (uint8_t *) "";
	roomName[6] = '_';
	
	for (int i = 0; embedItems[i].name; i++) {
		if (0 == memcmp(embedItems[i].name, roomName, 11)) {
			buffer = (uint8_t *) embedItems[i].pointer;
			break;
		}
	}
#endif
	
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (!state.entities[i].isUsed || state.entities[i].isDestroyed) continue;
		
		if (state.entities[i].tag != typePlayer.tag) {
			state.entities[i].Destroy();
		} else {
			state.entities[i].px = state.entities[i].x;
			state.entities[i].py = state.entities[i].y;
		}
	}
	
	int p = 0;
	int iir = 0;
	
	while (true) {
		uint8_t tag = buffer[p++];
		if (!tag) break;
		float x = *(float *) (buffer + p); p += 4;
		float y = *(float *) (buffer + p); p += 4;
		
		if (tag == (uint8_t) TAG_WORLD) {
			state.world = x;
		}
		
		for (int i = 0; entityTypes[i]; i++) {
			if (entityTypes[i]->tag == tag) {
				AddEntity(entityTypes[i], x, y, (state.roomX << 24) | (state.roomY << 16) | iir);
				iir++;
			}
		}
	}
	
#ifndef DEVBUILD
	musicIndex = state.world;
#endif
}

void CalculateCheck() {
	state.check = 0;
	
	uint8_t *buffer = (uint8_t *) &state;
	uint32_t check = 0x1D471D47;
	
	for (int i = 0; i < sizeof(SaveState); i++) {
		check ^= ((uint32_t) buffer[i] + 10) * (i + 100);
	}
	
	state.check = check;
}

float FadeInOut(float t) {
	if (t < 0.3f) return t / 0.3f;
	else if (t < 0.7f) return 1;
	else return 1 - (t - 0.7f) / 0.3f;
}

void InitialiseGame() {
	state.roomX = 10;
	state.roomY = 10;
	
	CreateTexture(&textureKey, key_png, key_pngBytes);
	CreateTexture(&textureDashMessage, dash_msg_png, dash_msg_pngBytes);
	CreateTexture(&textureKeyMessage, key_msg_png, key_msg_pngBytes);
	CreateTexture(&textureControlsMessage, controls_png, controls_pngBytes);
	
	int tag = 1;
	
	{
		static Texture texture;
		CreateTexture(&texture, player_png, player_pngBytes);
		typePlayer.tag = tag++;
		typePlayer.texture = &texture;
		typePlayer.frameCount = 6;
		typePlayer.stepsPerFrame = 5;
		typePlayer.layer = 1;
		
		typePlayer.step = [] (Entity *entity) {
			if (entity->player.respawn) {
				if (entity->stepIndex > entity->player.respawn) {
					entity->player.respawn = 0;
					entity->hide = false;
					entity->player.cx = state.checkX;
					entity->player.cy = state.checkY;
					entity->player.respawnGrow = entity->stepIndex + 20;
					entity->player.dash = 0;
				} else {
					return;
				}
			}
			
			if (!entity->player.respawnGrow && !entity->player.dash) {
				if (keysPressed[VK_SPACE] || keysPressed['Z']) {
					entity->player.cx += (entity->x - entity->player.cx) * 2;
					entity->player.cy += (entity->y - entity->player.cy) * 2;
					entity->player.th += 3.15f;
					entity->player.dth = -entity->player.dth;
					entity->player.dths = 5;
				} else if (keysPressed['X'] && state.hasDash) {
					entity->player.dash = 10;
				}
			}
			
			float rd = 40;
			
			if (entity->player.respawnGrow) {
				if (entity->stepIndex > entity->player.respawnGrow) {
					entity->player.respawnGrow = 0;
				} else {
					rd *= 1 - (entity->player.respawnGrow - entity->stepIndex) / 20.0f;
				}
			}
			
			entity->x = rd * cosf(entity->player.th) + entity->player.cx;
			entity->y = rd * sinf(entity->player.th) + entity->player.cy + 5 * sinf(4.71f + entity->stepIndex * 0.1f);
			
			if (entity->player.dash) {
				float pt = entity->player.th - entity->player.dth;
				float px = rd * cosf(pt) + entity->player.cx;
				float py = rd * sinf(pt) + entity->player.cy + 5 * sinf(4.71f + (entity->stepIndex - 1) * 0.1f);
				float dx = entity->x - px;
				float dy = entity->y - py;
				entity->player.cx += dx * 3.2f;
				entity->player.cy += dy * 3.2f;
				entity->player.dash--;
				AddEntity(&typeStar, entity->x + 8, entity->y + 8)->star.col = 0xFF;
			} else {
				entity->player.th += entity->player.dth;
			}
			
			if (entity->player.respawnGrow) {
				entity->px = entity->x;
				entity->py = entity->y;
			}
			
			if (entity->player.dths) {
				entity->player.th += entity->player.dth;
				entity->player.dths--;
				entity->stepIndex = 5 * 3;
			}
			
			if (FindEntity(entity->x + 5, entity->y + 5, entity->w - 10, entity->h - 10, TAG_KILL, 0)) {
				for (int i = 0; i < 20; i++) {
					AddEntity(&typeStar, entity->x + 8, entity->y + 8)->star.col = 0xFF0000;
				}
				
				entity->hide = true;
				entity->player.respawn = entity->stepIndex + 20;
				state.deathCount++;
			}
			
			if (entity->x > GAME_SIZE - 8) {
				entity->x -= GAME_SIZE;
				entity->player.cx -= GAME_SIZE;
				state.checkX -= GAME_SIZE;
				state.roomX++;
				LoadRoom();
			} else if (entity->x < -8) {
				entity->x += GAME_SIZE;
				entity->player.cx += GAME_SIZE;
				state.checkX += GAME_SIZE;
				state.roomX--;
				LoadRoom();
			}
			
			if (entity->y > GAME_SIZE - 8) {
				entity->y -= GAME_SIZE;
				entity->player.cy -= GAME_SIZE;
				state.checkY -= GAME_SIZE;
				state.roomY++;
				LoadRoom();
			} else if (entity->y < -8) {
				entity->y += GAME_SIZE;
				entity->player.cy += GAME_SIZE;
				state.checkY += GAME_SIZE;
				state.roomY--;
				LoadRoom();
			}
		};
		
		typePlayer.create = [] (Entity *entity) { 
			state.player = entity; 
			entity->player.cx = entity->x;
			entity->player.cy = entity->y;
			entity->player.dth = 0.06f;
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, block_png, block_pngBytes);
		
		static Texture block2;
		CreateTexture(&block2, block2_png, block2_pngBytes);
		
		typeBlock.tag = tag++;
		typeBlock.texture = &texture;
		typeBlock.kill = true;
		typeBlock.hide = true;
		typeBlock.layer = 2;
		
		typeBlock.create = [] (Entity *entity) {
			uint8_t r = GetRandomByte();
			
			if (r < 20) {
				entity->block.random = 1;
			} else if (r < 50) {
				entity->block.random = 2;
			} else {
				entity->block.random = 0;
			}
		};
		
		typeBlock.stepAlways = [] (Entity *entity) {
			if (FindEntity(entity->x + 1, entity->y + 1, entity->w - 2, entity->h - 2, entity->tag, entity)) {
				entity->Destroy();
			}
		};
		
		typeBlock.drawAfter = [] (Entity *entity) {
			if (!FindEntity(entity->x - 16, entity->y, 1, 1, entity->tag, entity)) {
				Draw(&block2, entity->x - 16, entity->y, 16, 16, 0, entity->block.random * 16, 16, 16, 1);
			}
			
			if (!FindEntity(entity->x + 16, entity->y, 1, 1, entity->tag, entity)) {
				Draw(&block2, entity->x + 16, entity->y, 16, 16, 32, entity->block.random * 16, 16, 16, 1);
			}
			
			if (!FindEntity(entity->x, entity->y - 16, 1, 1, entity->tag, entity)) {
				Draw(&block2, entity->x, entity->y - 16, 16, 16, 16, entity->block.random * 16, 16, 16, 1);
			}
			
			if (!FindEntity(entity->x, entity->y + 16, 1, 1, entity->tag, entity)) {
				Draw(&block2, entity->x, entity->y + 16, 16, 16, 48, entity->block.random * 16, 16, 16, 1);
			}
		};
	}
	
	{
		static Texture check1, check2;
		CreateTexture(&check1, check1_png, check1_pngBytes);
		CreateTexture(&check2, check2_png, check2_pngBytes);
		typeCheck.tag = tag++;
		typeCheck.texture = &check1;
		
		typeCheck.step = [] (Entity *entity) {
			if (state.checkX == entity->x && state.checkY == entity->y) {
				entity->texture = &check2;
			} else {
				entity->texture = &check1;
			}
			
			if (FindEntity(entity->x - 4, entity->y - 4, entity->w + 8, entity->h + 8, typePlayer.tag, 0)) {
				if (state.checkX != entity->x || state.checkY != entity->y) {
					for (int i = 0; i < 10; i++) AddEntity(&typeStar, entity->x + 8, entity->y + 8)->star.col = 0xFFFFFF;
				}
				
				state.checkX = entity->x;
				state.checkY = entity->y;
				
				wchar_t str[4096];
				StringCopy(str, appData);
				StringAppend(str, L"\\" L"__" BRANDW L"_save.dat");
				HANDLE h = CreateFileW(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, 0);
				DWORD w;
				CalculateCheck();
				WriteFile(h, &state, sizeof(SaveState), &w, 0);
				CloseHandle(h);
			}
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, moveblock_png, moveblock_pngBytes);
		typeMoveH.tag = tag++;
		typeMoveH.texture = &texture;
		typeMoveH.kill = true;
		typeMoveH.w = 16;
		typeMoveH.h = 16;
		typeMoveH.texOffX = -4;
		typeMoveH.texOffY = -4;
		
		typeMoveH.create = [] (Entity *entity) {
			entity->moveBlock.vel = -4;
		};
		
		typeMoveH.step = [] (Entity *entity) {
			entity->x += entity->moveBlock.vel;
			
			if (FindEntity(entity->x, entity->y, entity->w, entity->h, typeBlock.tag, 0)) {
				entity->moveBlock.vel = -entity->moveBlock.vel;
			}
		};
	}
	
	{
		// Removed entity.
		tag++;
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, star_png, star_pngBytes);
		typeStar.texture = &texture;
		typeStar.tag = tag++;
		typeStar.hide = true; // Draw manually.
		typeStar.layer = 2;
		
		typeStar.create = [] (Entity *entity) {
			float th = GetRandomByte() / 255.0 * 6.24;
			float sp = GetRandomByte() / 255.0 * 0.5 + 0.5;
			entity->star.vx = sp * cosf(th);
			entity->star.vy = sp * sinf(th);
			entity->star.life = GetRandomByte();
			entity->star.dth = (GetRandomByte() / 255.0f - 0.5f) * 0.2f;
		};
		
		typeStar.step = [] (Entity *entity) {
			entity->x += entity->star.vx;
			entity->y += entity->star.vy;
			entity->star.th += entity->star.dth;
			
			if (entity->star.life < entity->stepIndex) {
				entity->Destroy();
			}
		};
		
		typeStar.drawAfter = [] (Entity *entity) {
			Draw(entity->texture, entity->x - 4, entity->y - 4, -1, -1, 0, 0, -1, -1, 
				1.0 - (float) entity->stepIndex / entity->star.life, 
				((entity->star.col >> 16) & 0xFF) / 255.0f,
				((entity->star.col >> 8) & 0xFF) / 255.0f,
				((entity->star.col >> 0) & 0xFF) / 255.0f,
				entity->star.th);
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, moveblock_png, moveblock_pngBytes);
		typeMoveV.tag = tag++;
		typeMoveV.texture = &texture;
		typeMoveV.kill = true;
		typeMoveV.w = 16;
		typeMoveV.h = 16;
		typeMoveV.texOffX = -4;
		typeMoveV.texOffY = -4;
		
		typeMoveV.create = [] (Entity *entity) {
			entity->moveBlock.vel = -4;
		};
		
		typeMoveV.step = [] (Entity *entity) {
			entity->y += entity->moveBlock.vel;
			
			if (FindEntity(entity->x, entity->y, entity->w, entity->h, typeBlock.tag, 0)) {
				entity->moveBlock.vel = -entity->moveBlock.vel;
			}
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, powerup_png, powerup_pngBytes);
		typePowerup.tag = tag++;
		typePowerup.texture = &texture;
		
		typePowerup.step = [] (Entity *entity) {
			if (state.hasDash) {
				entity->Destroy();
				return;
			}
			
			if (FindEntity(entity->x, entity->y, entity->w, entity->h, typePlayer.tag, 0)) {
				state.hasDash = true;
				AddEntity(&typeShowMessage, 0, 0)->texture = &textureDashMessage;
				entity->Destroy();
			}
		};
	}
	
	{
		typeShowMessage.texture = &textureKeyMessage;
		typeShowMessage.tag = tag++;
		typeShowMessage.hide = true;
		
		typeShowMessage.draw = [] (Entity *entity) {
			Draw(entity->texture, entity->x, entity->y, -1, -1, 0, 0, -1, -1, FadeInOut(entity->stepIndex / 180.0));
			if (entity->stepIndex > 180) entity->Destroy();
		};
	}
	
	{
		typeKey.tag = tag++;
		typeKey.texture = &textureKey;
		
		typeKey.step = [] (Entity *entity) {
			if (state.hasKey) {
				entity->Destroy();
			} else if (FindEntity(entity->x, entity->y, entity->w, entity->h, typePlayer.tag, 0)) {
				state.hasKey = true;
				AddEntity(&typeShowMessage, 0, 0)->texture = &textureKeyMessage;
				entity->Destroy();
				for (int i = 0; i < 10; i++) AddEntity(&typeStar, entity->x + 8, entity->y + 8)->star.col = 0xFFFFFF;
			}
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, lock_png, lock_pngBytes);
		typeLock.tag = tag++;
		typeLock.texture = &texture;
		typeLock.kill = true;
		
		typeLock.step = [] (Entity *entity) {
			if (state.hasKey) {
				for (int i = 0; i < 1; i++) AddEntity(&typeStar, entity->x + 8, entity->y + 8)->star.col = 0x000000;
				entity->Destroy();
			}
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, moveblock_png, moveblock_pngBytes);
		typeSpin.tag = tag++;
		typeSpin.texture = &texture;
		typeSpin.kill = true;
		typeSpin.w = 16;
		typeSpin.h = 16;
		typeSpin.texOffX = -4;
		typeSpin.texOffY = -4;
		
		typeSpin.create = [] (Entity *entity) {
			entity->spin.cx = entity->x;
			entity->spin.cy = entity->y;
		};
		
		typeSpin.step = [] (Entity *entity) {
			entity->x = 60 * cosf(entity->spin.th) + entity->spin.cx;
			entity->y = 60 * sinf(entity->spin.th) + entity->spin.cy;
			entity->spin.th += 0.04f;
		};
		
		typeSpin.stepAlways = [] (Entity *entity) {
			if (levelEditing) {
				entity->x = entity->spin.cx;
				entity->y = entity->spin.cy;
			}
		};
	}
	
	{
		static Texture msg1, msg2, msg3, num;
		CreateTexture(&msg1, end1_png, end1_pngBytes);
		CreateTexture(&msg2, end2_png, end2_pngBytes);
		CreateTexture(&msg3, end3_png, end3_pngBytes);
		CreateTexture(&num, numbers_png, numbers_pngBytes);
		
		typeEnd.tag = tag++;
		typeEnd.texture = &textureKey;
		typeEnd.hide = true;
		
		typeEnd.create = [] (Entity *entity) {
			state.player->Destroy();
		};
		
		typeEnd.draw = [] (Entity *entity) {
			float t = entity->stepIndex / 180.0f;
			
			if (t < 1) {
				Draw(&msg1, 40, 150, -1, -1, 0, 0, -1, -1, FadeInOut(t));
			} else if (t < 2) {
				Draw(&msg2, 40, 150, -1, -1, 0, 0, -1, -1, FadeInOut(t - 1));
			} else if (t < 3) {
				Draw(&msg3, 40, 150, -1, -1, 0, 0, -1, -1, FadeInOut(t - 2));
				
				int p = state.deathCount;
				char digits[10];
				int dc = 0;
				
				if (p == 0) {
					digits[dc++] = 0;
				} else {
					while (p) {
						digits[dc++] = p % 10;
						p /= 10;
					}
				}
				
				int w = dc * 16;
				
				for (int i = dc - 1; i >= 0; i--) {
					Draw(&num, 40 + 150 - w / 2 + (dc - 1 - i) * 16, 
						150 + 33, 16, 30, 16 * digits[i], 0, 16, 30, FadeInOut(t - 2));
				}
			} else {
				DoExit(0);
			}
		};
	}
	
	state.checkX = GAME_SIZE / 2;
	state.checkY = GAME_SIZE / 2 - 20;

#ifndef DEVBUILD
	wchar_t str[4096];
	StringCopy(str, appData);
	StringAppend(str, L"\\" L"__" BRANDW L"_save.dat");
	HANDLE h = CreateFileW(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN, 0);
	bool noSave = true;
	
	if (h != INVALID_HANDLE_VALUE) {
		DWORD w;
		ReadFile(h, &state, sizeof(SaveState), &w, 0);
		CloseHandle(h);
		
		uint32_t oldCheck = state.check;
		CalculateCheck();
		Assert(oldCheck == state.check);
		
		noSave = false;
	}
#endif
	
	LoadRoom();
	
#ifdef DEVBUILD
	AddEntity(&typePlayer, -100, -100);
#else
	AddEntity(&typePlayer, state.checkX, state.checkY);

	if (noSave) {
		AddEntity(&typeShowMessage, 0, GAME_SIZE - 65)->texture = &textureControlsMessage;
	}
#endif
}

#ifdef DEVBUILD
void Save() {
	UpdateRoomName();
	char buffer[32];
	memcpy(buffer, "..\\embed\\", 9);
	memcpy(buffer + 9, roomName, 11);
	
	HANDLE h = CreateFile(buffer, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD w;
	
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (state.entities[i].isUsed) {
			WriteFile(h, &state.entities[i].tag, 1, &w, 0);
			WriteFile(h, &state.entities[i].x, 4, &w, 0);
			WriteFile(h, &state.entities[i].y, 4, &w, 0);
		}
	}
	
	{
		uint8_t tag = TAG_WORLD;
		float world = state.world;
		WriteFile(h, &tag, 1, &w, 0);
		WriteFile(h, &world, 4, &w, 0);
		WriteFile(h, &world, 4, &w, 0);
	}
	
	uint8_t zero = 0;
	WriteFile(h, &zero, 1, &w, 0);
	
	CloseHandle(h);
}
#endif

void UpdateGame() {	
	if (keysPressed[VK_ESCAPE]) {
		DoExit(0);
	}
	
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (state.entities[i].isUsed) {
			state.entities[i].stepIndex++;
			
			state.entities[i].px += (state.entities[i].x - state.entities[i].px) * 0.5f;
			state.entities[i].py += (state.entities[i].y - state.entities[i].py) * 0.5f;
		}
	}
	
	for (int i = 0; i < MAX_ENTITIES; i++) if (state.entities[i].isUsed && state.entities[i].stepAlways) state.entities[i].stepAlways(state.entities + i);
	
	if (!levelEditing) {
		for (int i = 0; i < MAX_ENTITIES; i++) if (state.entities[i].isUsed && state.entities[i].step) {
			state.entities[i].step(state.entities + i);
		}
	} else {
#ifdef DEVBUILD
		if (keysDown['B']) {
			AddEntity(&typeBlock, mouseX & ~(TILE_SIZE - 1), mouseY & ~(TILE_SIZE - 1));
		} else if (keysDown['E']) {
			Entity *e = FindEntity(mouseX, mouseY, 1, 1, TAG_ALL, 0);
			if (e) e->Destroy();
		} else if (keysPressed['C']) {
			AddEntity(&typeCheck, mouseX & ~(TILE_SIZE - 1), mouseY & ~(TILE_SIZE - 1));
		} else if (keysPressed['1']) {
			AddEntity(&typeMoveH, mouseX & ~(TILE_SIZE - 1), mouseY & ~(TILE_SIZE - 1));
		} else if (keysPressed['2']) {
			AddEntity(&typeMoveV, mouseX & ~(TILE_SIZE - 1), mouseY & ~(TILE_SIZE - 1));
		} else if (keysPressed['3']) {
			AddEntity(&typePowerup, mouseX & ~(TILE_SIZE - 1), mouseY & ~(TILE_SIZE - 1));
		} else if (keysPressed['4']) {
			AddEntity(&typeKey, mouseX & ~(TILE_SIZE - 1), mouseY & ~(TILE_SIZE - 1));
		} else if (keysPressed['5']) {
			AddEntity(&typeLock, mouseX & ~(TILE_SIZE - 1), mouseY & ~(TILE_SIZE - 1));
		} else if (keysPressed['6']) {
			AddEntity(&typeSpin, mouseX & ~(TILE_SIZE - 1), mouseY & ~(TILE_SIZE - 1));
		} else if (keysPressed['7']) {
			AddEntity(&typeEnd, mouseX & ~(TILE_SIZE - 1), mouseY & ~(TILE_SIZE - 1));
		} else if (keysPressed['H']) {
			Save();
			state.roomX--;
			LoadRoom();
		} else if (keysPressed['J']) {
			Save();
			state.roomY++;
			LoadRoom();
		} else if (keysPressed['K']) {
			Save();
			state.roomY--;
			LoadRoom();
		} else if (keysPressed['L']) {
			Save();
			state.roomX++;
			LoadRoom();
		} else if (keysPressed['0']) {
			state.world++;
		}
#endif
	}
	
#ifdef DEVBUILD
	if (keysPressed['P']) {
		levelEditing = !levelEditing;
		
		if (levelEditing) {
			state.player->x = -100;
			state.player->y = -100;
		} else {
			Save();
			state.player->player.cx = mouseX;
			state.player->player.cy = mouseY;
		}
	} else if (keysPressed['S']) {
		Save();
	}
#endif
	
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (state.entities[i].isUsed && state.entities[i].isDestroyed) state.entities[i].isUsed = false;
	}
	
	levelTick++;
	
	state.world %= 3;
	
	if (state.world == 0) {
		backgroundColor = 0xbef1b1;
	} else if (state.world == 1) {
		backgroundColor = 0xcee5f1;
	} else if (state.world == 2) {
		backgroundColor = 0xf3bdf6;
	}
}

void RenderGame() {	
	for (int layer = -1; layer <= 3; layer++) {
		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity *entity = state.entities + i;
			if (!entity->isUsed) continue;
			if (entity->layer != layer) continue;
			if (!entity->texture) continue;
			if (entity->hide) continue;
			
			int frame = entity->stepsPerFrame >= 0 ? ((entity->stepIndex / entity->stepsPerFrame) % entity->frameCount) : (-entity->stepsPerFrame - 1);
			Draw(entity->texture, (int) (entity->px + entity->texOffX + 0.5f), 
				(int) (entity->py + entity->texOffY + 0.5f),
				entity->texture->width, entity->texture->height / entity->frameCount,
				0, entity->texture->height / entity->frameCount * frame, 
				entity->texture->width, entity->texture->height / entity->frameCount);
		}
	}
	
	for (int i = 0; i < MAX_ENTITIES; i++) if (state.entities[i].isUsed && state.entities[i].draw      ) state.entities[i].draw      (state.entities + i);
	for (int i = 0; i < MAX_ENTITIES; i++) if (state.entities[i].isUsed && state.entities[i].drawAfter ) state.entities[i].drawAfter (state.entities + i);
	
	if (state.hasKey) {
		Draw(&textureKey, GAME_SIZE - 20, 4);
	}
	
#if 0
	for (int j = 0; j < 10; j++) {
		for (int i = 0; i < 6; i++) {
			if (state.visited & (1ull << (i + j * 6))) {
				Draw(&textureWhite, GAME_SIZE - 48 + i * 8, GAME_SIZE - 80 + j * 8, 8, 8, 0, 0, -1, -1, 1, 1, 0, 0);
			}
		}
	}
#endif
}