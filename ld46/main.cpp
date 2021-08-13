#include <windows.h> 
#include <GL/gl.h> 
#include <shellscalingapi.h>
#include <stdint.h>
#include <wincodec.h>
#include <shlwapi.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include "bin/embed.h"

void DoExit(int status) {
#if 0
	HANDLE h = CreateFile("_selfdel.bat", GENERIC_WRITE, 0, 0, 2, 0x80, 0);
	const char *deleteCommand = "timeout /t 1 /nobreak\ndel ld46.exe\ndel _selfdel.bat";
	DWORD bytesWritten;
	WriteFile(h, deleteCommand, strlen(deleteCommand), &bytesWritten, NULL);
	CloseHandle(h);
	ShellExecute(0, 0, "_selfdel.bat", 0, 0, SW_HIDE);
#endif
	
	ExitProcess(status);
}

uint8_t keysPressed[256], keysReleased[256], keysDown[256];

///////////////////////////////////////////////

#define COLOR_PALETTE_COUNT (5)
#define GAME_SIZE (160)

#define BRAND "Stoke"

struct Texture { unsigned width, height, id; };
void Draw(Texture *texture, float x, float y, float w = -1, float h = -1, float sx = 0, float sy = 0, float sw = -1, float sh = -1, float alpha = 1);
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
unsigned imageShader, imageShaderTransform, imageShaderTextureTransform, imageShaderBlendColor, imageShaderTextureSampler, imageShaderPalette;
unsigned squareIBO, squareVBO;
int destWidth, destHeight, destX, destY;
IWICImagingFactory *imagingFactory;
int currentPalette;
HMIDIOUT midiOut;

#pragma function(memset)
void *memset(void *s, int c, size_t n) {
	char *s2 = (char *) s;
	while (n--) *s2++ = c;
	return s;
}

#pragma function(memcpy)
void *memcpy(void *d, void *s, size_t n) {
	char *s2 = (char *) s, *d2 = (char *) d;
	while (n--) *d2++ = *s2++;
	return d;
}

