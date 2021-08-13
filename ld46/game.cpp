#define TAG_ALL (-1)
#define TAG_SOLID (-2)
#define TAG_MOVE_LAYER (-3)
#define TAG_PUSH (-4)

#define TILE_SIZE (8)

struct Entity {
	uint8_t tag;
	bool solid, push, onFloor, above;
	char symbol;
	int8_t frameCount, stepsPerFrame;
	Texture *texture;
	float texOffX, texOffY, w, h;
	
	void (*create)(Entity *);
	void (*destroy)(Entity *);
	void (*stepBefore)(Entity *);
	void (*step)(Entity *);
	void (*stepAfter)(Entity *);
	void (*drawBefore)(Entity *);
	void (*draw)(Entity *);
	void (*drawAfter)(Entity *);
	
	float x, y, px, py;
	int stepIndex;
	bool isUsed, isDestroyed, wasPushed;
	
	union {
		int blockType; // blocks
		bool direction; // bots
		int level; // levels
	};
	
	void Destroy() {
		if (isDestroyed) return;
		isDestroyed = true;
		if (destroy) destroy(this);
	}
};

Texture backgroundTexture, fontTexture, msgboxTexture, blockTexture, smokeTexture;

Entity typePlayer, typeBlock, typeAxe, typeLogs, typeTree, typeFire, typeBlockAlt,
	typeLeft, typeRight, typeUp, typeDown, typeBotH, typeBotV, typeSmoke, typeDoor, typeKey,
	typeLevel;

Entity *entityTypes[] = { 
	&typePlayer, &typeBlock, &typeAxe, &typeLogs, &typeTree, &typeFire, &typeBlockAlt,
	&typeLeft, &typeRight, &typeUp, &typeDown, &typeBotH, &typeBotV, &typeSmoke, &typeDoor, &typeKey,
	&typeLevel,
};
	
int loadLevelIndices = 1;

uint8_t *soundEffects[] = {
#define SFX_MOVE (0)
	move_wav,
#define SFX_UNDO (1)
	undo_wav,
#define SFX_ARROW (2)
	arrow_wav,
#define SFX_CHOP (3)
	chop_wav,
#define SFX_UNLOCK (4)
	unlock_wav,
#define SFX_STOKE (5)
	stoke_wav,
#define SFX_LOSE (6)
	lose_wav,
#define SFX_WIN (7)
	win_wav,
};

int triggerSFX = -1;
int levelTick;
bool justLoaded = true;
int currentBGM;

#define OVERWORLD (2)

#if 0
const uint8_t *levels[] = { l4_4_txt };
#else
const uint8_t *levels[] = { lintro_txt, l1_1_txt, overworld_txt,
	l1_2_txt, l1_3_txt, l1_4_txt, 
	l2_1_txt, l2_2_txt, l2_3_txt, l2_4_txt,
	l3_1_txt, l3_2_txt, l3_3_txt, l3_4_txt, 
	l4_1_txt, l4_2_txt, l4_3_txt, l4_4_txt,
	l5_1_txt, l5_2_txt, l5_3_txt, lend_txt,
};
#endif

int levelPalettes[] = { 0, 0, 0,
	0, 0, 0, 
	1, 1, 1, 1, 
	2, 2, 2, 2,
	3, 3, 3, 3,
	4, 4, 4, 4, 
	0,
};

DWORD completedLevels;

#define MAX_ENTITIES (400)

struct GameState {
	Entity entities[MAX_ENTITIES];
	
	Entity *player;
	int stepsRemaining, logsRemaining, showMessage, messageCount;
	int currentLevel;
};

#define MAX_UNDO (200)
GameState undoStack[MAX_UNDO];
GameState state;
int undoStackIndex;
uint8_t floorRandom[GAME_SIZE / TILE_SIZE][GAME_SIZE / TILE_SIZE];
bool updateStep;

void PlaySFX(int index) {
	if (index > triggerSFX) {
		triggerSFX = index;
	}
}

