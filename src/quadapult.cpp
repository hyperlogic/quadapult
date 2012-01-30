#include "quadapult.h"
#include <stdio.h>
#include <string>
#include <vector>
#include "sprite.h"
#include "render.h"
#include "texture.h"

std::vector<Sprite*> s_spriteVec;
std::vector<Texture*> s_textureVec;

void QUADAPULT_Init(const char* path)
{
    RenderInit();

    const int NUM_TEXTURES = 3;
    static const char* textureArray[NUM_TEXTURES + 1] = {"texture/happy.tga", "texture/sad.tga", "texture/t.tga", 0};

    Texture::SetSearchPath(path);
    for (int i = 0; textureArray[i] != 0; ++i)
    {
        Texture* texture = new Texture();
        texture->LoadFromFile(textureArray[i], Texture::FlipVertical);
        s_textureVec.push_back(texture);
    }

    const int NUM_SPRITES = 4000;
    s_spriteVec.reserve(NUM_SPRITES);
    for (int i = 0; i < NUM_SPRITES; ++i)
    {
        Sprite* sprite = new Sprite();
        sprite->SetColor(Vector4f(RandomScalar(0.0f, 1.0f), RandomScalar(0.0f, 1.0f),
                                  RandomScalar(0.0f, 1.0f), RandomScalar(0.0f, 1.0f)));
        sprite->SetPosition(Vector2f(RandomScalar(0.0f, 768.0f), RandomScalar(0.0f, 1024.0f)));
        sprite->SetDepth(RandomScalar(0.0f, 1.0f));
        sprite->SetSize(Vector2f(RandomScalar(10.0f, 20.0f), RandomScalar(0.1f, 10.0f)));
        sprite->SetTexture(s_textureVec[i % NUM_TEXTURES]);
        //sprite->SetTexture(s_textureVec[0]);
        s_spriteVec.push_back(sprite);
    }

    Matrixf proj = Matrixf::Ortho(0, 768, 1024, 0, -10, 10);
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

    // TODO: sort!

    // draw sorted sprites.
    const int numSprites = s_spriteVec.size();
    for (int i = 0; i < numSprites; ++i)
        s_spriteVec[i]->Draw();
}

void QUADAPULT_Shutdown()
{
    // TODO: unref textures
    // TODO: delete sprites
}
