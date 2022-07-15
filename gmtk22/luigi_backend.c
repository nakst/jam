#define UI_IMPLEMENTATION
#include "../luigi2.h"
uint8_t heapBase[64 * 1024 * 1024];
#define __heap_base (heapBase[0])
#include "main.c"
#include <stdio.h>

void LogInteger(uint32_t x) { 
	fprintf(stderr, "%d\n", x); 
}

void Panic() { 
	fprintf(stderr, "panic\n"); 
	while (1); 
}

void PlaySound(const char *name, uint32_t nameBytes, bool loop) { 
	// TODO.
	fprintf(stderr, "%s '%.*s'\n", loop ? "loop" : "oneshot", (int) nameBytes, name); 
}

void Save(uint32_t save) {
	FILE *f = fopen("bin/save.dat", "wb");
	if (f) { fwrite(&save, 1, 4, f); fclose(f); }
}

int CanvasMessage(UIElement *element, UIMessage message, int di, void *dp) {
	if (message == UI_MSG_ANIMATE) {
		GenerateFrame(UIAnimateClock());
		UIElementRepaint(element, NULL);
	} else if (message == UI_MSG_PAINT) {
		UIPainter *painter = (UIPainter *) dp;
		int scale = painter->height / GAME_HEIGHT;
		int scale2 = painter->width / GAME_WIDTH;
		if (scale2 < scale) scale = scale2;
		UIDrawBlock(painter, element->bounds, 0x808080);
		if (scale < 1) return 0;
		int ox = (painter->width - GAME_WIDTH * scale) >> 1;
		int oy = (painter->height - GAME_HEIGHT * scale) >> 1;

		for (int i = 0; i < GAME_HEIGHT * scale; i++) {
			for (int j = 0; j < GAME_WIDTH * scale; j++) {
				uint32_t p = ((const uint32_t *) heapBase)[(i / scale) * GAME_WIDTH + (j / scale)];
				p = (p & 0xFF00) | (p << 16) | ((p >> 16) & 0xFF);
				painter->bits[(oy + i) * painter->width + (ox + j)] = p;
			}
		}
	} else if (message == UI_MSG_KEY_TYPED || message == UI_MSG_KEY_RELEASED) {
		uint32_t code = ((UIKeyTyped *) dp)->code;
		if (code == UI_KEYCODE_LEFT) code = KEY_LEFT;
		else if (code == UI_KEYCODE_RIGHT) code = KEY_RIGHT;
		else if (code == UI_KEYCODE_UP) code = KEY_UP;
		else if (code == UI_KEYCODE_DOWN) code = KEY_DOWN;
		else code = 0;
		if (code) HandleEvent(message == UI_MSG_KEY_RELEASED ? 2 : 1, code);
	}

	return 0;
}

int main() {
	FILE *f = fopen("bin/save.dat", "rb");
	uint32_t save = 0;
	if (f) { fread(&save, 1, 4, f); fclose(f); }
	Initialise(save);
	UIInitialise();
	UIWindowCreate(0, UI_ELEMENT_PARENT_PUSH, "gmtk22", GAME_WIDTH, GAME_HEIGHT);
	UIElement *canvas = UIElementCreate(sizeof(UIElement), 0, 0, CanvasMessage, "Canvas");
	UIElementAnimate(canvas, false);
	UIElementFocus(canvas);
	return UIMessageLoop();
}
