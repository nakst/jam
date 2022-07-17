// TODO Last puzzle. Ideas: using a crate for leverage on ice; stepping off a button and onto a door for one-way.

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

#define MSG_STEP (1)
#define MSG_DRAW (2)

#define DRAW_STRING(_x, _y, _s) { int x = _x, y = _y; const char *s = _s; for (int i = 0; s[i]; i++) { DrawGlyph(&x, &y, s[i], ++j); } }

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
		struct { bool pushing, justPushed; } push; 
	};
} Entity;

#define MAX_ENTITIES (500)
Entity entities[MAX_ENTITIES];

Entity templateStar;

#define MAX_UNDO (10000)
int undoStack[MAX_UNDO];
int undoPosition;
bool newAction;
bool postUndo;

Entity *player, *controller;
int dice[6]; // inner, outer, left, right, top, bottom

Texture tileset, font, textureStar;
Texture diceFaces[7];

#define MOVEMENT_TIME (12)
int movementTick;
int movementX, movementY;
bool movementSlide;
int movementQX, movementQY;
int wonTick;
int levelNameTick = 1;

int parMovesLeft;

bool menuOpen;
int currentLevel;
uint8_t parTimesAchieved;
uint8_t levelsBeaten;

int menuIndex;
int menuTick;

int fontOffsetX[50];
int fontOffsetY[50];

void Load();

