#include "quadapult.h"
#include <stdio.h>
#include <string>
#include <vector>
#include "sprite.h"
#include "render.h"
#include "texture.h"
#include "sortandsweep.h"

std::vector<Sprite*> s_spriteVec;
std::vector<Texture*> s_textureVec;
SortAndSweep s_sortAndSweep;

void QUADAPULT_Init(const char* path)
{
    RenderInit();

    const int NUM_TEXTURES = 3;
    static const char* textureArray[NUM_TEXTURES + 1] = {"texture/happy.tga", "texture/sad.tga", "texture/t.tga", 0};

	srand(12);

    Texture::SetSearchPath(path);
    for (int i = 0; textureArray[i] != 0; ++i)
    {
        Texture* texture = new Texture();
        texture->LoadFromFile(textureArray[i], Texture::FlipVertical);
        s_textureVec.push_back(texture);
    }

	const float WIDTH = 320.0f;
	const float HEIGHT = 480.0f;

    const int NUM_SPRITES = 2;
    s_spriteVec.reserve(NUM_SPRITES);
    for (int i = 0; i < NUM_SPRITES; ++i)
    {
        Sprite* sprite = new Sprite();

		Vector2f pos(RandomScalar(0.0f, WIDTH), RandomScalar(0.0f, HEIGHT));
		Vector2f size(Vector2f(RandomScalar(200.0f, 200.0f), RandomScalar(200.0f, 200.0f)));

        sprite->SetColor(Vector4f(RandomScalar(0.0f, 1.0f), RandomScalar(0.0f, 1.0f),
                                  RandomScalar(0.0f, 1.0f), RandomScalar(0.0f, 1.0f)));
        sprite->SetPosition(pos);
        sprite->SetSize(size);
        sprite->SetDepth(RandomScalar(0.0f, 1.0f));
        sprite->SetTexture(s_textureVec[i % NUM_TEXTURES]);
        s_spriteVec.push_back(sprite);

		s_sortAndSweep.Insert(Box(pos, pos + size, (void*)sprite));
    }

    Matrixf proj = Matrixf::Ortho(0, WIDTH, HEIGHT, 0, -10, 10);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(reinterpret_cast<float*>(&proj));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
}

void QUADAPULT_Update(float dt)
{

}

void QUADAPULT_Draw()
{
    glClearColor(0, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);

	GLuint curTex = -1;

	int numTextureBinds = 0;

    // draw unsorted sprites.
    const int numSprites = s_spriteVec.size();
    for (int i = 0; i < numSprites; ++i)
	{
		Sprite* sprite = s_spriteVec[i];
		const Texture* texture = sprite->GetTexture();
		GLuint tex = texture->GetTexture();
		if (curTex != tex)
		{
			glBindTexture(GL_TEXTURE_2D, tex);
			curTex = tex;
			numTextureBinds++;
		}
		s_spriteVec[i]->Draw();
	}
	//printf("numTextureBinds = %d\n", numTextureBinds);
}

void QUADAPULT_Shutdown()
{
    // TODO: unref textures
    // TODO: delete sprites
}
