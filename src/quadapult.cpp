#include "quadapult.h"
#include <stdio.h>
#include <string>
#include <vector>
#include "sprite.h"

#if defined DARWIN
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif defined IOS
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#endif

std::vector<Sprite*> s_spriteVec;

void QUADAPULT_Init()
{
    const int NUM_SPRITES = 1000;
    s_spriteVec.reserve(NUM_SPRITES);
    for (int i = 0; i < NUM_SPRITES; ++i)
    {
        Sprite* sprite = new Sprite();
        sprite->SetColor(Vector4f(RandomScalar(0.0f, 1.0f), RandomScalar(0.0f, 1.0f),
                                  RandomScalar(0.0f, 1.0f), RandomScalar(0.0f, 1.0f)));
        sprite->SetPosition(Vector2f(RandomScalar(0.0f, 800.0f), RandomScalar(0.0f, 600.0f)));
        sprite->SetDepth(RandomScalar(0.0f, 1.0f));
        s_spriteVec.push_back(sprite);
    }
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

}
