#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define GAME_WIDTH  (320)
#define GAME_HEIGHT (200)
#define GAME_FPS    (60)

#ifdef DEFMEM
void *memset(void *s, int c, size_t n) {
	uint8_t *s8 = (uint8_t *) s;
	for (uintptr_t i = 0; i < n; i++) s8[i] = (uint8_t) c;
	return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
	uint8_t *dest8 = (uint8_t *) dest;
	const uint8_t *src8 = (const uint8_t *) src;
	for (uintptr_t i = 0; i < n; i++) dest8[i] = src8[i];
	return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
	if ((uintptr_t) dest < (uintptr_t) src) {
		return memcpy(dest, src, n);
	} else {
		uint8_t *dest8 = (uint8_t *) dest;
		const uint8_t *src8 = (const uint8_t *) src;
		for (uintptr_t i = n; i; i--) dest8[i - 1] = src8[i - 1];
		return dest;
	}
}

size_t strlen(const char *s) {
	const char *t = s;
	while (*s) s++;
	return s - t;
}

extern uint8_t __heap_base;
#endif

void LogInteger(uint32_t x);
void Save(uint32_t x);
void Panic();
void PlaySound(const char *name, uint32_t nameBytes, bool loop, double volume);

#include "png_decoder.c"
#include "bin/embed.h"

#define KEY_LEFT  (37)
#define KEY_UP    (38)
#define KEY_RIGHT (39)
#define KEY_DOWN  (40)
#define KEY_BACK  (8)

typedef struct Texture {
	uint32_t *bits;
	int32_t width, height, stride;
} Texture;

uint32_t *const imageData = (uint32_t *) &__heap_base;
uint8_t *allocatorPosition = &__heap_base + GAME_WIDTH * GAME_HEIGHT * 4;

uint32_t previousTimeMs;
uint8_t keysHeld[256];

uint64_t randomSeed;

uint8_t GetRandomByte() {
	randomSeed = randomSeed * 214013 + 2531011;
	return (uint8_t) (randomSeed >> 16);
}

