#ifndef __RENDER_H__
#define __RENDER_H__

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#include "abaci.h"

#if defined DARWIN

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#elif TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#else

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#endif

#include <cassert>
#include <string>
#include <vector>

void RenderInit();

bool FileExists(const std::string& filename);
bool FindFileInSearchPath(const std::string& searchPath, const std::string& filename, std::string& foundFilename);
bool IsPowerOfTwo(int number);

class Sprite;
void DrawSprites(const Sprite** sprites, size_t numSprites);

#ifdef DEBUG
#define GL_ERROR_CHECK(x) GLErrorCheck(x)
#define ASSERT(x) assert(x)
void GLErrorCheck(const char* message);
#else
#define GL_ERROR_CHECK(x)
#define ASSERT(x)
#endif

#endif
