#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define GAME_WIDTH  (144)
#define GAME_HEIGHT (144)
#define GAME_FPS    (60)

#ifdef __linux__
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

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

int strcmp(const char *s1, const char *s2) {
	while (true) {
		if (*s1 != *s2) {
			if (*s1 == 0) return -1;
			else if (*s2 == 0) return 1;
			return *s1 - *s2;
		}

		if (*s1 == 0) {
			return 0;
		}

		s1++;
		s2++;
	}
}

size_t strlen(const char *s) {
	const char *t = s;
	while (*s) s++;
	return s - t;
}

char *strcpy(char *dest, const char *src) {
	memcpy(dest, src, strlen(src) + 1);
	return dest;
}

extern uint8_t __heap_base;
#endif

void LogInteger(uint32_t x);
void SaveGameToCookie(const char *data, uint32_t dataBytes);
void Panic();
void PlaySound(const char *name, uint32_t nameBytes, int loop, double volume);
void WriteSave(const char *name, uint32_t nameBytes, const char *value, uint32_t valueBytes);
void ReadSave(const char *name, uint32_t nameBytes, char *value, uint32_t valueBytes);

#define PLAY_SOUND(name, loop, volume) PlaySound(name, strlen(name), loop, volume)

#include "png_decoder.c"
#include "bin/embed.h"

#define KEY_LEFT  (37)
#define KEY_UP    (38)
#define KEY_RIGHT (39)
#define KEY_DOWN  (40)
#define KEY_SHIFT (16)

typedef struct Texture {
	uint32_t *bits;
	int32_t width, height, stride;
} Texture;

static uint32_t *const imageData = (uint32_t *) &__heap_base;
static uint8_t *allocatorPosition = &__heap_base + GAME_WIDTH * GAME_HEIGHT * 4;

static Texture drawTarget;
static int32_t cameraX;
static int32_t cameraY;

static uint32_t previousTimeMs;
static uint8_t keysHeld[256];

static uint64_t randomSeed;

uint8_t GetRandomByte() {
	randomSeed = randomSeed * 214013 + 2531011;
	return (uint8_t) (randomSeed >> 16);
}

typedef union ConvertFloatInteger {
	float f;
	uint32_t i;
} ConvertFloatInteger;

#define FLB(x) (((ConvertFloatInteger) { .i = (x) }).f)
#define PI (3.14159f)

static float /* -1..1 */ CheapSineApproximation01(float x /* 0..1 */) {
	return ((((-56.8889f * x + 142.2222f) * x - 103.1111f) * x + 12.4444f) * x + 5.3333f) * x;
}

static float _SineFloat(float x) {
	float x2 = x * x;
	return x * (FLB(0x3F800000) + x2 * (FLB(0xBE2AAAA0) + x2 * (FLB(0x3C0882C0) + x2 * FLB(0xB94C6000))));
}

static float _CosineFloat(float x) {
	float x2 = x * x;
	return FLB(0x3F800000) + x2 * (FLB(0xBEFFFFDA) + x2 * (FLB(0x3D2A9F60) + x2 * FLB(0xBAB22C00)));
}

static float Sin(float x) {
	bool negate = false;
	if (x < 0) { x = -x; negate = true; }
	x -= 2 * PI * __builtin_floorf(x / (2 * PI));

	if (x < PI / 2) {
	} else if (x < PI) {
		x = PI - x;
	} else if (x < 3 * PI / 2) {
		x = x - PI;
		negate = !negate;
	} else {
		x = PI * 2 - x;
		negate = !negate;
	}

	float y = x < PI / 4 ? _SineFloat(x) : _CosineFloat(PI / 2 - x);
	return negate ? -y : y;
}

static float Cos(float x) {
	return Sin(x + PI / 2);
}

static float SignFloat(float x) {
	return x > 0 ? 1 : x < 0 ? -1 : 0;
}

