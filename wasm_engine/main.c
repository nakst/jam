#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define GAME_WIDTH  (320)
#define GAME_HEIGHT (200)
#define GAME_FPS    (60)

void *memset(void *s, int c, size_t n) {
	uint8_t *s8 = (uint8_t *) s;
	for (uintptr_t i = 0; i < n; i++) {
		s8[i] = (uint8_t) c;
	}
	return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
	uint8_t *dest8 = (uint8_t *) dest;
	const uint8_t *src8 = (const uint8_t *) src;
	for (uintptr_t i = 0; i < n; i++) {
		dest8[i] = src8[i];
	}
	return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
	if ((uintptr_t) dest < (uintptr_t) src) {
		return memcpy(dest, src, n);
	} else {
		uint8_t *dest8 = (uint8_t *) dest;
		const uint8_t *src8 = (const uint8_t *) src;
		for (uintptr_t i = n; i; i--) {
			dest8[i - 1] = src8[i - 1];
		}
		return dest;
	}
}

#include "png_decoder.c"
#include "embed.h"

#define KEY_LEFT  (37)
#define KEY_UP    (38)
#define KEY_RIGHT (39)
#define KEY_DOWN  (40)

#define RECT_4PD(x, y, w, h) ((Rectangle) { (int32_t) (x), (int32_t) ((x) + (w)), (int32_t) (y), (int32_t) ((y) + (h)) })

typedef struct Texture {
	uint32_t *bits;
	uint32_t width, height, stride;
} Texture;

typedef struct Rectangle {
	int32_t l, r, t, b;
} Rectangle;

void LogInteger(uint32_t x);
void Panic();
void PlaySound(const char *name, uint32_t nameBytes, bool loop);
extern uint8_t __heap_base;

volatile uint32_t *const imageData = (volatile uint32_t *) &__heap_base;
uint8_t *allocatorPosition = &__heap_base + GAME_WIDTH * GAME_HEIGHT * 4;
uint32_t previousTimeMs;
uint8_t keysHeld[256];

void *Allocate(size_t bytes) {
	uint8_t *p = allocatorPosition;
	allocatorPosition += bytes;
	return p;
}

Texture TextureCreate(const uint8_t *data, size_t dataBytes) {
	uint8_t *originalAllocatorPosition = allocatorPosition;
	PNGReader reader = { 0 };
	reader.buffer = data;
	reader.bytes = dataBytes;
	Texture texture;
	bool success = PNGParse(&reader, &texture.bits, &texture.width, &texture.height, Allocate);
	if (!success) Panic();
	allocatorPosition = originalAllocatorPosition;
	uint32_t *newBits = Allocate(texture.width * texture.height * 4);
	memmove(newBits, texture.bits, texture.width * texture.height * 4);
	texture.bits = newBits;
	texture.stride = texture.width;
	return texture;
}

Texture TextureSub(Texture texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	if (x < 0 || x + width > texture.width || y < 0 || y + height > texture.height || !width || !height) Panic();
	texture.bits += y * texture.stride + x;
	texture.width = width;
	texture.height = height;
	return texture;
}

Rectangle RectangleIntersection(Rectangle a, Rectangle b) {
	if (a.l < b.l) a.l = b.l;
	if (a.t < b.t) a.t = b.t;
	if (a.r > b.r) a.r = b.r;
	if (a.b > b.b) a.b = b.b;
	return a;
}

void Draw(Texture texture, uint32_t x, uint32_t y) {
	Rectangle clip = RectangleIntersection(RECT_4PD(x, y, texture.width, texture.height), RECT_4PD(0, 0, GAME_WIDTH, GAME_HEIGHT));
	if (clip.r <= clip.l || clip.b <= clip.t) return;
	texture = TextureSub(texture, clip.l - x, clip.t - y, clip.r - clip.l, clip.b - clip.t);
	x = clip.l, y = clip.t;

	for (uintptr_t i = 0; i < texture.height; i++) {
		for (uintptr_t j = 0; j < texture.width; j++) {
			imageData[(i + y) * GAME_WIDTH + (j + x)] = texture.bits[i * texture.stride + j];
		}
	}
}

// TODO Advanced drawing routines.
void Draw2(Texture texture, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void Draw3(Texture texture, uint32_t x, uint32_t y, float alpha);
void Draw4(Texture texture, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float red, float green, float blue, float alpha, float rotation);

#include "game.c"

void Initialise() {
	GameInitialise();
}

void GenerateFrame(uint32_t timeMs) {
	if (timeMs < previousTimeMs) {
		previousTimeMs = timeMs; // Handle integer overflow.
	}

	while (previousTimeMs + 1000 / GAME_FPS < timeMs) {
		previousTimeMs += 1000 / GAME_FPS;
		GameUpdate();
	}

	GameRender();
}

void HandleEvent(uint8_t event, uint32_t detail) {
	if (event == 1) keysHeld[detail] = 1;
	if (event == 2) keysHeld[detail] = 0;
}
