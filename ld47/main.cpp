#include <windows.h> 
#include <GL/gl.h> 
#include <shellscalingapi.h>
#include <stdint.h>
#include <Shlobj.h>
#include <wincodec.h>
#include <shlwapi.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include "bin/embed.h"
#include "bin/embed_def.h"

void DoExit(int status) {
	ExitProcess(status);
}

uint8_t keysPressed[256], keysReleased[256], keysDown[256];

///////////////////////////////////////////////

uint32_t backgroundColor;

int mouseX, mouseY;
bool mouseDown;

#define GAME_SIZE (380)
#define DEFAULT_SCALE (2)

#define BRAND "fly"
#define BRANDW L"fly"

struct Texture { unsigned width, height, id; };
void Draw(Texture *texture, 
	float x, float y, float w = -1, float h = -1, 
	float sx = 0, float sy = 0, float sw = -1, float sh = -1, 
	float alpha = 1, float r = 1, float g = 1, float b = 1,
	float rot = 0);
void CreateTexture(Texture *texture, uint8_t *data, size_t dataBytes);

bool KeyPressed(int x) { uint8_t r = keysPressed[x]; keysPressed[x] = 0; return r; }
bool KeyReleased(int x) { uint8_t r = keysReleased[x]; keysReleased[x] = 0; return r; }
bool KeyDown(int x) { uint8_t r = keysDown[x]; return r; }

void InitialiseGame();
void UpdateGame();
void RenderGame();

///////////////////////////////////////////////

extern "C" int _fltused = 0;
typedef HRESULT (*GetDpiForMonitorType)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);
typedef BOOL (*SetProcessDpiAwarenessContextType)(DPI_AWARENESS_CONTEXT value);
HMODULE shcore, instance;
HWND root;
HGLRC renderingContext;
HDC dummyDC, mainDC;
unsigned imageShader, imageShaderTransform, imageShaderTextureTransform, imageShaderBlendColor, imageShaderTextureSampler;
unsigned squareIBO, squareVBO;
int destWidth, destHeight, destX, destY;
bool destFlip;
IWICImagingFactory *imagingFactory;

float (*sinf)(float);
float (*cosf)(float);
float (*atan2f)(float, float);

#pragma function(memset)
void *memset(void *s, int c, size_t n) {
	char *s2 = (char *) s;
	while (n--) *s2++ = c;
	return s;
}

#pragma function(memcmp)
int memcmp(const void *a, const void *b, size_t n) {
	uint8_t *a2 = (uint8_t *) a;
	uint8_t *b2 = (uint8_t *) b;
	for (uintptr_t i = 0; i < n; i++) {
		if (a2[i] > b2[i]) return 1;
		if (a2[i] < b2[i]) return -1;
	}
	return 0;
}

#pragma function(memcpy)
void *memcpy(void *d, const void *s, size_t n) {
	char *s2 = (char *) s, *d2 = (char *) d;
	while (n--) *d2++ = *s2++;
	return d;
}

#pragma function(strlen)
size_t strlen(const char *s) {
	size_t c = 0;
	for (; *s; s++) c++;
	return c;
}

#pragma function(memmove)
void *memmove(void *d, const void *s, size_t n) {
	char *s2 = (char *) s, *d2 = (char *) d;
	if (d2 < s2) { while (n--) *d2++ = *s2++; return d; }
	s2 += n, d2 += n;
	while (n--) *(--d2) = *(--s2);
	return d;
}

void *malloc(size_t size) {
	return HeapAlloc(GetProcessHeap(), 0, size);
}

void free(void *pointer) {
	HeapFree(GetProcessHeap(), 0, pointer);
}