Entity *AddEntity(Entity *templateEntity, float x, float y) {
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
			if (state.entities[i].create) state.entities[i].create(state.entities + i);
			return state.entities + i;
		}
	}
	
	Assert(false);
	return nullptr;
}
	
Entity *FindEntity(float x, float y, float w, float h, int tag, Entity *exclude) {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (state.entities[i].isUsed 
				&& (state.entities[i].tag == tag 
					|| tag == TAG_ALL 
					|| (tag == TAG_SOLID && state.entities[i].solid) 
					|| (tag == TAG_PUSH && state.entities[i].push)
					|| (tag == TAG_MOVE_LAYER && !state.entities[i].onFloor && !state.entities[i].above)) 
				&& state.entities + i != exclude) {
			if (x <= state.entities[i].x + state.entities[i].w && state.entities[i].x <= x + w 
					&& y <= state.entities[i].y + state.entities[i].h && state.entities[i].y <= y + h) {
				return state.entities + i;
			}
		}
	}
	
	return nullptr;
}

bool Move(Entity *entity, float dx, float dy) {
	Entity *collision = FindEntity(entity->x + dx, entity->y + dy, entity->w, entity->h, TAG_MOVE_LAYER, entity);
	
	if (!collision) {
		entity->x += dx;
		entity->y += dy;
		entity->wasPushed = true;
		return true;
	} else if (collision->tag == typeTree.tag && entity->tag == typeAxe.tag) {
		AddEntity(&typeLogs, collision->x, collision->y);
		collision->Destroy();
		state.logsRemaining--;
		PlaySFX(SFX_CHOP);
		return false;
	} else if (collision->tag == typeFire.tag && entity->tag == typeLogs.tag) {
		entity->Destroy();
		state.logsRemaining--;
		PlaySFX(SFX_STOKE);
		return true;
	} else if (collision->tag == typeDoor.tag && entity->tag == typeKey.tag) {
		entity->Destroy();
		collision->Destroy();
		PlaySFX(SFX_UNLOCK);
		return true;
	} else if (collision->push) {
		if (Move(collision, dx, dy)) {
			entity->x += dx;
			entity->y += dy;
			entity->wasPushed = true;
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

void LoadLevel(int index) {
	currentPalette = levelPalettes[index];
	loadLevelIndices = 1;
	levelTick = 0;
	
	if (index == sizeof(levels) / sizeof(levels[0])) {
		DoExit(0);
	} else if (OVERWORLD > sizeof(levels) / sizeof(levels[0])) {
		index = 0;
	}
	
	if (index != OVERWORLD) justLoaded = false;
	
	state.currentLevel = index;
	const char *level = (const char *) levels[index];
	state.logsRemaining = 0;
	
	for (int i = 0; i < MAX_ENTITIES; i++) {
		state.entities[i].isUsed = false;
	}
	
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 20; j++) {
			char symbol = level[i + j * 22];
			
			bool flipDirection = symbol == 'c' || symbol == 'C';
			if (flipDirection) symbol--;
			
			for (int k = 0; k < sizeof(entityTypes) / sizeof(entityTypes[0]); k++) {
				if (entityTypes[k]->symbol == symbol) {
					Entity *entity = AddEntity(entityTypes[k], i * TILE_SIZE, j * TILE_SIZE);
					if (flipDirection) entity->direction = true;
				}
			}
		}
	}
	
	state.stepsRemaining = ReadProperty(level, "steps");
	state.showMessage = ReadProperty(level, "msg");
	state.messageCount = ReadProperty(level, "msgc");
	if (state.messageCount == -1) state.messageCount = 1;
	
	int newBGM = ReadProperty(level, "bgm");
	if (newBGM != -1 && newBGM != currentBGM) {
		uint8_t *bgms[] = { bgm1_mid, bgm2_mid, bgm3_mid };
		bgmTrack = bgms[newBGM % 3];
		currentBGM = newBGM;
	}
	
	for (int i = 0; i < GAME_SIZE / TILE_SIZE; i++) {
		for (int j = 2; j < GAME_SIZE / TILE_SIZE; j++) {
			uint8_t b = GetRandomByte();
			floorRandom[i][j] = 0;
			if (b > 240) floorRandom[i][j] = 1;
			if (b > 250) floorRandom[i][j] = 2;
			if (b > 254) floorRandom[i][j] = 3;
		}
	}
}

void InitialiseGame() {
	int tag = 1;
	
	CreateTexture(&backgroundTexture, background_png, background_pngBytes);
	CreateTexture(&fontTexture, font_png, font_pngBytes);
	CreateTexture(&msgboxTexture, msgbox_png, msgbox_pngBytes);
	
	{
		static Texture texture;
		CreateTexture(&texture, player_png, player_pngBytes);
		typePlayer.tag = tag++;
		typePlayer.texture = &texture;
		typePlayer.symbol = '#';
		typePlayer.push = true;
		
		typePlayer.step = [] (Entity *) {
			bool left = KeyPressed(VK_LEFT), right = KeyPressed(VK_RIGHT), up = KeyPressed(VK_UP), down = KeyPressed(VK_DOWN);
			
			if (left || right || up || down) {
				if (undoStackIndex == MAX_UNDO) {
					memmove(undoStack, undoStack + 1, sizeof(GameState) * (MAX_UNDO - 1));
					undoStackIndex--;
				}
				
				Copy(undoStack + undoStackIndex, &state, sizeof(state));
				undoStackIndex++;
				
				if (state.stepsRemaining != 1001) {
					state.stepsRemaining--;
				}
				
				updateStep = true;
				
				PlaySFX(SFX_MOVE);
			}
			
			if (left) {
				Move(state.player, -TILE_SIZE, 0);
			} else if (right) {
				Move(state.player,  TILE_SIZE, 0);
			} else if (up) {
				Move(state.player, 0, -TILE_SIZE);
			} else if (down) {
				Move(state.player, 0,  TILE_SIZE);
			}
		};
		
		typePlayer.create = [] (Entity *entity) { state.player = entity; };
	}
	
	{
		static Texture blockTexture, blockTexture2;
		CreateTexture(&blockTexture, block_png, block_pngBytes);
		CreateTexture(&blockTexture2, block2_png, block2_pngBytes);
		typeBlock.tag = tag++;
		typeBlock.symbol = 'X';
		typeBlock.w = typeBlock.h = 7;
		
		typeBlock.step = [] (Entity *entity) {
			if (!entity->blockType) {
				entity->blockType = 1
					+ (FindEntity(entity->x + TILE_SIZE, entity->y, 1, 1, typeBlock.tag, entity) ? 1 : 0)
					+ (FindEntity(entity->x - TILE_SIZE, entity->y, 1, 1, typeBlock.tag, entity) ? 2 : 0)
					+ (FindEntity(entity->x, entity->y + TILE_SIZE, 1, 1, typeBlock.tag, entity) ? 4 : 0)
					+ (FindEntity(entity->x, entity->y - TILE_SIZE, 1, 1, typeBlock.tag, entity) ? 8 : 0)
					+ (GetRandomByte() > 240 ? 16 : 0);
			}
		};
		
		typeBlock.drawBefore = [] (Entity *entity) {
			int blockType = entity->blockType - 1;
			Draw(blockType & 16 ? &blockTexture2 : &blockTexture, entity->x, entity->y, 8, 8, (blockType & 3) * 8, ((blockType >> 2) & 3) * 8, 8, 8);
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, block3_png, block3_pngBytes);
		typeBlockAlt.tag = tag++;
		typeBlockAlt.symbol = 'x';
		typeBlockAlt.w = typeBlockAlt.h = 7;
		
		typeBlockAlt.step = [] (Entity *entity) {
			if (!entity->blockType) {
				entity->blockType = 1
					+ (FindEntity(entity->x + TILE_SIZE, entity->y, 1, 1, typeBlockAlt.tag, entity) ? 1 : 0)
					+ (FindEntity(entity->x - TILE_SIZE, entity->y, 1, 1, typeBlockAlt.tag, entity) ? 2 : 0)
					+ (FindEntity(entity->x, entity->y + TILE_SIZE, 1, 1, typeBlockAlt.tag, entity) ? 4 : 0)
					+ (FindEntity(entity->x, entity->y - TILE_SIZE, 1, 1, typeBlockAlt.tag, entity) ? 8 : 0)
					+ (GetRandomByte() > 240 ? 16 : 0);
			}
		};
		
		typeBlockAlt.drawBefore = [] (Entity *entity) {
			int blockType = entity->blockType - 1;
			Draw(&texture, entity->x, entity->y, 8, 8, (blockType & 3) * 8, ((blockType >> 2) & 3) * 8, 8, 8);
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, axe_png, axe_pngBytes);
		typeAxe.tag = tag++;
		typeAxe.texture = &texture;
		typeAxe.push = true;
		typeAxe.symbol = 'a';
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, logs_png, logs_pngBytes);
		typeLogs.tag = tag++;
		typeLogs.texture = &texture;
		typeLogs.push = true;
		typeLogs.symbol = 'l';
		
		typeLogs.create = [] (Entity *) {
			state.logsRemaining++;
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, tree_png, tree_pngBytes);
		typeTree.tag = tag++;
		typeTree.texture = &texture;
		typeTree.symbol = 't';
		
		typeTree.create = [] (Entity *) {
			state.logsRemaining++;
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, fire_png, fire_pngBytes);
		typeFire.tag = tag++;
		typeFire.texture = &texture;
		typeFire.frameCount = 4;
		typeFire.stepsPerFrame = 10;
		typeFire.symbol = 'F';
		
		typeFire.step = [] (Entity *entity) {
			if (0 == (entity->stepIndex % 60) && GetRandomByte() < 128) {
				AddEntity(&typeSmoke, entity->x + (GetRandomByte() & 7), entity->y);
			}
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, up_png, up_pngBytes);
		typeUp.tag = tag++;
		typeUp.texture = &texture;
		typeUp.symbol = '^';
		typeUp.onFloor = true;
		
		typeUp.stepAfter = [] (Entity *entity) {
			Entity *collision = FindEntity(entity->x, entity->y, entity->w, entity->h, TAG_PUSH, entity);
			if (collision) if (Move(collision, 0, -TILE_SIZE)) PlaySFX(SFX_ARROW);
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, down_png, down_pngBytes);
		typeDown.tag = tag++;
		typeDown.texture = &texture;
		typeDown.symbol = 'v';
		typeDown.onFloor = true;
		
		typeDown.stepAfter = [] (Entity *entity) {
			Entity *collision = FindEntity(entity->x, entity->y, entity->w, entity->h, TAG_PUSH, entity);
			if (collision) if (Move(collision, 0, TILE_SIZE)) PlaySFX(SFX_ARROW);
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, left_png, left_pngBytes);
		typeLeft.tag = tag++;
		typeLeft.texture = &texture;
		typeLeft.symbol = '<';
		typeLeft.onFloor = true;
		
		typeLeft.stepAfter = [] (Entity *entity) {
			Entity *collision = FindEntity(entity->x, entity->y, entity->w, entity->h, TAG_PUSH, entity);
			if (collision) if (Move(collision, -TILE_SIZE, 0)) PlaySFX(SFX_ARROW);
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, right_png, right_pngBytes);
		typeRight.tag = tag++;
		typeRight.texture = &texture;
		typeRight.symbol = '>';
		typeRight.onFloor = true;
		
		typeRight.stepAfter = [] (Entity *entity) {
			Entity *collision = FindEntity(entity->x, entity->y, entity->w, entity->h, TAG_PUSH, entity);
			if (collision) if (Move(collision, TILE_SIZE, 0)) PlaySFX(SFX_ARROW);
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, bot_png, bot_pngBytes);
		typeBotH.tag = tag++;
		typeBotH.texture = &texture;
		typeBotH.symbol = 'b';
		typeBotH.push = true;
		typeBotH.frameCount = 4;
		typeBotH.stepsPerFrame = -3;
		
		typeBotH.stepAfter = [] (Entity *entity) {
			if (updateStep && !entity->wasPushed) {
				if (entity->direction) {
					if (!Move(entity, TILE_SIZE, 0)) {
						entity->direction = false;
					}
				} else {
					if (!Move(entity, -TILE_SIZE, 0)) {
						entity->direction = true;
					}
				}
			}
			
			entity->stepsPerFrame = entity->direction ? -4 : -3;
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, bot_png, bot_pngBytes);
		typeBotV.tag = tag++;
		typeBotV.texture = &texture;
		typeBotV.symbol = 'B';
		typeBotV.push = true;
		typeBotV.frameCount = 4;
		typeBotV.stepsPerFrame = -1;
		
		typeBotV.stepAfter = [] (Entity *entity) {
			if (updateStep && !entity->wasPushed) {
				if (entity->direction) {
					if (!Move(entity, 0, TILE_SIZE)) {
						entity->direction = false;
					}
				} else {
					if (!Move(entity, 0, -TILE_SIZE)) {
						entity->direction = true;
					}
				}
			}
			
			entity->stepsPerFrame = entity->direction ? -2 : -1;
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, smoke_png, smoke_pngBytes);
		typeSmoke.tag = tag++;
		typeSmoke.texture = &texture;
		typeSmoke.above = true;
		
		typeSmoke.stepAfter = [] (Entity *entity) {
			if (!(entity->stepIndex % 10)) {
				entity->y -= 1;
				entity->x += GetRandomByte() > 128 ? 1 : -1;
			}
			
			if (entity->stepIndex > 100 || GetRandomByte() < 1) {
				entity->Destroy();
			}
		};
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, key_png, key_pngBytes);
		typeKey.tag = tag++;
		typeKey.texture = &texture;
		typeKey.symbol = 'k';
		typeKey.push = true;
	}
	
	{
		static Texture texture;
		CreateTexture(&texture, door_png, door_pngBytes);
		typeDoor.tag = tag++;
		typeDoor.texture = &texture;
		typeDoor.symbol = 'd';
	}
	
	{
		static Texture texture, completed, locked;
		CreateTexture(&texture, level_unlocked_png, level_unlocked_pngBytes);
		CreateTexture(&completed, level_completed_png, level_completed_pngBytes);
		CreateTexture(&locked, level_locked_png, level_locked_pngBytes);
		typeLevel.tag = tag++;
		typeLevel.texture = &texture;
		typeLevel.symbol = 'L';
		typeLevel.frameCount = 4;
		typeLevel.stepsPerFrame = 10;
		
		typeLevel.create = [] (Entity *entity) {
			entity->level = loadLevelIndices++;
			if (loadLevelIndices == OVERWORLD) loadLevelIndices++;
			entity->onFloor = true;
			
			if (completedLevels & (1 << entity->level)) {
				entity->texture = &completed;
				entity->frameCount = 1;
			} else {
				int requirementLevel = entity->level - 1;
				if ((entity->level & 3) == 2) requirementLevel--;
				if (entity->level == 3) requirementLevel--;
				int mask = 1 << requirementLevel;
				if (entity->level == 21) mask = 0x1FFFFA;
				
				if ((completedLevels & mask) != mask) {
					entity->texture = &locked;
					entity->frameCount = 1;
					entity->onFloor = false;
				}
			}
		};
		
		typeLevel.stepAfter = [] (Entity *entity) {
			if ((entity->texture == &texture || entity->texture == &completed) && FindEntity(entity->x, entity->y, 1, 1, typePlayer.tag, entity)) {
				updateStep = false;
				LoadLevel(entity->level);
				undoStackIndex = 0;
			}
		};
	}
	
	{
		HANDLE file = CreateFile("Stoke Save File.dat", GENERIC_READ, 0, 0, OPEN_EXISTING, 0x80, 0);
		
		if (file != INVALID_HANDLE_VALUE) {
			DWORD bytes;
			ReadFile(file, &completedLevels, 4, &bytes, 0);
			CloseHandle(file);
		}
	}
	
	LoadLevel((completedLevels & 2) ? OVERWORLD : 0);
}