#pragma optimize( "", off )
void *Copy(void *d, void *s, size_t n) {
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
void *memmove(void *d, void *s, size_t n) {
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

void InitialiseDCForGL(HDC hdc) {
	int attributes[] = {
		0x2001 /*WGL_DRAW_TO_WINDOW_ARB*/, GL_TRUE,
		0x2010 /*WGL_SUPPORT_OPENGL_ARB*/, GL_TRUE,
		0x2011 /*WGL_DOUBLE_BUFFER_ARB*/, GL_TRUE,
		0x2013 /*WGL_PIXEL_TYPE_ARB*/, 0x202B /*WGL_TYPE_RGBA_ARB*/,
		0x2014 /*WGL_COLOR_BITS_ARB*/, 32,
		0x2041 /*WGL_SAMPLE_BUFFERS_ARB*/, 1,
		0x2042 /*WGL_SAMPLES_ARB*/, 4,
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
		float sx, float sy, float sw, float sh, float alpha) {
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
	
	glUseProgram(imageShader);
	float transform1[] = { w, 0, 0, 0, /**/ 0, h, 0, 0, /**/ 0, 0, 1, 0, /**/ 0, 0, 0, 1 };
	// float transform2[] = { cosf(rot), -sinf(rot), 0, 0, /**/ sinf(rot), cosf(rot), 0, 0, /**/ 0, 0, 1, 0, /**/ 0, 0, 0, 1 };
	float transform3[] = { 1, 0, 0, x, /**/ 0, 1, 0, y, /**/ 0, 0, 1, 0, /**/ 0, 0, 0, 1, };
	// Transform(transform2, transform1);
	Transform(transform3, transform1);
	float textureTransform[] = { sw, 0, 0, sx, /**/ 0, sh, 0, sy, /**/ 0, 0, 1, 0, /**/ 0, 0, 0, 1, };
	glUniformMatrix4fv(imageShaderTransform, 1, GL_TRUE, transform1);
	glUniformMatrix4fv(imageShaderTextureTransform, 1, GL_TRUE, textureTransform);
	glUniform4f(imageShaderBlendColor, 1, 1, 1, alpha);
	glUniform1i(imageShaderPalette, currentPalette);
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
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

uint8_t *volatile currentSFX;
bool volatile resetSFX;

void PlaySFX(uint8_t *sfx) {
	currentSFX = sfx;
	resetSFX = true;
}

uint8_t *volatile bgmTrack = bgm1_mid;

#include "game.cpp"

LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_DESTROY) {
		DoExit(0);
	} else if (message == WM_DPICHANGED) {
		RECT *newBounds = (RECT *) lParam;
		MoveWindow(window, newBounds->left, newBounds->top, newBounds->right - newBounds->left, newBounds->bottom - newBounds->top, TRUE);
	} else if ((message == WM_SYSKEYDOWN || message == WM_KEYDOWN) && wParam < 256) {
		keysDown[wParam] = true;
		/* if (~lParam & (1 << 30)) */ keysPressed[wParam] = 2;
		if (wParam == 'P') currentPalette = (currentPalette + 1) % COLOR_PALETTE_COUNT;
	} else if ((message == WM_SYSKEYUP || message == WM_KEYUP) && wParam < 256) {
		keysDown[wParam] = false;
		keysReleased[wParam] = 2;
	} else {
		return DefWindowProc(window, message, wParam, lParam);
	}

	return 0;
}

#pragma pack(push, 1)
struct MIDIHeader {
	DWORD signature, size;
	WORD format, tracks, ticksPerQuarter;
};

struct MIDITrack {
	DWORD signature, length;
};

struct MIDIPlayback {
	uint8_t *position, previousCommand;
	DWORD time;
};
#pragma pack(pop)

DWORD MIDIReadVariable(uint8_t *&position) {
	DWORD value = 0;
	
	while (true) {
		uint32_t c = *position++;
		value = (value << 7) | (c & 0x7F);
		if (~c & 0x80) break;
	}
	
	return value;
}

DWORD SwapEndian32(DWORD in) {
	return    ((in & 0xFF000000) >> 24) 
		| ((in & 0x00FF0000) >> 8)
		| ((in & 0x0000FF00) << 8)
		| ((in & 0x000000FF) << 24);
}

WORD SwapEndian16(WORD in) {
	return    ((in & 0xFF00) >> 8)
		| ((in & 0x00FF) << 8);
}

void SleepAccurate(DWORD time) {
	if (!time) return;
	
	uint64_t start, end, frequency;
	QueryPerformanceCounter((LARGE_INTEGER *) &start);
	QueryPerformanceFrequency((LARGE_INTEGER *) &frequency);
	
	while (true) {
		QueryPerformanceCounter((LARGE_INTEGER *) &end);
		if ((end - start) * 1000000 / frequency >= time) break;
	}
}

void MIDIPlay(uint8_t *position, bool loop) {
	if (!midiOut) return;
	
	start:;
	MIDIHeader *header = (MIDIHeader *) position;
	position += sizeof(MIDIHeader);
	
	BYTE trackCount = SwapEndian16(header->tracks);
	MIDIPlayback tracks[16] = {};
	
	for (int i = 0; i < trackCount; i++) {
		MIDITrack *track = (MIDITrack *) position;
		position += sizeof(MIDITrack);
		tracks[i].position = position;
		position += SwapEndian32(track->length);
	}
	
	DWORD time = 0, clock = 500000;
	
	while (trackCount && bgmTrack == (uint8_t *) header) {
		int next = -1;
		DWORD nextAbsolute = INT_MAX;
		
		for (int i = 0; i < trackCount; i++) {
			uint8_t *position = tracks[i].position;
			DWORD absolute = MIDIReadVariable(position) + tracks[i].time;
			
			if (absolute < nextAbsolute) {
				next = i;
				nextAbsolute = absolute;
			}
		}
		
		Assert(next != -1);
		MIDIPlayback *track = tracks + next;
		
		DWORD delta = MIDIReadVariable(track->position);
		SleepAccurate(clock / SwapEndian16(header->ticksPerQuarter) * (delta + track->time - time));
		time = (track->time += delta);
			
		DWORD command = *track->position++;
		
		if (~command & 0x80) {
			command = track->previousCommand;
			track->position--;
		} else {
			track->previousCommand = command;
		}
		
		if ((command & 0xF0) != 0xF0) {
			DWORD data1 = *track->position++, data2 = 0;
			
			if ((command & 0xF0) != 0xC0 && (command & 0xF0) != 0xD0) {
				data2 = *track->position++;
			}
	
			midiOutShortMsg(midiOut, command | (data1 << 8) | (data2 << 16));
		} else if (command == 0xFF) {
			command = *track->position++;
			
			if (command == 0x2F) {
				trackCount--;
				tracks[next] = tracks[trackCount];
			} else if (command == 0x51) {
				BYTE length = *track->position++;
				DWORD b1 = *track->position++, b2 = *track->position++, b3 = *track->position++;
				clock = (b1 << 16) | (b2 << 8) | b3;
			} else {
				BYTE length = *track->position++;
				track->position += length;
			}
		} else {
			Assert(false);
		}
	}
	
	if (loop && bgmTrack == (uint8_t *) header) {
		position = (uint8_t *) header;
		goto start;
	}
	
	midiOutClose(midiOut);
	midiOutOpen(&midiOut, 0, 0, 0, CALLBACK_NULL);
}

DWORD WINAPI AudioThread(void *) {
	while (true) {
		MIDIPlay(bgmTrack, true);
	}
	return 0;
}

DWORD WINAPI SFXThread(void *) {
	IMMDeviceEnumerator *deviceEnumerator;
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **) &deviceEnumerator);
	if (hr != S_OK) return 0;
	IMMDevice *device;
	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
	if (hr != S_OK) return 0;
	IAudioClient *client;
	hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void **) &client);
	if (hr != S_OK) return 0;
	device->Release();
	
	WAVEFORMATEX format = {};
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = 44100;
	format.wBitsPerSample = 16;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	
	client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
		20000000, 0, &format, 0);	
	IAudioRenderClient *render;
	client->GetService(__uuidof(IAudioRenderClient), (void **) &render);
	uint32_t bufferSize;
	client->GetBufferSize(&bufferSize);
	client->Start();
	
	int sampleIndex = 0;
	uint8_t *previousSamples = nullptr;
	
	while (true) {
		uint32_t padding;
		client->GetCurrentPadding(&padding);
		int32_t count = 44100 / 30 - padding;
		if (count <= 44100 / 60) continue;
		int16_t *buffer;
		render->GetBuffer(count, (BYTE **) &buffer);
		
		uint8_t *samples = currentSFX + 0x2C;
		if (previousSamples != samples || resetSFX) sampleIndex = 0;
		resetSFX = false;
		previousSamples = samples;
		DWORD sampleCount = 0;
		if (samples != (uint8_t *) 0x2C) sampleCount = ((DWORD *) samples)[-1];
		
		for (int i = 0; i < count; i++) {
			int sampleIndex2 = sampleIndex / 5.5125 + 0.5; // "High-quality" sample rate conversion.
			int16_t value = sampleIndex2 >= sampleCount ? 0 
				: ((int16_t) ((uint16_t) samples[sampleIndex2 >= sampleCount ? sampleCount - 1 : sampleIndex2] * 256 - 32768));
			*buffer++ = value, *buffer++ = value;
			sampleIndex++;
		}
		
		render->ReleaseBuffer(count, 0);
	}
	
	return 0;
}