int ReadProperty(const char *string, const char *property) {
	size_t propertyLength = strlen(property);
	size_t stringLength = strlen(string);
	
	for (int i = 0; i < stringLength - propertyLength; i++) {
		for (int j = 0; j < propertyLength; j++) {
			if (string[i + j] != property[j]) {
				goto next;
			}
		}
		
		if (string[i + propertyLength] != '=') {
			goto next;
		}
		
		i += propertyLength + 1;
		int value = 0;
		
		while (string[i] >= '0' && string[i] <= '9') {
			value *= 10;
			value += string[i] - '0';
			i++;
		}
		
		return value;
		
		next:;
	}
	
	return -1;
}

void Assert(bool x) {
	if (!x) {
		MessageBox(root, "An internal error occurred. You may need to update your graphics drivers.", 0, MB_OK);
		DoExit(0);
	}
}

void StringCopy(wchar_t *d, const wchar_t *s) {
	while (true) {
		wchar_t c = *s++;
		*d++ = c;
		if (!c) break;
	}
}

void StringAppend(wchar_t *d, const wchar_t *s) {
	while (*d) d++;
	StringCopy(d, s);
}

#define GL_CLAMP_TO_EDGE 0x812F
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_TEXTURE0 0x84C0
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_MULTISAMPLE 0x809D
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_FRAMEBUFFER 0x8D40
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_BGRA 0x80E1

typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLboolean;
typedef signed char GLbyte;
typedef char GLchar;
typedef short GLshort;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned long GLulong;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