const char *levels[][16] = {
#if 0
	{
		"/////...............",
		"/////...............",
		"/////...............",
		"/////###########....",
		"/////#         #....",
		".....#    c c  #....",
		".....#p  iiiii #....",
		".....###########....",
		"....................",
		"....................",
		"....................",
		"....................",
		"....................",
		"level 0\ndevelopers hideout",
		"",
		"10",
	},
#endif

	{
		"/////...............",
		"/////...............",
		"/////...#########...",
		"/////...#       #...",
		"/////...# ##### #...",
		".....#### #...# #...",
		".....#    #...# #...",
		".....#3####...# #...",
		".....# ######## #...",
		".#####  4 g 2   #...",
		".#p    #######  #...",
		".#######.....####...",
		"....................",
		"level 1\nthe journey begins",
		"arrow keys to move  ",
		"35",
	},

	{
		"/////...............",
		"/////...............",
		"/////...............",
		"/////...............",
		"/////...#####.......",
		"......###   ###.....",
		"......#p    6g#.....",
		"......###   ###.....",
		"........######......",
		"....................",
		"....................",
		"....................",
		"....................",
		"level 2\nroll a six",
		"",
		"08",
	},

	{
		"/////...............",
		"/////..############.",
		"/////..#p       d #.",
		"/////..#  c    ## #.",
		"/////..#     b ##4#.",
		".......#       ## #.",
		".......########## #.",
		"..############### #.",
		"..#g   d         c#.",
		"..#######3##   ##b#.",
		"......###c#########.",
		"......###b#######...",
		"........####........",
		"level 3\noh wow sokoban mechanics",
		"press z to undo  ",
		"43",
	},

	{
		"/////...............",
		"/////...############",
		"/////...#iiiiiiiii##",
		"/////...#i#######i##",
		"/////...#i#...###i##",
		"......###i#...#g i4#",
		"..###.#  2#######ii#",
		"..#p#.#i  i  ####ii#",
		"..# #.######i##  i #",
		"..#i######## ##  i #",
		"..#  iiiiiii #######",
		"..############......",
		"....................",
		"level 4\nice is slippery",
		"press r to restart  ",
		"29",
	},

	{
		"/////..###########..",
		"/////..#        p#..",
		"/////..# # # # # #..",
		"/////..#         #..",
		"/////..# #c# #c# #..",
		".......#         #..",
		".......# # # # # #..",
		".......#         #..",
		"...#####5######dd#..",
		"...#g    #....####..",
		"...#####b#..........",
		".......# #..........",
		".......###..........",
		"level 5\nfield of rocks",
		"",
		"71",
	},

	{
		"/////...............",
		"/////...............",
		"/////...............",
		"/////..#########....",
		"/////..# iiii 1#....",
		".....###ii#iiii#....",
		".....# iii ##ii#....",
		".....#ii##i##ii#....",
		".....#ii##iigii#....",
		".....# i ipii3##....",
		".....##########.....",
		"....................",
		"....................",
		"level 6\narctic maze",
		"",
		"25",
	},

	{
		"/////...............",
		"/////...............",
		"/////....####.......",
		"/////....#  #.......",
		"/////....#g  #......",
		".........#   #......",
		".........# p #......",
		".........#   #......",
		".........#####......",
		"....................",
		"....................",
		"....................",
		"....................",
		"level 7\nit doesnt exist yet",
		"",
		"99",
	},

	{
		"e...................",
		"....................",
		"...###############..",
		"...#             #..",
		"...#             #..",
		"...#             #..",
		"...#      p      #..",
		"...#             #..",
		"...#             #..",
		"...#             #..",
		"...###############..",
		"....................",
		"....................",
		"awesome\nyou beat the game",
		"",
		"99",
	},
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

void Player(Entity *entity, int message) {
	if (message == MSG_DRAW) {
		if (movementTick) {
			float t = ((float) movementTick / MOVEMENT_TIME);
			int mdx = movementX * 16 * t;
			int mdy = movementY * 16 * t;

			if (movementSlide) {
				Blit(diceFaces[dice[1]], entity->x + mdx, entity->y + mdy);
			} else if (movementY == 1) {
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

bool ControllerStep(bool allowSounds, uint8_t *keysHeld) {
	if (levelNameTick != 0) {
		if (levelNameTick == 1) {
			undoPosition = 0;
			Save(((uint32_t) currentLevel << 16) | ((uint32_t) levelsBeaten << 8) | ((uint32_t) parTimesAchieved << 0));
			Load();
		}

		levelNameTick++;

		if (levelNameTick == 180) {
			levelNameTick = 0;
		}
	} else if (wonTick != 0) {
		wonTick++;

		if (wonTick == 45) {
			wonTick = 0;
			levelNameTick = 1;

			for (int i = 0; i < MAX_ENTITIES; i++) {
				if ((entities[i].slot == SLOT_USED) && &entities[i] != controller) {
					EntityDestroy(&entities[i]);
				}
			}
		}
	} else if (movementTick == 0) {
		postUndo = false;

		for (int i = 0; i < MAX_ENTITIES; i++) {
			if ((entities[i].slot == SLOT_USED) && (entities[i].flags & FLAG_PUSH)) {
				entities[i].push.justPushed = false;
			}
		}

		if (keysHeld[KEY_LEFT] || keysHeld['A']) {
			movementTick = 1;
			movementX = -1;
			movementY = 0;
			parMovesLeft--;

			if (undoPosition != MAX_UNDO) {
				undoStack[undoPosition++] = KEY_LEFT;
				newAction = true;
			}
		} else if (keysHeld[KEY_RIGHT] || keysHeld['D']) {
			movementTick = 1;
			movementX = 1;
			movementY = 0;
			parMovesLeft--;

			if (undoPosition != MAX_UNDO) {
				undoStack[undoPosition++] = KEY_RIGHT;
				newAction = true;
			}
		} else if (keysHeld[KEY_UP] || keysHeld['W']) {
			movementTick = 1;
			movementX = 0;
			movementY = -1;
			parMovesLeft--;

			if (undoPosition != MAX_UNDO) {
				undoStack[undoPosition++] = KEY_UP;
				newAction = true;
			}
		} else if (keysHeld[KEY_DOWN] || keysHeld['S']) {
			movementTick = 1;
			movementX = 0;
			movementY = 1;
			parMovesLeft--;

			if (undoPosition != MAX_UNDO) {
				undoStack[undoPosition++] = KEY_DOWN;
				newAction = true;
			}
#if 0
		} else if (movementQX || movementQY) {
			movementTick = 1;
			movementX = movementQX;
			movementY = movementQY;
#endif
		} else if (keysHeld[KEY_BACK] || keysHeld['Z']) {
			if (undoPosition) {
				for (int i = 0; i < MAX_ENTITIES; i++) {
					if ((entities[i].slot == SLOT_USED) && &entities[i] != controller) {
						entities[i].slot = 0;
					}
				}

				Load();
				undoPosition--;
				int t = parMovesLeft - undoPosition;
				movementSlide = false;
				uint8_t kh[256];
				memset(kh, 0, 256);

				for (int i = 0; i < undoPosition; i++) {
					while (!ControllerStep(false, kh));
					kh[undoStack[i]] = true;
					ControllerStep(false, kh);
					kh[undoStack[i]] = false;
					undoPosition--;
				}

				parMovesLeft = t;
				postUndo = true;
			}
		} else if (keysHeld['R']) {
			for (int i = 0; i < MAX_ENTITIES; i++) {
				if ((entities[i].slot == SLOT_USED) && &entities[i] != controller) {
					entities[i].slot = 0;
				}
			}

			Load();
			undoPosition = 0;
		} else if (keysHeld['M']) {
			keysHeld['M'] = 0;
			menuOpen = true;
			menuIndex = currentLevel == 7 ? 0 : currentLevel;
			menuTick = 0;
		}

		return true;
	} else if (movementTick == 1) {
		int newX = player->x + movementX * 16;
		int newY = player->y + movementY * 16;
		bool valid = true;

		while (true) {
			Entity *push = EntityFind(newX, newY, 1, 1, 0, FLAG_PUSH, NULL);

			if (push) {
				if (movementSlide && !push->push.justPushed && !newAction) {
					valid = false;
				} else {
					newX += movementX * 16;
					newY += movementY * 16;
					continue;
				}
			}

			if (EntityFind(newX, newY, 1, 1, 0, FLAG_SOLID, NULL)) {
				valid = false;
			}

			break;
		}
		
		if (!valid) {
			movementTick = 0;
			movementQX = movementQY = 0;
			if (allowSounds) PlaySound("audio/bump_sfx.wav", 18, false, 1.0);

			if (newAction) {
				parMovesLeft++;
				undoPosition--;
			}
		} else {
			movementTick++;

			newX = player->x + movementX * 16;
			newY = player->y + movementY * 16;

			if (EntityFind(newX, newY, 1, 1, TAG_ICE, 0, NULL)) {
				if (allowSounds) PlaySound("audio/slip_sfx.wav", 18, false, 1.0);
			}

			bool pushing = false;

			while (true) {
				Entity *push = EntityFind(newX, newY, 1, 1, 0, FLAG_PUSH, NULL);

				if (push) {
					push->push.pushing = true;
					push->push.justPushed = true;
					pushing = true;
					newX += movementX * 16;
					newY += movementY * 16;
					continue;
				}

				break;
			}

			if (pushing) {
				if (allowSounds) PlaySound("audio/crate_sfx.wav", 19, false, 0.9);
			}
		}

		newAction = false;
	} else if (movementTick == MOVEMENT_TIME) {
		movementTick = 0;

		if (movementSlide) {
			movementSlide = false;
			player->x += movementX * 16;
			player->y += movementY * 16;
		} else if (movementX == -1) {
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
			if ((entities[i].slot == SLOT_USED) && (entities[i].flags & FLAG_PUSH) && entities[i].push.pushing) {
				entities[i].x += movementX * 16;
				entities[i].y += movementY * 16;
				entities[i].push.pushing = false;
			}
		}

		Entity *ice = EntityFind(player->x, player->y, 1, 1, TAG_ICE, 0, NULL);

		if (ice) {
			movementTick = 1;
			movementSlide = true;
		} else {
			Entity *goal = EntityFind(player->x, player->y, 1, 1, TAG_GOAL, 0, NULL);

			if (goal) {
				if (allowSounds) {
					if (parMovesLeft >= 0) {
						PlaySound("audio/win_sfx_par.wav", 21, false, 0.9);
						parTimesAchieved |= 1 << currentLevel;
					} else {
						PlaySound("audio/win_sfx.wav", 17, false, 0.8);
					}

					levelsBeaten |= 1 << currentLevel;
				}

				wonTick = 1;
				currentLevel++;

				for (int i = 0; i < (parMovesLeft >= 0 ? 100 : 20); i++) {
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
				if (allowSounds) PlaySound("audio/reject_sfx.wav", 20, false, 0.6);
			} else {
				if (allowSounds) {
					static int prior = 0;
					int r = 0;
					while (r == prior) r = GetRandomByte() % 3;
					prior = r;
					if (r == 0) PlaySound("audio/dice_roll_1.wav", 21, false, 1.0);
					if (r == 1) PlaySound("audio/dice_roll_2.wav", 21, false, 1.0);
					if (r == 2) PlaySound("audio/dice_roll_3.wav", 21, false, 1.0);
				}
			}

			bool doorsDown = false;
			static bool previousDoorsDown = false;

			for (int i = 0; i < MAX_ENTITIES; i++) {
				if ((entities[i].slot == SLOT_USED) && (entities[i].tag == TAG_BUTTON)) {
					if (EntityFind(entities[i].x, entities[i].y, 1, 1, 0, FLAG_HEAVY, NULL)) {
						doorsDown = !doorsDown;
					}
				}
			}

			if (previousDoorsDown != doorsDown && allowSounds) {
				PlaySound("audio/door_move_sfx.wav", 23, false, 0.6);
			}

			previousDoorsDown = doorsDown;

			for (int i = 0; i < MAX_ENTITIES; i++) {
				if ((entities[i].slot == SLOT_USED) && (entities[i].tag == TAG_DOOR)) {
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

	return false;
}

void DrawGlyph(int *x, int *y, int c, int i) {
	if (c == '\n') {
		*y = *y + 12;
		*x = 0;
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

		Blend(texture, *x + fontOffsetX[i % 50], *y + fontOffsetY[i % 50], 
				wonTick == 0 ? 0xFF : wonTick * 5);
		*x = *x + 8;
	}
}

void Controller(Entity *entity, int message) {
	if (message == MSG_DRAW) {
		if (currentLevel == 7 && levelNameTick == 0 && wonTick == 0) {
		} else if (levelNameTick == 0 && wonTick == 0) {
			int mdx = movementSlide ? 0 : movementX * movementTick * 16 / MOVEMENT_TIME;
			int mdy = movementSlide ? 0 : movementY * movementTick * 16 / MOVEMENT_TIME;

			if (movementTick && !movementSlide) Blit(diceFaces[dice[0]], 2 * 16 + mdx - 2 * 16 * movementX, 2 * 16);
			if (movementTick && !movementSlide) Blit(diceFaces[dice[0]], 2 * 16, 2 * 16 + mdy - 2 * 16 * movementY);
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

			const char *hint = levels[currentLevel][14];
			int x = 0, y = 8;

			for (int i = 0; hint[i]; i++) {
				char c = hint[i];

				if (!x) {
					int lc = 0;

					for (int j = i; hint[j] && hint[j] != '\n'; j++) {
						lc++;
					}

					x = GAME_WIDTH - lc * 8;
				}

				DrawGlyph(&x, &y, c, i);
			}

			char par[7];

			if (parMovesLeft >= 0) {
				par[0] = 'p';
				par[1] = 'a';
				par[2] = 'r';
				par[3] = ' ';
				par[4] = parMovesLeft / 10 + '0';
				par[5] = parMovesLeft % 10 + '0';
				par[6] = 0;
				if (par[4] == '0') { par[4] = par[5]; par[5] = 0; }
			} else if (parMovesLeft >= -3) {
				par[0] = 'n';
				par[1] = 'o';
				par[2] = 'p';
				par[3] = 'e';
				par[4] = 0;
			} else {
				par[0] = 0;
			}

			x = 16;
			y = GAME_HEIGHT - 12;

			for (int i = 0; par[i]; i++) {
				DrawGlyph(&x, &y, par[i], i);
			}
		} else {
			const char *levelName = levels[currentLevel][13];
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

				DrawGlyph(&x, &y, c, i);
			}
		}
	} else if (message == MSG_STEP) {
		ControllerStep(true /* allowSounds */, keysHeld);
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
			if (l && t && r && b) entity->texture = TextureSub(tileset, 0 * 16, 6 * 16, 16, 16);
		}
	}
}

void Ending(Entity *entity, int message) {
	if (message == MSG_DRAW) {
		Fill(0, 0, GAME_WIDTH, GAME_HEIGHT, 0x222323);

		int tick = entity->tick - 220;
		int j = 0;

		if (tick > 20) {
			DRAW_STRING(25, 32 + 24 * 0, "you finished all 7 levels");
		}

		if (tick > 80) {
			DRAW_STRING(25, 32 + 24 * 1, "you achieved ");
			char t[2] = { '0', 0 };
			for (int i = 0; i < 7; i++) if (parTimesAchieved & (1 << i)) t[0]++;
			DRAW_STRING(25 + 8 * 13, 24 + 32 * 1, t);
			DRAW_STRING(25 + 8 * 14, 24 + 32 * 1, " out of 7 par times");
		}

		if (tick > 140) {
			DRAW_STRING(25, 32 + 24 * 2, "made by nakst for the gmtk22 jam");
		}

		if (tick > 200) {
			DRAW_STRING(25, 32 + 24 * 3, "thanks for playing amigo");
		}

		if (tick > 280) {
			DRAW_STRING(25, 32 + 24 * 4, "press m for the level select menu");
		}
	} else if (message == MSG_STEP) {
		Entity *entity = EntityCreate(&templateStar, (GAME_WIDTH + 60) * (GetRandomByte() / 255.0f) - 30, 0, 0);
		float speed = (GetRandomByte() / 255.0f) * 2.0f + 1.0f;
		entity->dy = speed;
		entity->layer = 4;
	}
}

void Load() {
	dice[0] = 1;
	dice[1] = 6;
	dice[2] = 2;
	dice[3] = 5;
	dice[4] = 3;
	dice[5] = 4;

	parMovesLeft = (levels[currentLevel][15][0] - '0') * 10 + (levels[currentLevel][15][1] - '0');

	for (int i = 0; i < 13; i++) {
		for (int j = 0; j < 20; j++) {
			char c = levels[currentLevel][i][j];
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

			if (c == 'e') {
				tile->message = Ending;
				tile->layer = 3;
			}
		}
	}
}

void GameInitialise(uint32_t save) {
	if ((save >> 16) < sizeof(levels) / sizeof(levels[0])) {
		currentLevel = save >> 16;
		levelsBeaten = (save >> 8) & 0xFF;
		parTimesAchieved = (save >> 0) & 0xFF;
	}

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

	controller = EntityCreate(NULL, 0, 0, 0);
	controller->message = Controller;
	controller->layer = 3;

	PlaySound("audio/bgm.opus", 14, true, 0.5);

	// Preload sound effects.
	PlaySound("audio/bump_sfx.wav", 18, false, 0.0);
	PlaySound("audio/slip_sfx.wav", 18, false, 0.0);
	PlaySound("audio/crate_sfx.wav", 19, false, 0.0);
	PlaySound("audio/win_sfx.wav", 17, false, 0.0);
	PlaySound("audio/reject_sfx.wav", 20, false, 0.0);
	PlaySound("audio/dice_roll_1.wav", 21, false, 0.0);
	PlaySound("audio/dice_roll_2.wav", 21, false, 0.0);
	PlaySound("audio/dice_roll_3.wav", 21, false, 0.0);
	PlaySound("audio/door_move_sfx.wav", 23, false, 0.0);
}

void GameUpdate() {
	{
		static int fontOffsetTick = 0;
		fontOffsetTick++;

		if ((fontOffsetTick % 40) == 0) {
			for (int i = 0; i < 50; i++) {
				fontOffsetX[i] = GetRandomByte() & 1;
				fontOffsetY[i] = GetRandomByte() & 1;
			}
		}
	}

	int repeat = 1;
	if (postUndo) repeat = 2;
	if (menuOpen) repeat = 0;

	for (int n = 0; n < repeat; n++) {
		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity *entity = entities + i;

			if (entity->slot == SLOT_USED) {
				entity->x += entity->dx;
				entity->y += entity->dy;
				entity->message(entity, MSG_STEP);
				entity->tick++;
			}
		}
	}

	if (menuOpen) {
		menuTick++;

		if (keysHeld['M']) {
			menuOpen = false;
			keysHeld['M'] = 0;
		} else if ((keysHeld['S'] || keysHeld[KEY_DOWN]) && menuIndex != 6) {
			menuIndex++;
			keysHeld['S'] = keysHeld[KEY_DOWN] = 0;
			if (~levelsBeaten & (1 << (menuIndex - 1))) menuIndex--;
		} else if ((keysHeld['W'] || keysHeld[KEY_UP]) && menuIndex != 0) {
			menuIndex--;
			keysHeld['W'] = keysHeld[KEY_UP] = 0;
			if (menuIndex != 0 && (~levelsBeaten & (1 << (menuIndex - 1)))) menuIndex++;
		} else if (keysHeld['D'] || keysHeld[KEY_RIGHT]) {
			for (int i = 0; i < MAX_ENTITIES; i++) {
				if ((entities[i].slot & SLOT_USED) && &entities[i] != controller) {
					EntityDestroy(&entities[i]);
				}
			}

			undoPosition = 0;
			currentLevel = menuIndex;
			levelNameTick = 1;
			menuOpen = false;
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

	for (int layer = -1; layer <= 4; layer++) {
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

	if (menuOpen) {
		FillBlend(0, 0, GAME_WIDTH, GAME_HEIGHT, 0xC0000000);
		int j = 0;

		DRAW_STRING(48, 16, "select the level menu");

		for (int i = 0; i < 7; i++) {
			char t[8] = "level  ";
			t[6] = '1' + i;
			DRAW_STRING(48, 40 + i * 16, t);

			if (i == 0 || (levelsBeaten & (1 << (i - 1)))) {
				if (parTimesAchieved & (1 << i)) {
					DRAW_STRING(160, 40 + i * 16, "achieved par");
				} else {
					if (levelsBeaten & (1 << (i))) {
						DRAW_STRING(160, 40 + i * 16, "havent got par");
					} else {
						DRAW_STRING(160, 40 + i * 16, "new");
					}
				}
			} else {
				DRAW_STRING(160, 40 + i * 16, "locked");
			}

			if (i == menuIndex) {
				int k[] = { 2, 3, 4, 5, 4, 3 };
				Blend(TextureSub(tileset, 16 * k[(menuTick / 5) % 6], 16 * 6, 16, 16), 28, 40 + i * 16 - 4, 0xFF);
			}
		}

		DRAW_STRING(48, 40 + 7 * 16 + 8, "press m to close menu");
		DRAW_STRING(48, 40 + 7 * 16 + 8 + 12, "press right arrow to switch");
	}
}