void *Allocate(size_t bytes) {
	uint8_t *p = allocatorPosition;
	allocatorPosition += bytes;
	memset(p, 0, bytes);
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

Texture TextureSub(Texture texture, int32_t x, int32_t y, int32_t width, int32_t height) {
	if (x + width > texture.width || y + height > texture.height || x < 0 || y < 0 || width <= 0 || height <= 0) Panic();
	texture.bits += y * texture.stride + x;
	texture.width = width;
	texture.height = height;
	return texture;
}

bool ClipToGameBounds(Texture *texture, int32_t *x, int32_t *y) {
	if (*x >= GAME_WIDTH || *y >= GAME_HEIGHT) return false;
	if (*x + texture->width  > GAME_WIDTH ) *texture = TextureSub(*texture, 0, 0, GAME_WIDTH - *x, texture->height);
	if (*y + texture->height > GAME_HEIGHT) *texture = TextureSub(*texture, 0, 0, texture->width, GAME_HEIGHT - *y);
	if (texture->width + *x <= 0 || texture->height + *y <= 0) return false;
	if (*x < 0) *texture = TextureSub(*texture, -(*x), 0, texture->width + *x, texture->height), *x = 0;
	if (*y < 0) *texture = TextureSub(*texture, 0, -(*y), texture->width, texture->height + *y), *y = 0;
	return true;
}

void Blit(Texture texture, int32_t x, int32_t y) {
	if (!ClipToGameBounds(&texture, &x, &y)) return;

	for (int32_t i = 0; i < texture.height; i++) {
		for (int32_t j = 0; j < texture.width; j++) {
			imageData[(i + y) * GAME_WIDTH + (j + x)] = texture.bits[i * texture.stride + j];
		}
	}
}

void BlitScaled(Texture texture, int32_t x, int32_t y, int32_t w, int32_t h) {
	for (int32_t i = 0; i < h; i++) {
		for (int32_t j = 0; j < w; j++) {
			uint32_t tx = j * texture.width  / w;
			uint32_t ty = i * texture.height / h;

			if (tx < (uint32_t) texture.width && ty < (uint32_t) texture.height
					&& (uint32_t) (i + y) < (uint32_t) GAME_HEIGHT && (uint32_t) (j + x) < (uint32_t) GAME_WIDTH) {
				imageData[(i + y) * GAME_WIDTH + (j + x)] = texture.bits[ty * texture.stride + tx];
			}
		}
	}
}

void Blend(Texture texture, int32_t x, int32_t y, int32_t alpha) {
	if (!ClipToGameBounds(&texture, &x, &y)) return;
	if (alpha < 0) return;

	for (int32_t i = 0; i < texture.height; i++) {
		for (int32_t j = 0; j < texture.width; j++) {
			uint32_t *destinationPixel = &imageData[(i + y) * GAME_WIDTH + (j + x)];
			uint32_t modified = texture.bits[i * texture.stride + j];
			uint32_t m1 = (((modified & 0xFF000000) >> 24) * alpha) >> 8;

			if (m1 >= 0xFE) {
				*destinationPixel = modified;
			} else if (m1 != 0x00) {
				uint32_t m2 = 255 - m1;
				uint32_t original = *destinationPixel;
				uint32_t r2 = m2 * (original & 0x00FF00FF);
				uint32_t g2 = m2 * (original & 0x0000FF00);
				uint32_t r1 = m1 * (modified & 0x00FF00FF);
				uint32_t g1 = m1 * (modified & 0x0000FF00);
				uint32_t result = 0xFF000000 | (0x0000FF00 & ((g1 + g2) >> 8)) | (0x00FF00FF & ((r1 + r2) >> 8));
				*destinationPixel = result;
			}
		}
	}
}

void BlendScaled(Texture texture, int32_t x, int32_t y, int32_t w, int32_t h, int32_t alpha) {
	if (alpha < 0) return;

	for (int32_t i = 0; i < h; i++) {
		for (int32_t j = 0; j < w; j++) {
			uint32_t tx = j * texture.width  / w;
			uint32_t ty = i * texture.height / h;

			if (tx >= (uint32_t) texture.width || ty >= (uint32_t) texture.height
					|| (uint32_t) (i + y) >= (uint32_t) GAME_HEIGHT || (uint32_t) (j + x) >= (uint32_t) GAME_WIDTH) {
				continue;
			}

			uint32_t *destinationPixel = &imageData[(i + y) * GAME_WIDTH + (j + x)];
			uint32_t modified = texture.bits[ty * texture.stride + tx];
			uint32_t m1 = (((modified & 0xFF000000) >> 24) * alpha) >> 8;

			if (m1 >= 0xFE) {
				*destinationPixel = modified;
			} else if (m1 != 0x00) {
				uint32_t m2 = 255 - m1;
				uint32_t original = *destinationPixel;
				uint32_t r2 = m2 * (original & 0x00FF00FF);
				uint32_t g2 = m2 * (original & 0x0000FF00);
				uint32_t r1 = m1 * (modified & 0x00FF00FF);
				uint32_t g1 = m1 * (modified & 0x0000FF00);
				uint32_t result = 0xFF000000 | (0x0000FF00 & ((g1 + g2) >> 8)) | (0x00FF00FF & ((r1 + r2) >> 8));
				*destinationPixel = result;
			}
		}
	}
}

void Fill(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
	int32_t l = x, r = x + width, t = y, b = y + height;
	if (l < 0) l = 0;
	if (r > GAME_WIDTH) r = GAME_WIDTH;
	if (t < 0) t = 0;
	if (b > GAME_HEIGHT) b = GAME_HEIGHT;
	color = (color & 0xFF00) | ((color & 0xFF0000) >> 16) | ((color & 0xFF) << 16);

	for (int32_t i = t; i < b; i++) {
		for (int32_t j = l; j < r; j++) {
			imageData[i * GAME_WIDTH + j] = color | 0xFF000000;
		}
	}
}

float FadeInOut(float t) {
	if (t < 0.3f) return t / 0.3f;
	else if (t < 0.7f) return 1;
	else return 1 - (t - 0.7f) / 0.3f;
}

float SmoothStep(float x) {
	return x * x * x * (x * (x * 6 - 15) + 10);
}

#ifndef M_PI
#define M_PI (3.14159)
#endif

typedef union ConvertFloatInteger {
	float f;
	uint32_t i;
} ConvertFloatInteger;

#define F(x) (((ConvertFloatInteger) { .i = (x) }).f)

float EsCRTfloorf(float x) {
	ConvertFloatInteger convert = {x};
	uint32_t sign = convert.i & 0x80000000;
	int exponent = (int) ((convert.i >> 23) & 0xFF) - 0x7F;

	if (exponent >= 23) {
		// There aren't any bits representing a fractional part.
	} else if (exponent >= 0) {
		// Positive exponent.
		uint32_t mask = 0x7FFFFF >> exponent;
		if (!(mask & convert.i)) return x; // Already an integer.
		if (sign) convert.i += mask;
		convert.i &= ~mask; // Mask out the fractional bits.
	} else if (exponent < 0) {
		// Negative exponent.
		return sign ? -1.0 : 0.0;
	}

	return convert.f;
}

float _SineFloat(float x) {
	// Calculates sin(x) for x in [0, pi/4].

	float x2 = x * x;

	return x * (F(0x3F800000) + x2 * (F(0xBE2AAAA0) + x2 * (F(0x3C0882C0) + x2 * F(0xB94C6000))));
}

float _CosineFloat(float x) {
	// Calculates cos(x) for x in [0, pi/4].

	float x2 = x * x;

	return F(0x3F800000) + x2 * (F(0xBEFFFFDA) + x2 * (F(0x3D2A9F60) + x2 * F(0xBAB22C00)));
}

float EsCRTsinf(float x) {
	bool negate = false;

	// x in -infty, infty

	if (x < 0) {
		x = -x;
		negate = true;
	}

	// x in 0, infty

	x -= 2 * M_PI * EsCRTfloorf(x / (2 * M_PI));

	// x in 0, 2*pi

	if (x < M_PI / 2) {
	} else if (x < M_PI) {
		x = M_PI - x;
	} else if (x < 3 * M_PI / 2) {
		x = x - M_PI;
		negate = !negate;
	} else {
		x = M_PI * 2 - x;
		negate = !negate;
	}

	// x in 0, pi/2

	float y = x < M_PI / 4 ? _SineFloat(x) : _CosineFloat(M_PI / 2 - x);
	return negate ? -y : y;
}

float EsCRTcosf(float x) {
	bool negate = false;

	// x in -infty, infty

	if (x < 0) {
		x = -x;
	}

	// x in 0, infty

	x -= 2 * M_PI * EsCRTfloorf(x / (2 * M_PI));

	// x in 0, 2*pi

	if (x < M_PI / 2) {
	} else if (x < M_PI) {
		x = M_PI - x;
		negate = !negate;
	} else if (x < 3 * M_PI / 2) {
		x = x - M_PI;
		negate = !negate;
	} else {
		x = M_PI * 2 - x;
	}

	// x in 0, pi/2

	float y = x < M_PI / 4 ? _CosineFloat(x) : _SineFloat(M_PI / 2 - x);
	return negate ? -y : y;
}

#undef F

#include "game.c"

void Initialise(uint32_t save) {
	randomSeed = 1347135;
	GameInitialise(save);
}

void GenerateFrame(uint32_t timeMs) {
	if (timeMs < previousTimeMs) {
		previousTimeMs = timeMs; // Handle integer overflow.
	}

	if (timeMs - previousTimeMs > 500) {
		previousTimeMs = timeMs - 500; // Limit frames to process if lagging.
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