GLuint (APIENTRY *glCreateProgram	    )();
void   (APIENTRY *glShaderSource	    )(GLuint shader, GLsizei count, const GLchar *const* string, const GLint* length);
GLuint (APIENTRY *glCreateShader	    )(GLenum type);
void   (APIENTRY *glCompileShader	    )(GLuint shader);
void   (APIENTRY *glGetShaderiv		    )(GLuint shader, GLenum pname, GLint* param);
void   (APIENTRY *glGetShaderInfoLog	    )(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
void   (APIENTRY *glAttachShader	    )(GLuint program, GLuint shader);
void   (APIENTRY *glLinkProgram		    )(GLuint program);
void   (APIENTRY *glGetProgramInfoLog	    )(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
GLint  (APIENTRY *glGetUniformLocation	    )(GLuint program, const GLchar* name);
void   (APIENTRY *glBindBuffer		    )(GLenum target, GLuint buffer);
void   (APIENTRY *glEnableVertexAttribArray )(GLuint index);
void   (APIENTRY *glUniform4f		    )(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void   (APIENTRY *glUniformMatrix4fv	    )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void   (APIENTRY *glUseProgram		    )(GLuint program);
void   (APIENTRY *glValidateProgram	    )(GLuint program);
void   (APIENTRY *glGenBuffers		    )(GLsizei n, GLuint* buffers);
void   (APIENTRY *glBufferData		    )(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
void   (APIENTRY *glActiveTexture	    )(GLenum texture);
void   (APIENTRY *glVertexAttribPointer	    )(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
void   (APIENTRY *glUniform1i		    )(GLint location, GLint v0);
void   (APIENTRY *glUniform1f		    )(GLint location, GLfloat v0);
void   (APIENTRY *glFramebufferTexture2D    )(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void   (APIENTRY *glDrawBuffers             )(GLsizei n, const GLenum* bufs);
void   (APIENTRY *glBindFramebuffer         )(GLenum target, GLuint framebuffer);
void   (APIENTRY *glGenFramebuffers         )(GLsizei n, GLuint* framebuffers);
void   (APIENTRY *glDeleteFramebuffers      )(GLsizei n, const GLuint* framebuffers);
GLenum (APIENTRY *glCheckFramebufferStatus  )(GLenum target);
BOOL   (APIENTRY *wglChoosePixelFormatARB   )(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
BOOL   (APIENTRY *wglCreateContextAttribsARB)(HDC hdc, HGLRC context, const int *attributeList);
void   (APIENTRY *glGenVertexArrays)(GLsizei n,	GLuint *arrays);
void   (APIENTRY *glBindVertexArray)(GLuint array);
void   (APIENTRY *glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);

void InitialiseDCForGL(HDC hdc) {
	int attributes[] = {
		0x2001 /*WGL_DRAW_TO_WINDOW_ARB*/, GL_TRUE,
		0x2010 /*WGL_SUPPORT_OPENGL_ARB*/, GL_TRUE,
		0x2011 /*WGL_DOUBLE_BUFFER_ARB*/, GL_TRUE,
		0x2013 /*WGL_PIXEL_TYPE_ARB*/, 0x202B /*WGL_TYPE_RGBA_ARB*/,
		0x2014 /*WGL_COLOR_BITS_ARB*/, 32,
		0x2041 /*WGL_SAMPLE_BUFFERS_ARB*/, 1,
		// 0x2042 /*WGL_SAMPLES_ARB*/, 4,
		0
	};
	
	int format;
	unsigned int numberFormats;
	bool result = wglChoosePixelFormatARB(hdc, attributes, NULL, 1, &format, &numberFormats);
	Assert(numberFormats && result);
	PIXELFORMATDESCRIPTOR pfd;
	result = DescribePixelFormat(hdc, format, sizeof(pfd), &pfd);
	Assert(result);
	result = SetPixelFormat(hdc, format, &pfd);
	Assert(result);
}

unsigned LoadShader(char *shaderTextVertex, char *shaderTextFragment) {
	uint32_t shaderProgram = glCreateProgram();
	int success;
	
	{
		uint32_t shaderObject = glCreateShader(GL_VERTEX_SHADER);
		
		const char *p[1];
		p[0] = shaderTextVertex;
		int lengths[1];
		lengths[0] = strlen(shaderTextVertex);
		
		glShaderSource(shaderObject, 1, p, lengths);
		glCompileShader(shaderObject);
		
		glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);
		if (!success) {
			char compileOutput[1024];
			glGetShaderInfoLog(shaderObject, sizeof(compileOutput), NULL, compileOutput);
			MessageBoxA(NULL, compileOutput, "Shader compilation error", MB_ICONEXCLAMATION);
		
			return -1;
		}
		
		glAttachShader(shaderProgram, shaderObject);	
	}
	
	{
		uint32_t shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
		
		const char *p[1];
		p[0] = shaderTextFragment;
		int lengths[1];
		lengths[0] = strlen(shaderTextFragment);
		
		glShaderSource(shaderObject, 1, p, lengths);
		glCompileShader(shaderObject);
		
		glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);
		if (!success) {
			char compileOutput[1024];
			glGetShaderInfoLog(shaderObject, sizeof(compileOutput), NULL, compileOutput);
			MessageBoxA(NULL, compileOutput, "Shader compilation error", MB_ICONEXCLAMATION);
			
			return -1;
		}
		
		glAttachShader(shaderProgram, shaderObject);	
	}
	
	glLinkProgram(shaderProgram);
	
	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char compileOutput[1024];
		glGetProgramInfoLog(shaderProgram, sizeof(compileOutput), NULL, compileOutput);
		MessageBoxA(NULL, compileOutput, "Shader compilation error", MB_ICONEXCLAMATION);
		
		return -1;
	}
	
	glValidateProgram(shaderProgram);
	
	return shaderProgram;
}

void Transform(float *left, float *right) {
	float result[16];
	
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			float s = left[0 + i * 4] * right[j + 0 * 4]
				+ left[1 + i * 4] * right[j + 1 * 4]
				+ left[2 + i * 4] * right[j + 2 * 4]
				+ left[3 + i * 4] * right[j + 3 * 4];
			result[i * 4 + j] = s;
		}
	}
	
	memcpy(right, result, sizeof(result));
}

void Draw(Texture *texture, float x, float y, float w, float h,
		float sx, float sy, float sw, float sh, float alpha,
		float r, float g, float b, float rot) {
	if (sw == -1 && sh == -1) sw = texture->width, sh = texture->height;
	if (w == -1 && h == -1) w = sw, h = sh;
	
	if (x + w < 0 || y + h < 0 || x > destWidth || y > destHeight) return;
	
	x += destX;
	y += destY;
	y = destHeight - y - h;
	x -= destWidth / 2;
	y -= destHeight / 2;
	x *= 2.0f / destWidth;
	y *= 2.0f / destHeight;
	w *= 1.0f / destWidth;
	h *= 1.0f / destHeight; 
	x += w;
	y += h;
	
	sx /= texture->width;
	sy /= texture->height;
	sw /= texture->width;
	sh /= texture->height;
	
	if (destFlip) {
		sy = 1 - sy;
		sh = -sh;
	}
	
	glUseProgram(imageShader);
	float transform1[] = { w, 0, 0, 0, /**/ 0, h, 0, 0, /**/ 0, 0, 1, 0, /**/ 0, 0, 0, 1 };
	float transform2[] = { cosf(rot), -sinf(rot), 0, 0, /**/ sinf(rot), cosf(rot), 0, 0, /**/ 0, 0, 1, 0, /**/ 0, 0, 0, 1 };
	float transform3[] = { 1, 0, 0, x, /**/ 0, 1, 0, y, /**/ 0, 0, 1, 0, /**/ 0, 0, 0, 1, };
	Transform(transform2, transform1);
	Transform(transform3, transform1);
	float textureTransform[] = { sw, 0, 0, sx, /**/ 0, sh, 0, sy, /**/ 0, 0, 1, 0, /**/ 0, 0, 0, 1, };
	glUniformMatrix4fv(imageShaderTransform, 1, GL_TRUE, transform1);
	glUniformMatrix4fv(imageShaderTextureTransform, 1, GL_TRUE, textureTransform);
	glUniform4f(imageShaderBlendColor, r, g, b, alpha);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void CreateTexture(Texture *texture, uint8_t *data, size_t dataBytes) {
	IStream *stream = SHCreateMemStream(data, dataBytes);
	IWICBitmapDecoder *decoder;
	imagingFactory->CreateDecoderFromStream(stream, NULL, WICDecodeMetadataCacheOnLoad, &decoder);
	IWICBitmapFrameDecode *frame;
	decoder->GetFrame(0, &frame);
	UINT width, height;
	frame->GetSize(&width, &height);
	BYTE *bits = (BYTE *) malloc(width * height * 4);
	frame->CopyPixels(NULL, width * 4, width * height * 4, bits);
	texture->width = width, texture->height = height;
	glGenTextures(1, &texture->id);
	Assert(texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bits);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	free(bits);
	frame->Release();
	decoder->Release();
	stream->Release();
}

uint64_t randomSeed;

uint8_t GetRandomByte() {
	randomSeed = randomSeed * 214013 + 2531011;
	return (uint8_t) (randomSeed >> 16);
}

int musicIndex;

wchar_t *appData;

DWORD CALLBACK MusicThread(void *) {
	wchar_t str[4096];
	
	StringCopy(str, appData);
	StringAppend(str, L"\\__bgm1.mid");
	HANDLE h = CreateFileW(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, 0);
	DWORD w;
	WriteFile(h, bgm1_mid, bgm1_midBytes, &w, 0);
	CloseHandle(h);
	StringCopy(str, appData);
	StringAppend(str, L"\\__bgm2.mid");
	h = CreateFileW(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, 0);
	WriteFile(h, bgm2_mid, bgm2_midBytes, &w, 0);
	CloseHandle(h);
	StringCopy(str, appData);
	StringAppend(str, L"\\__bgm3.mid");
	h = CreateFileW(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, 0);
	WriteFile(h, bgm3_mid, bgm3_midBytes, &w, 0);
	CloseHandle(h);
	Sleep(2000);
	
	int m = musicIndex;
	goto start;
	
	while (true) {
		if (musicIndex != m) {
			m = musicIndex;
			mciSendString("close bgmtrack", 0, 0, 0);
			start:;
			StringCopy(str, L"open ");
			StringAppend(str, appData);
			if (m == 0) StringAppend(str, L"\\__bgm1.mid");
			if (m == 1) StringAppend(str, L"\\__bgm2.mid");
			if (m == 2) StringAppend(str, L"\\__bgm3.mid");
			StringAppend(str, L" type mpegvideo alias bgmtrack");
			mciSendStringW(str, 0, 0, 0);
			mciSendString("play bgmtrack repeat", 0, 0, 0);
		}
		
		Sleep(500);
	}
}

Texture textureWhite;

#include "game.cpp"

LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_DESTROY) {
		DoExit(0);
	} else if (message == WM_DPICHANGED) {
		RECT *newBounds = (RECT *) lParam;
		MoveWindow(window, newBounds->left, newBounds->top, newBounds->right - newBounds->left, newBounds->bottom - newBounds->top, TRUE);
	} else if (message == WM_KEYDOWN && wParam < 256) {
		keysDown[wParam] = true;
		if (~lParam & (1 << 30)) keysPressed[wParam] = 1;
	} else if (message == WM_KEYUP && wParam < 256) {
		keysDown[wParam] = false;
		keysReleased[wParam] = 1;
	} else {
		return DefWindowProc(window, message, wParam, lParam);
	}

	return 0;
}

void WinMainCRTStartup() {
	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, 0, &appData);
	
#ifndef DEVBUILD
	CreateThread(0, 0, MusicThread, 0, 0, 0);
#endif
	
	instance = GetModuleHandle(0);
	
	QueryPerformanceCounter((LARGE_INTEGER *) &randomSeed);
	
	CoInitialize(NULL);
	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void **) &imagingFactory);
	
	OSVERSIONINFOEXW version = { sizeof(version) };
	((LONG (*)(PRTL_OSVERSIONINFOEXW)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"))(&version);

	if (version.dwMajorVersion >= 10) {
		shcore = LoadLibrary("shcore.dll");
		SetProcessDpiAwarenessContextType setProcessDpiAwarenessContext 
			= (SetProcessDpiAwarenessContextType) GetProcAddress(LoadLibrary("user32.dll"), 
			"SetProcessDpiAwarenessContext");
		setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	} else {
		SetProcessDPIAware();
	}
	
	HMODULE crt = GetModuleHandle("msvcrt.dll");
	*(void **) &sinf = GetProcAddress(crt, "sinf");
	*(void **) &cosf = GetProcAddress(crt, "cosf");
	*(void **) &atan2f = GetProcAddress(crt, "atan2f");

	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProcedure;
	windowClass.lpszClassName = "frame";
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hIcon = LoadIcon(instance, MAKEINTRESOURCE(1));
	RegisterClassEx(&windowClass);
	RECT rect = {};
	rect.right = GAME_SIZE * DEFAULT_SCALE;
	rect.bottom = GAME_SIZE * DEFAULT_SCALE;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	root = CreateWindowEx(0, "frame", BRAND, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 
		rect.right - rect.left, rect.bottom - rect.top, 0, 0, 0, 0);
	
	{
		WNDCLASS dummyClass = {};
		dummyClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		dummyClass.lpfnWndProc = DefWindowProc;
		dummyClass.lpszClassName = "DummyClass";
		RegisterClass(&dummyClass);
		HWND dummy = CreateWindow(dummyClass.lpszClassName, "", 0, 0, 0, 0, 0, 0, 0, 0, 0);
		Assert(dummy);
		HDC hdc = GetDC(dummy);
		PIXELFORMATDESCRIPTOR pfd = {};
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.cColorBits = 32;
		pfd.cAlphaBits = 8;
		int format = ChoosePixelFormat(hdc, &pfd);
		Assert(format);
		SetPixelFormat(hdc, format, &pfd);
		HGLRC context = wglCreateContext(hdc);
		bool result = wglMakeCurrent(hdc, context);
		Assert(result);
		
		{
			#define LOADEXT(x) x = (decltype(x)) wglGetProcAddress(#x);Assert(x)
			LOADEXT(wglChoosePixelFormatARB);
			LOADEXT(wglCreateContextAttribsARB);
			LOADEXT(glCreateProgram);
			LOADEXT(glCreateShader);
			LOADEXT(glAttachShader);
			LOADEXT(glDrawBuffers);
			LOADEXT(glCheckFramebufferStatus);
			LOADEXT(glGenFramebuffers);
			LOADEXT(glGetShaderInfoLog);
			LOADEXT(glUniform1f);
			LOADEXT(glShaderSource);
			LOADEXT(glActiveTexture);
			LOADEXT(glUniformMatrix4fv);
			LOADEXT(glGetUniformLocation);
			LOADEXT(glBindBuffer);
			LOADEXT(glEnableVertexAttribArray);
			LOADEXT(glGetProgramInfoLog);
			LOADEXT(glUseProgram);
			LOADEXT(glValidateProgram);
			LOADEXT(glLinkProgram);	
			LOADEXT(glUniform4f);
			LOADEXT(glVertexAttribPointer);
			LOADEXT(glCompileShader);
			LOADEXT(glFramebufferTexture2D);
			LOADEXT(glGetShaderiv);
			LOADEXT(glGenBuffers);	
			LOADEXT(glBufferData);
			LOADEXT(glUniform1i);
			LOADEXT(glBindFramebuffer);
			LOADEXT(glDeleteFramebuffers);
			LOADEXT(glBindVertexArray);
			LOADEXT(glGenVertexArrays);
			LOADEXT(glFramebufferTexture);
		}
		
		wglMakeCurrent(hdc, 0);
		wglDeleteContext(context);
		ReleaseDC(dummy, hdc);
		DestroyWindow(dummy);
		
		dummy = CreateWindow(dummyClass.lpszClassName, "", 0, 0, 0, 0, 0, 0, 0, 0, 0);
		Assert(dummy);
		hdc = GetDC(dummy);
		InitialiseDCForGL(hdc);
		
		int glAttributes[] = {
			0x2091 /*WGL_CONTEXT_MAJOR_VERSION_ARB*/, 3,
			0x2092 /*WGL_CONTEXT_MINOR_VERSION_ARB*/, 3,
			0x9126 /*WGL_CONTEXT_PROFILE_MASK_ARB*/, 1,
			0
		};
		
		wglCreateContextAttribsARB(hdc, 0, glAttributes);
		renderingContext = wglCreateContext(hdc);
		Assert(renderingContext != INVALID_HANDLE_VALUE);
		dummyDC = hdc;
		wglMakeCurrent(dummyDC, renderingContext);
		
		unsigned vertexArrayID;
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_MULTISAMPLE);
		
		{
			float squareVBOArray[] = { -1, -1, 0.5f, 0, 1, /**/ -1, 1, 0.5f, 0, 0, /**/ 1, 1, 0.5f, 1, 0, /**/ 1, -1, 0.5f, 1, 1 };
			unsigned squareIBOArray[] = { 0, 1, 2, 0, 2, 3 };
			glGenBuffers(1, &squareVBO);
			glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(squareVBOArray), squareVBOArray, GL_STATIC_DRAW);
			glGenBuffers(1, &squareIBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareIBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIBOArray), squareIBOArray, GL_STATIC_DRAW);
			Assert(squareIBO && squareVBO);
			
			imageShader = LoadShader(
				R"Shader(#version 330
				layout (location = 0) in vec3 Position;
				layout (location = 1) in vec2 TexCoord;
				uniform mat4 transform;
				uniform mat4 textureTransform;
				out vec2 TexCoord0;
				void main() { gl_Position = transform * vec4(Position, 1.0);
				TexCoord0 = (textureTransform * vec4(TexCoord, 0.0, 1.0)).xy; })Shader",
				R"Shader(#version 330
				layout(location = 0) out vec4 FragColor;
				in vec2 TexCoord0;
				uniform sampler2D textureSampler;
				uniform vec4 blendColor;
				uniform int palette;
				void main() { 
					vec4 m = texture2D(textureSampler, TexCoord0.st);
					FragColor = m * blendColor; 
				})Shader");
			imageShaderTransform = glGetUniformLocation(imageShader, "transform");
			imageShaderTextureTransform = glGetUniformLocation(imageShader, "textureTransform");
			imageShaderBlendColor = glGetUniformLocation(imageShader, "blendColor");
			imageShaderTextureSampler = glGetUniformLocation(imageShader, "textureSampler");
			Assert(imageShader < INT_MAX && imageShaderTransform < INT_MAX && imageShaderTextureTransform < INT_MAX 
				&& imageShaderBlendColor < INT_MAX && imageShaderTextureSampler < INT_MAX);
		}
		
		mainDC = GetDC(root);
		InitialiseDCForGL(mainDC);
		Assert(wglMakeCurrent(mainDC, renderingContext));
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	unsigned frameBuffer = 0, frameBufferTexture = 0;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glGenTextures(1, &frameBufferTexture);
	glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GAME_SIZE, GAME_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frameBufferTexture, 0);
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	
	InitEmbed();
	InitialiseGame();
	
	CreateTexture(&textureWhite, white_png, white_pngBytes);
	
	uint64_t frequency, lastCount, cumulativeDelta = 0, perFrame;
	QueryPerformanceFrequency((LARGE_INTEGER *) &frequency);
	QueryPerformanceCounter((LARGE_INTEGER *) &lastCount);
	perFrame = frequency / 60;
	
	while (true) {
		MSG message;
	
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		
		POINT mouse;
		GetCursorPos(&mouse);
		ScreenToClient(root, &mouse);
		mouseX = mouse.x / DEFAULT_SCALE;
		mouseY = mouse.y / DEFAULT_SCALE;
		
		mouseDown = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
		
		uint64_t currentCount;
		QueryPerformanceCounter((LARGE_INTEGER *) &currentCount);
		cumulativeDelta += currentCount - lastCount;
		lastCount = currentCount;
		
		int max = 5;
		
		while (cumulativeDelta > perFrame && max--) {
			UpdateGame();
			cumulativeDelta -= perFrame;
		
			for (int i = 0; i < 256; i++) {
				if (keysPressed[i]) keysPressed[i]--;
				if (keysReleased[i]) keysReleased[i]--;
			}
		}
		
		{
			RECT client;
			GetClientRect(root, &client);
			
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
			glViewport(0, 0, GAME_SIZE, GAME_SIZE);
			destWidth = GAME_SIZE, destHeight = GAME_SIZE;
			destFlip = false;
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareIBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const GLvoid *) 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const GLvoid *) (3 * sizeof(float)));
			glUniform1i(imageShaderTextureSampler, 0);
			
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT); 
			Draw(&textureWhite, 0, 0, GAME_SIZE, GAME_SIZE, 0, 0, 1, 1, 1, 
				((backgroundColor >> 16) & 0xFF) / 255.0f, 
				((backgroundColor >> 8) & 0xFF) / 255.0f, 
				((backgroundColor >> 0) & 0xFF) / 255.0f);
			
			RenderGame();
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			Texture texture = { GAME_SIZE, GAME_SIZE, frameBufferTexture };
			destFlip = true;
			destWidth = client.right, destHeight = client.bottom;
			glViewport(0, 0, client.right, client.bottom); 
			Draw(&texture, client.right / 2 - client.bottom / 2, 0, client.bottom, client.bottom);
				
			SwapBuffers(mainDC);
			Sleep(5);
		}
	}
	
	DoExit(0);
}