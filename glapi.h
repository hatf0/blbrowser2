#pragma once
#include <Windows.h>
void *GetAnyGLFuncAddress(const char *name);
void initGL();
#define GLFUNC(returnType, convention, name, ...)         \
	typedef returnType (convention*name##Fn)(__VA_ARGS__); \
	extern name##Fn name;

GLFUNC(void, __stdcall, BL_glBindTexture, unsigned int target, unsigned int texture);
GLFUNC(void, __stdcall, BL_glGetTexLevelParameteriv, unsigned int target, int level, unsigned int pname, int *params);
GLFUNC(void, __stdcall, BL_glTexImage2D, unsigned int target, int level, int internalFormat, int width, int height, int border, unsigned int format, unsigned int type, const void* pixels);
GLFUNC(unsigned int, __stdcall, BL_glGetError, void);
GLFUNC(void, __stdcall, BL_glTexParameteri, unsigned int, unsigned int, int);
GLFUNC(void, __stdcall, BL_glEnable, unsigned int);
GLFUNC(void, __stdcall, BL_glTexSubImage2D, unsigned int, int, int, int, int, int, unsigned int, unsigned int, void*);
GLFUNC(const char*, __stdcall, BL_glGetString, unsigned int);
GLFUNC(void, __stdcall, BL_glGenerateMipmap, unsigned int);
GLFUNC(PROC, WINAPI, BL_wglGetProcAddress, LPCSTR bleh);
extern char* glVersion;
extern unsigned int glMajor;