void WinMainCRTStartup() {
	instance = GetModuleHandle(0);
	
	QueryPerformanceCounter((LARGE_INTEGER *) &randomSeed);
	
	CoInitialize(NULL);
	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void **) &imagingFactory);
	
	if (MMSYSERR_NOERROR != midiOutOpen(&midiOut, 0, 0, 0, CALLBACK_NULL)) {
		midiOut = NULL;
	}
	
	CreateThread(0, 0, AudioThread, 0, 0, 0);
	CreateThread(0, 0, SFXThread, 0, 0, 0);
	
	OSVERSIONINFOEXW version = { sizeof(version) };
	((LONG (*)(PRTL_OSVERSIONINFOEXW)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"))(&version);

	if (version.dwMajorVersion >= 10) {
		shcore = LoadLibrary("shcore.dll");
		SetProcessDpiAwarenessContextType setProcessDpiAwarenessContext = (SetProcessDpiAwarenessContextType) GetProcAddress(LoadLibrary("user32.dll"), 
			"SetProcessDpiAwarenessContext");
		setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	} else {
		SetProcessDPIAware();
	}

	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProcedure;
	windowClass.lpszClassName = "frame";
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hIcon = LoadIcon(instance, MAKEINTRESOURCE(1));
	RegisterClassEx(&windowClass);
	RECT rect = {};
	rect.right = GAME_SIZE * 4;
	rect.bottom = GAME_SIZE * 4;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	root = CreateWindowEx(0, "frame", BRAND, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, 0, 0, 0, 0);
	
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
		
		glClearColor(0, 0, 0, 1);
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
					if (m.a > 0.1 && palette == 2) {
						if (m.g < 0.3) {
							m = vec4(45./255., 27./255., 0./255., 1);
						} else if (m.g < 0.6) {
							m = vec4(30./255., 96./255., 110./255., 1);
						} else if (m.g < 0.7) {
							m = vec4(90./255., 185./255., 168./255., 1);
						} else {
							m = vec4(196./255., 240./255., 194./255., 1);
						}
					} else if (m.a > 0.1 && palette == 0) {
						if (m.g < 0.3) {
							m = vec4(44./255., 33./255., 55./255., 1);
						} else if (m.g < 0.6) {
							m = vec4(118./255., 68./255., 98./255., 1);
						} else if (m.g < 0.7) {
							m = vec4(169./255., 104./255., 104./255., 1);
						} else {
							m = vec4(237./255., 180./255., 161./255., 1);
						}
					} else if (m.a > 0.1 && palette == 3) {
						if (m.g < 0.3) {
							m = vec4(51./255., 44./255., 80./255., 1);
						} else if (m.g < 0.6) {
							m = vec4(70./255., 135./255., 143./255., 1);
						} else if (m.g < 0.7) {
							m = vec4(148./255., 227./255., 68./255., 1);
						} else {
							m = vec4(226./255., 243./255., 228./255., 1);
						}
					} else if (m.a > 0.1 && palette == 4) {
						if (m.g < 0.3) {
							m = vec4(1./255., 40./255., 36./255., 1);
						} else if (m.g < 0.6) {
							m = vec4(38./255., 89./255., 53./255., 1);
						} else if (m.g < 0.7) {
							m = vec4(255./255., 77./255., 109./255., 1);
						} else {
							m = vec4(252./255., 222./255., 234./255., 1);
						}
					}
					FragColor = m * blendColor; 
				})Shader");
			imageShaderTransform = glGetUniformLocation(imageShader, "transform");
			imageShaderTextureTransform = glGetUniformLocation(imageShader, "textureTransform");
			imageShaderBlendColor = glGetUniformLocation(imageShader, "blendColor");
			imageShaderTextureSampler = glGetUniformLocation(imageShader, "textureSampler");
			imageShaderPalette = glGetUniformLocation(imageShader, "palette");
			Assert(imageShader < INT_MAX && imageShaderTransform < INT_MAX && imageShaderTextureTransform < INT_MAX 
				&& imageShaderBlendColor < INT_MAX && imageShaderTextureSampler < INT_MAX && imageShaderPalette < INT_MAX);
		}
		
		mainDC = GetDC(root);
		InitialiseDCForGL(mainDC);
		Assert(wglMakeCurrent(mainDC, renderingContext));
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	InitialiseGame();
	
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
		
		uint64_t currentCount;
		QueryPerformanceCounter((LARGE_INTEGER *) &currentCount);
		cumulativeDelta += currentCount - lastCount;
		lastCount = currentCount;
		
		while (cumulativeDelta > perFrame) {
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
			glViewport(0, 0, client.right, client.bottom); 
			destWidth = GAME_SIZE * client.right / client.bottom, destHeight = GAME_SIZE;
			destX = destWidth / 2 - GAME_SIZE / 2, destY = 0;
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareIBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const GLvoid *) 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const GLvoid *) (3 * sizeof(float)));
			glUniform1i(imageShaderTextureSampler, 0);
			glClear(GL_COLOR_BUFFER_BIT); 
			
			RenderGame();
				
			SwapBuffers(mainDC);
		}
	}
	
	DoExit(0);
}