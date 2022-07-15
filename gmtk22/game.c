#define SLOT_USED       (1 << 0)
#define SLOT_DESTROYING (1 << 1)
#define SLOT_HIDE       (1 << 2)

#define TAG_SOLID (1 << 0)

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
	void (*message)(struct Entity *entity, int message, int di);
} Entity;

typedef struct GameState {
#define MAX_ENTITIES (1000)
	Entity entities[MAX_ENTITIES];
} GameState;

GameState *state;

Entity *EntityCreate(Entity *templateEntity, float x, float y, uint32_t uid) {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = state->entities + i;
		if (entity->slot & SLOT_USED) continue;
		*entity = *templateEntity;
		entity->slot |= SLOT_USED;
		if (entity->frameCount) entity->frameCount = 1;
		if (!entity->stepsPerFrame) entity->stepsPerFrame = 1;
		entity->x = x;
		entity->y = y;
		if (!entity->w) entity->w = entity->texture.width;
		if (!entity->h) entity->h = entity->texture.height;
		entity->uid = uid;
		entity->message(entity, MSG_CREATE, 0);
		return entity;
	}
	
	static Entity fake = {};
	return &fake;
}

void EntityDestroy(Entity *entity) {
	if (~entity->slot & SLOT_DESTROYING) {
		entity->slot |= SLOT_DESTROYING;
		entity->message(entity, MSG_DESTROY, 0);
	}
}

Entity *EntityFind(float x, float y, float w, float h, uint8_t tag, uint8_t flags, Entity *exclude) {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = state->entities + i;
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

void GameInitialise(uint32_t save) {
	state = Allocate(sizeof(GameState));
}

void GameUpdate() {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = state->entities + i;

		if (entity->slot & SLOT_USED) {
			entity->message(entity, MSG_STEP, 0);
			entity->tick++;
		}
	}

	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity *entity = state->entities + i;

		if (entity->slot & SLOT_DESTROYING) {
			entity->slot &= ~SLOT_USED;
		}
	}
}

void GameRender() {
	for (int layer = -1; layer <= 3; layer++) {
		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity *entity = state->entities + i;
			if ((~entity->slot & SLOT_USED) || (entity->slot & SLOT_HIDE) || (entity->layer != layer)) continue;
			
			if (entity->texture.width && entity->texture.height) {
				uint32_t frame = entity->stepsPerFrame >= 0 
					? ((entity->tick / entity->stepsPerFrame) % entity->frameCount) 
					: (-entity->stepsPerFrame - 1);
				entity->texture.bits += frame * entity->texture.width * entity->texture.stride;
				Blend(entity->texture, entity->x + entity->texOffX, entity->y + entity->texOffY, 0xFF);
			}

			entity->message(entity, MSG_DRAW, layer);
		}
	}
}