static float _ArcTangentFloat(float x) {
	float x2 = x * x;
	return x * (FLB(0x3F7FFFF8) + x2 * (FLB(0xBEAAA53C) + x2 * (FLB(0x3E4BC990) + x2 * (FLB(0xBE084A60) + x2 * FLB(0x3D8864B0)))));
}

static float ArcTan(float x) {
	bool negate = false;
	if (x < 0) { x = -x; negate = true; }
	bool reciprocalTaken = false;
	if (x > 1) { x = 1 / x; reciprocalTaken = true; }
	float y = x < 0.5f ? _ArcTangentFloat(x) : (0.463647609000806116f + _ArcTangentFloat((2 * x - 1) / (2 + x)));
	if (reciprocalTaken) { y = PI / 2 - y; }
	return negate ? -y : y;
}

static float ArcTan2(float y, float x) {
	if (x == 0) return y > 0 ? PI / 2 : -PI / 2;
	else if (x > 0) return ArcTan(y / x);
	else if (y >= 0) return PI + ArcTan(y / x);
	else return -PI + ArcTan(y / x);
}

static void *Allocate(size_t bytes) {
	uint8_t *p = allocatorPosition;
	allocatorPosition += bytes;
	memset(p, 0, bytes);
	return p;
}

static Texture TextureCreate(const uint8_t *data, size_t dataBytes) {
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

static Texture TextureSub(Texture texture, int32_t x, int32_t y, int32_t width, int32_t height) {
	if (x + width > texture.width || y + height > texture.height || x < 0 || y < 0 || width <= 0 || height <= 0) Panic();
	texture.bits += y * texture.stride + x;
	texture.width = width;
	texture.height = height;
	return texture;
}

static bool ClipToGameBounds(Texture *texture, int32_t *x, int32_t *y) {
	if (*x >= drawTarget.width || *y >= drawTarget.height) return false;
	if (*x + texture->width  > drawTarget.width ) *texture = TextureSub(*texture, 0, 0, drawTarget.width - *x, texture->height);
	if (*y + texture->height > drawTarget.height) *texture = TextureSub(*texture, 0, 0, texture->width, drawTarget.height - *y);
	if (texture->width + *x <= 0 || texture->height + *y <= 0) return false;
	if (*x < 0) *texture = TextureSub(*texture, -(*x), 0, texture->width + *x, texture->height), *x = 0;
	if (*y < 0) *texture = TextureSub(*texture, 0, -(*y), texture->width, texture->height + *y), *y = 0;
	return true;
}

static void Blit(Texture texture, int32_t x, int32_t y) {
	x -= cameraX, y -= cameraY;
	if (!ClipToGameBounds(&texture, &x, &y)) return;

	for (int32_t i = 0; i < texture.height; i++) {
		for (int32_t j = 0; j < texture.width; j++) {
			drawTarget.bits[(i + y) * drawTarget.stride + (j + x)] = texture.bits[i * texture.stride + j];
		}
	}
}

static void BlitScaled(Texture texture, int32_t x, int32_t y, int32_t w, int32_t h) {
	x -= cameraX, y -= cameraY;
	for (int32_t i = 0; i < h; i++) {
		for (int32_t j = 0; j < w; j++) {
			uint32_t tx = j * texture.width  / w;
			uint32_t ty = i * texture.height / h;

			if (tx < (uint32_t) texture.width && ty < (uint32_t) texture.height
					&& (uint32_t) (i + y) < (uint32_t) drawTarget.height && (uint32_t) (j + x) < (uint32_t) drawTarget.width) {
				drawTarget.bits[(i + y) * drawTarget.stride + (j + x)] = texture.bits[ty * texture.stride + tx];
			}
		}
	}
}

static void Blend(Texture texture, int32_t x, int32_t y, int32_t alpha) {
	x -= cameraX, y -= cameraY;
	if (!ClipToGameBounds(&texture, &x, &y)) return;
	if (alpha <= 0) return;

	for (int32_t i = 0; i < texture.height; i++) {
		for (int32_t j = 0; j < texture.width; j++) {
			uint32_t *destinationPixel = &drawTarget.bits[(i + y) * drawTarget.stride + (j + x)];
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

static void BlendRotated(Texture texture, int32_t x, int32_t y, int32_t alpha, float angle) {
	x -= cameraX, y -= cameraY;
	if (!ClipToGameBounds(&texture, &x, &y)) return;
	if (alpha <= 0) return;

	float c = Cos(angle);
	float s = Sin(angle);

	for (int32_t i = 0; i < texture.height; i++) {
		for (int32_t j = 0; j < texture.width; j++) {
			int32_t i0 = c * i + s * j + y;
			int32_t j0 = -s * i + c * j + x;

			if (i0 < 0 || i0 >= drawTarget.height || j0 < 0 || j0 >= drawTarget.width) {
				continue;
			}

			uint32_t *destinationPixel = &drawTarget.bits[i0 * drawTarget.stride + j0];
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

static void BlendScaled(Texture texture, int32_t x, int32_t y, int32_t w, int32_t h, int32_t alpha) {
	x -= cameraX, y -= cameraY;
	if (alpha < 0) return;

	for (int32_t i = 0; i < h; i++) {
		for (int32_t j = 0; j < w; j++) {
			uint32_t tx = j * texture.width  / w;
			uint32_t ty = i * texture.height / h;

			if (tx >= (uint32_t) texture.width || ty >= (uint32_t) texture.height
					|| (uint32_t) (i + y) >= (uint32_t) drawTarget.height || (uint32_t) (j + x) >= (uint32_t) drawTarget.width) {
				continue;
			}

			uint32_t *destinationPixel = &drawTarget.bits[(i + y) * drawTarget.stride + (j + x)];
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

static void Fill(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
	x -= cameraX, y -= cameraY;
	int32_t l = x, r = x + width, t = y, b = y + height;
	if (l < 0) l = 0;
	if (r > drawTarget.width) r = drawTarget.width;
	if (t < 0) t = 0;
	if (b > drawTarget.height) b = drawTarget.height;
//	color = (color & 0xFF00) | ((color & 0xFF0000) >> 16) | ((color & 0xFF) << 16);

	for (int32_t i = t; i < b; i++) {
		for (int32_t j = l; j < r; j++) {
			drawTarget.bits[i * drawTarget.stride + j] = color | 0xFF000000;
		}
	}
}

static void BlendPixel(int32_t x, int32_t y, uint32_t color) {
	uint32_t *destinationPixel = &drawTarget.bits[y * drawTarget.stride + x];
	uint32_t modified = color;
	uint32_t m1 = color >> 24;

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

static void FillBlend(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
	x -= cameraX, y -= cameraY;
	int32_t l = x, r = x + width, t = y, b = y + height;
	if (l < 0) l = 0;
	if (r > drawTarget.width) r = drawTarget.width;
	if (t < 0) t = 0;
	if (b > drawTarget.height) b = drawTarget.height;
	color = (color & 0xFF00FF00) | ((color & 0xFF0000) >> 16) | ((color & 0xFF) << 16);

	for (int32_t i = t; i < b; i++) {
		for (int32_t j = l; j < r; j++) {
			BlendPixel(j, i, color);
		}
	}
}

static void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	color = (color & 0xFF00FF00) | ((color & 0xFF0000) >> 16) | ((color & 0xFF) << 16);
	x0 -= cameraX, y0 -= cameraY;
	x1 -= cameraX, y1 -= cameraY;
	x2 -= cameraX, y2 -= cameraY;

	// Step 1: Sort the points by their y-coordinate.
	if (y1 < y0) { int xt = x0; x0 = x1, x1 = xt; int yt = y0; y0 = y1, y1 = yt; }
	if (y2 < y1) { int xt = x1; x1 = x2, x2 = xt; int yt = y1; y1 = y2, y2 = yt; }
	if (y1 < y0) { int xt = x0; x0 = x1, x1 = xt; int yt = y0; y0 = y1, y1 = yt; }
	if (y2 == y0) return;

	// Step 2: Clip the triangle.
	if (x0 < 0 && x1 < 0 && x2 < 0) return;
	if (x0 >= drawTarget.width && x1 >= drawTarget.width && x2 >= drawTarget.width) return;
	if (y2 < 0 || y0 >= drawTarget.height) return;
	bool needsXClip = x0 < 0 + 1 || x0 >= drawTarget.width - 1
		|| x1 < 0 + 1 || x1 >= drawTarget.width - 1
		|| x2 < 0 + 1 || x2 >= drawTarget.width - 1;
	bool needsYClip = y0 < 0 + 1 || y2 >= drawTarget.height - 1;
#define _APPLY_CLIP(xo, yo) \
	if (needsYClip && (yi + yo < 0 || yi + yo >= drawTarget.height)) continue; \
	if (needsXClip && xf + xo < 0) xf = 0 - xo; \
	if (needsXClip && xt + xo > drawTarget.width) xt = drawTarget.width - xo;

	// Step 3: Split into 2 triangles with bases aligned with the x-axis.
	float xm0 = (x2 - x0) * (y1 - y0) / (y2 - y0), xm1 = x1 - x0;
	if (xm1 < xm0) { float xmt = xm0; xm0 = xm1, xm1 = xmt; }
	float xe0 = xm0 + x0 - x2, xe1 = xm1 + x0 - x2;
	int ym = y1 - y0, ye = y2 - y1;
	float ymr = 1.0f / ym, yer = 1.0f / ye;

	// Step 4: Draw the top part.
	for (float y = 0; y < ym; y++) {
		int xf = xm0 * y * ymr, xt = xm1 * y * ymr, yi = (int) y;
		_APPLY_CLIP(x0, y0);
		for (int x = xf; x < xt; x++) BlendPixel(x0 + x, y0 + yi, color);
	}

	// Step 5: Draw the bottom part.
	for (float y = 0; y < ye; y++) {
		int xf = xe0 * (ye - y) * yer, xt = xe1 * (ye - y) * yer, yi = (int) y;
		_APPLY_CLIP(x2, y1);
		for (int x = xf; x < xt; x++) BlendPixel(x2 + x, y1 + yi, color);
	}
}


static float FadeInOut(float t) {
	if (t < 0.3f) return t / 0.3f;
	else if (t < 0.7f) return 1;
	else return 1 - (t - 0.7f) / 0.3f;
}

static float SmoothStep(float x) {
	return x * x * x * (x * (x * 6 - 15) + 10);
}

static float ClampRange(float from, float to, float x) {
	return x > to ? to : x < from ? from : x;
}

static float SmoothLinearMap(float inFrom, float inTo, float outFrom, float outTo, float value) {
	float raw = SmoothStep((value - inFrom) / (inTo - inFrom));
	return raw * (outTo - outFrom) + outFrom;
}

static float SmoothInLinearMap(float inFrom, float inTo, float outFrom, float outTo, float value) {
	float raw = (value - inFrom) / (inTo - inFrom);
	raw = ClampRange(0, 1, raw);
	raw *= raw;
	raw *= raw;
	return raw * (outTo - outFrom) + outFrom;
}

static float LinearMap(float inFrom, float inTo, float outFrom, float outTo, float value) {
	float raw = (value - inFrom) / (inTo - inFrom);
	return raw * (outTo - outFrom) + outFrom;
}

#define MIN(a, b) ((a)>(b)?(b):(a))
#define MAX(a, b) ((a)<(b)?(b):(a))
#define ABSDIFF(a, b) (MAX(a,b)-MIN(a,b))

#include "game.c"

int Initialise() {
	randomSeed = 1347135;
	GameInitialise();
	return (GAME_WIDTH << 16) | GAME_HEIGHT;
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