void UpdateGame() {
	if (KeyPressed(VK_ESCAPE)) {
		if (state.currentLevel <= OVERWORLD && (state.currentLevel != 1 || (~completedLevels & 2))) {
			DoExit(0);
		} else {
			LoadLevel(OVERWORLD);
			undoStackIndex = 0;
			levelTick = 1000;
		}
	}
	
	for (int i = 0; i < MAX_ENTITIES; i++) if (state.entities[i].isUsed && state.entities[i].stepBefore ) state.entities[i].stepBefore(state.entities + i);
	
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (state.entities[i].isUsed) {
			state.entities[i].stepIndex++;
			
			state.entities[i].px += (state.entities[i].x - state.entities[i].px) * 0.5f;
			state.entities[i].py += (state.entities[i].y - state.entities[i].py) * 0.5f;
		}
	}
	
	for (int i = 0; i < MAX_ENTITIES; i++) if (state.entities[i].isUsed && state.entities[i].step       ) state.entities[i].step      (state.entities + i);
	for (int i = 0; i < MAX_ENTITIES; i++) if (state.entities[i].isUsed && state.entities[i].stepAfter  ) state.entities[i].stepAfter (state.entities + i);
	
	updateStep = false;
	
	if (state.showMessage != -1 || state.stepsRemaining == 1001) {}
	else if (!state.logsRemaining && state.player) PlaySFX(SFX_WIN);
	else if (!state.stepsRemaining && state.player) PlaySFX(SFX_LOSE);
	
	if (KeyPressed(VK_F10) && KeyPressed('N')) {
		LoadLevel(++state.currentLevel);
	} else if (KeyPressed(VK_F11) && KeyPressed('N')) {
		LoadLevel(sizeof(levels) / sizeof(levels[0]) - 2);
	} else if (state.stepsRemaining == 1001) {
	} else if (state.showMessage != -1 || !state.logsRemaining || !state.stepsRemaining) {
		if (state.player) { state.player->Destroy(); state.player = nullptr; }
		
		if (KeyPressed('X')) { 
			if (state.showMessage != -1) {
				if (state.messageCount == 1000) {
					DoExit(0);
				} else if (state.messageCount == 1) {
					LoadLevel(++state.currentLevel); 
					undoStackIndex = 0;
				} else {
					state.showMessage++;
					state.messageCount--;
				}
			} else if (!state.logsRemaining) {
				completedLevels |= 1 << state.currentLevel;
				
				HANDLE file = CreateFile("Stoke Save File.dat", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0x80, 0);
		
				if (file != INVALID_HANDLE_VALUE) {
					DWORD bytes;
					WriteFile(file, &completedLevels, 4, &bytes, 0);
					CloseHandle(file);
				}
				
				LoadLevel(OVERWORLD);
				undoStackIndex = 0;
			} else {
				LoadLevel(state.currentLevel); 
			}
		} else if (KeyPressed('U') && undoStack) {
			if (state.logsRemaining && state.showMessage == -1) {
				--undoStackIndex;
				Copy(&state, undoStack + undoStackIndex, sizeof(GameState));
			}
		}
	} else if (KeyPressed('R')) {
		LoadLevel(state.currentLevel);
	} else if (KeyPressed('U') && undoStackIndex) {
		--undoStackIndex;
		Copy(&state, undoStack + undoStackIndex, sizeof(GameState));
		PlaySFX(SFX_UNDO);
	}
	
	for (int i = 0; i < MAX_ENTITIES; i++) {
		state.entities[i].wasPushed = false;
		if (state.entities[i].isUsed && state.entities[i].isDestroyed) state.entities[i].isUsed = false;
	}
	
	if (triggerSFX != -1) {
		PlaySFX(soundEffects[triggerSFX]);
		triggerSFX = -1;
	}
	
	levelTick++;
}

