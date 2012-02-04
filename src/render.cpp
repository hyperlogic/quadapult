#include "render.h"
#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include "sprite.h"

unsigned int s_frameCount = 0;
bool s_dumpRenderInfo = false;
bool s_dumpExtensionInfo = false;

#ifdef DEBUG
// If there is a glError this outputs it along with a message to stderr.
// otherwise there is no output.
void GLErrorCheck(const char* message)
{
    GLenum val = glGetError();
    switch (val)
    {
    case GL_INVALID_ENUM:
        fprintf(stderr, "GL_INVALID_ENUM : %s\n", message);
        break;
    case GL_INVALID_VALUE:
        fprintf(stderr, "GL_INVALID_VALUE : %s\n", message);
        break;
    case GL_INVALID_OPERATION:
        fprintf(stderr, "GL_INVALID_OPERATION : %s\n", message);
        break;
#ifndef GL_VERSION_ES_CM_1_1
    case GL_STACK_OVERFLOW:
        fprintf(stderr, "GL_STACK_OVERFLOW : %s\n", message);
        break;
    case GL_STACK_UNDERFLOW:
        fprintf(stderr, "GL_STACK_UNDERFLOW : %s\n", message);
        break;
#endif
    case GL_OUT_OF_MEMORY:
        fprintf(stderr, "GL_OUT_OF_MEMORY : %s\n", message);
        break;
    case GL_NO_ERROR:
        break;
    }
}
#endif

// prints all available OpenGL extensions
static void _DumpExtensions()
{
    const GLubyte* extensions = glGetString(GL_EXTENSIONS);
    printf("extensions =\n");

    std::string str((const char *)extensions);
    size_t s = 0;
    size_t t = str.find_first_of(' ', s);
    while (t != std::string::npos)
    {
        printf("    %s\n", str.substr(s, t - s).c_str());
        s = t + 1;
        t = str.find_first_of(' ', s);
    }
}

void RenderInit()
{
    // print out gl version info
    if (true)
    {
        const GLubyte* version = glGetString(GL_VERSION);
        printf("OpenGL\n");
        printf("    version = %s\n", version);

        const GLubyte* vendor = glGetString(GL_VENDOR);
        printf("    vendor = %s\n", vendor);

        const GLubyte* renderer = glGetString(GL_RENDERER);
        printf("    renderer = %s\n", renderer);

#ifndef GL_VERSION_ES_CM_1_1
        const GLubyte* shadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
        printf("    shader language version = %s\n", shadingLanguageVersion);

        int maxTextureUnits;
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
        printf("    max texture units = %d\n", maxTextureUnits);
#endif
    }

    if (true)
        _DumpExtensions();
}

bool FileExists(const std::string& filename)
{
    //printf("searching for %s\n", filename.c_str());

    FILE* fp = fopen(filename.c_str(), "r");
    if (fp)
    {
        fclose(fp);
        return true;
    }
    return false;
}

// searchPath is a string of paths seperated by semi-colons
// filename is the basename of a file to find "tree.png"
// foundFilename is the full path of the found file.
// returns false if file cannot be found.
bool FindFileInSearchPath(const std::string& searchPath, const std::string& filename, std::string& foundFilename)
{
    std::string currentFilename;
    size_t start = 0;
    size_t end = searchPath.find_first_of(";", start);
    while (end != std::string::npos)
    {
        currentFilename = searchPath.substr(start, end - start);
        currentFilename.append(filename);
        if (FileExists(currentFilename))
        {
            foundFilename = currentFilename;
            return true;
        }

        start = end + 1;
        end = searchPath.find_first_of(";", start);
    }

    currentFilename = searchPath.substr(start, end - start);
    currentFilename.append(filename);
    if (FileExists(currentFilename))
    {
        foundFilename = currentFilename;
        return true;
    }

    return false;
}

bool IsPowerOfTwo(int number)
{
    return (number & (number - 1)) == 0;
}

void DrawSprites(const Sprite** sprites, size_t numSprites)
{
    glClearColor(0, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);

	GLuint curTex = -1;
	unsigned int numTextureBinds = 0;

#if 1
    // unbatched drawing
    for (unsigned int i = 0; i < numSprites; ++i)
    {
        const Sprite* sprite = sprites[i];
        ASSERT(sprite);
        const Texture* texture = sprite->GetTexture();
        ASSERT(texture);
        GLuint tex = texture->GetTexture();
        if (curTex != tex)
        {
            glBindTexture(GL_TEXTURE_2D, tex);
            curTex = tex;
            numTextureBinds++;
        }
        sprite->Draw();
    }
#else

	// batched drawing
	unsigned int batchSizeTotal = 0;
	unsigned int numBatches = 0;

    // for batched drawing.
    static std::vector<float> vertVec;
    static std::vector<uint8_t> colorVec;
    static std::vector<float> uvVec;
    static std::vector<uint16_t> indexVec;

    glBindTexture(GL_TEXTURE_2D, s_textureVec[0]->GetTexture());

    const int numSprites = s_nodeVec.size();
    for (int i = 1; i < numSprites; ++i)
    {
		const Sprite* sprite = s_nodeVec[i]->sprite;
		const Texture* texture = sprite->GetTexture();
		GLuint tex = texture->GetTexture();

		if (curTex != tex && indexVec.size() > 0)
		{
			// draw the current batch, and start the next.
			numTextureBinds++;
			numBatches++;
			batchSizeTotal += indexVec.size();

			// flush
            glBindTexture(GL_TEXTURE_2D, curTex);
            Sprite::DrawVecs(vertVec, colorVec, uvVec, indexVec);
		}
		curTex = tex;
		sprite->PushBack(vertVec, colorVec, uvVec, indexVec);
    }

    // draw the last batch
	if (indexVec.size() > 0)
	{
		numTextureBinds++;
		numBatches++;
		batchSizeTotal += indexVec.size();

		// flush
		glBindTexture(GL_TEXTURE_2D, curTex);
		Sprite::DrawVecs(vertVec, colorVec, uvVec, indexVec);
	}

	//printf("numBatches = %d\n", numBatches);
	//printf("avgBatchSize = %.2f\n", (double)batchSizeTotal / numBatches);

#endif

}