void String(const char *string, int x, int y) {
	int sx = x;
	for (char c; (c = *string); string++) {
		int index = 0;
		if (c >= 'a' && c <= 'z') index = c - 'a';
		else if (c >= 'A' && c <= 'Z') index = c - 'A';
		else if (c == ',') index = 26;
		else if (c == '!') index = 27;
		else if (c == ' ') { x += TILE_SIZE; continue; }
		else if (c == '\n') { x = sx, y += TILE_SIZE; continue; }
		else if (c >= '0' && c <= '9') index = 28 + c - '0';
		else if (c == ':') index = 38;
		else if (c == '.') index = 39;
		Draw(&fontTexture, x, y, TILE_SIZE, TILE_SIZE, 0, index * TILE_SIZE, TILE_SIZE, TILE_SIZE);
		x += TILE_SIZE;
	}
}

void WriteNumber(int n, int x, int y) {
	char b[3] = {};
	b[0] = n / 10 + '0';
	b[1] = n % 10 + '0';
	String(b, x, y);
}

void RenderGame() {
	for (int i = 0; i < GAME_SIZE / TILE_SIZE; i++) {
		for (int j = 0; j < GAME_SIZE / TILE_SIZE; j++) {
			Draw(&backgroundTexture, i * TILE_SIZE, j * TILE_SIZE, TILE_SIZE, TILE_SIZE, 0, TILE_SIZE * floorRandom[i][j], TILE_SIZE, TILE_SIZE);
		}
	}
	
	for (int i = 0; i < MAX_ENTITIES; i++) if (state.entities[i].isUsed && state.entities[i].drawBefore) state.entities[i].drawBefore(state.entities + i);
	
	for (int layer = 0; layer < 3; layer++) {
		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity *entity = state.entities + i;
			if (!entity->isUsed) continue;
			int l = entity->onFloor ? 0 : entity->above ? 2 : 1;
			if (l != layer) continue;
			if (!entity->texture) continue;
			
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
	
	if (state.stepsRemaining == 1001) {
		String((levelTick % 30) < 20 && levelTick < 120 && !justLoaded ? "saved!\n    pick a level!" : "\n    pick a level!", 0, 0);
	} else if (state.showMessage == -1) {
		String("   time left:\n   logs left:", 0, 0);
		WriteNumber(state.stepsRemaining > 500 ? 1000 - state.stepsRemaining : state.stepsRemaining, 14 * TILE_SIZE, 0 * TILE_SIZE);
		WriteNumber(state.logsRemaining, 14 * TILE_SIZE, 1 * TILE_SIZE);
	}
	
	if (state.stepsRemaining == 1001) {
	} else if (state.showMessage != -1) {
		const char *messages[] = {
			"find logs to put\n  on the fire,\nbefore the flame\n   dies out!\n\n  X      Next",
			"arrow keys: move\nR: restart level\nU:     undo step\n\n\n  X      Next",
			"    the end!\n   nice work!\n\n game by nakst\n  made with c\n   and opengl"
		};
		
		Draw(&msgboxTexture, 0, 5 * TILE_SIZE);
		String(messages[state.showMessage], 2 * TILE_SIZE, 7 * TILE_SIZE);
	} else if (!state.logsRemaining) {
		Draw(&msgboxTexture, 0, 5 * TILE_SIZE);
		String(" nice! All the\nlogs were placed\n  on the fire.\n\n\n  X   Continue", 2 * TILE_SIZE, 7 * TILE_SIZE);
	} else if (!state.stepsRemaining) {
		Draw(&msgboxTexture, 0, 5 * TILE_SIZE);
		String("  You took too\nmany steps. The\nfire died out...\n\n\n  X      Retry", 2 * TILE_SIZE, 7 * TILE_SIZE);
	}
}