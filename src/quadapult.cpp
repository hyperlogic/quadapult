#include "quadapult.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <float.h>
#include "sprite.h"
#include "render.h"
#include "texture.h"
#include "sortandsweep.h"
#include "graph.h"
#include "assert.h"

std::vector<Sprite*> s_spriteVec;
std::vector<Texture*> s_textureVec;
SortAndSweep s_sortAndSweep;

Graph* BuildGraph()
{
	Graph* graph = new Graph();

	SortAndSweep::OverlapPairVec::const_iterator pairIter = s_sortAndSweep.GetOverlapPairVec().begin();
	SortAndSweep::OverlapPairVec::const_iterator pairEnd = s_sortAndSweep.GetOverlapPairVec().end();
	for (; pairIter != pairEnd; ++pairIter)
	{
		const Sprite* a = reinterpret_cast<const Sprite*>(pairIter->first->userPtr);
		const Sprite* b = reinterpret_cast<const Sprite*>(pairIter->second->userPtr);
		assert(a && b);

		if (a->GetDepth() < b->GetDepth())
			graph->AddEdge(a, b);
		else
			graph->AddEdge(b, a);
	}

	return graph;
}

void QUADAPULT_Init(const char* path)
{
    RenderInit();
    srand(1998);

    const int NUM_TEXTURES = 3;
    static const char* textureArray[NUM_TEXTURES + 1] = {"texture/happy.tga", "texture/sad.tga", "texture/t.tga", 0};

	srand(10);

    Texture::SetSearchPath(path);
    for (int i = 0; textureArray[i] != 0; ++i)
    {
        Texture* texture = new Texture();
        texture->SetMinFilter(GL_LINEAR);
        texture->SetMagFilter(GL_LINEAR);
        texture->SetSWrap(GL_CLAMP_TO_EDGE);
        texture->SetTWrap(GL_CLAMP_TO_EDGE);
        texture->LoadFromFile(textureArray[i], Texture::FlipVertical);
        s_textureVec.push_back(texture);
    }

	const float WIDTH = 320.0f;
	const float HEIGHT = 480.0f;
	const float SIZE = 200;
    const int NUM_SPRITES = 2;

	// special case this is meant to indicate the "screen"!
	Sprite* s_screenSprite = new Sprite();
	Vector2f pos(RandomScalar(0.0f, WIDTH - SIZE), RandomScalar(0.0f, HEIGHT - SIZE));
	Vector2f size(Vector2f(SIZE, SIZE));
	float depth = -FLT_MAX;
	s_screenSprite->SetPosition(pos);
	s_screenSprite->SetSize(size);
	s_screenSprite->SetDepth(depth);
	s_screenSprite->SetName("screen");
	SortAndSweep::AABB screenBox(Vector2f(0, 0), Vector2f(WIDTH, HEIGHT), reinterpret_cast<void*>(s_screenSprite));
	s_sortAndSweep.Insert(screenBox);

    s_spriteVec.reserve(NUM_SPRITES);
    for (int i = 0; i < NUM_SPRITES; ++i)
    {
        Sprite* sprite = new Sprite();

		Vector2f pos(RandomScalar(0.0f, WIDTH - SIZE), RandomScalar(0.0f, HEIGHT - SIZE));
		Vector2f size(Vector2f(SIZE, SIZE));
		float depth = RandomScalar(0.0f, 1.0f);

        sprite->SetColor(Vector4f(RandomScalar(0.0f, 1.0f), RandomScalar(0.0f, 1.0f), RandomScalar(0.0f, 1.0f), 1.0f));
        sprite->SetPosition(pos);
        sprite->SetSize(size);
        sprite->SetDepth(depth);
        //sprite->SetTexture(s_textureVec[i % NUM_TEXTURES]);
		int texNum = RandomInt(0, NUM_TEXTURES-1);
		sprite->SetTexture(s_textureVec[texNum]);
		char temp[512];
		sprintf(temp, "%d:%s[%.3f]", i, textureArray[texNum], depth);
		sprite->SetName(temp);

        s_spriteVec.push_back(sprite);

		SortAndSweep::AABB spriteBox(pos, pos + size, reinterpret_cast<void*>(sprite));
		s_sortAndSweep.Insert(spriteBox);
    }

	printf("m_overlapVec.size = %lu\n", s_sortAndSweep.GetOverlapPairVec().size());

    Matrixf proj = Matrixf::Ortho(0, WIDTH, HEIGHT, 0, -10, 10);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(reinterpret_cast<float*>(&proj));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

	s_sortAndSweep.Dump();

	// Build overlap graph!
	Graph* graph = BuildGraph();
	graph->Dump();
}

void QUADAPULT_Update(float dt)
{

}

void QUADAPULT_Draw()
{
    glClearColor(0, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);

	GLuint curTex = -1;
	unsigned int numTextureBinds = 0;

#if 0

    // unbatched drawing
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

    const int numSprites = s_spriteVec.size();
    for (int i = 0; i < numSprites; ++i)
    {
		Sprite* sprite = s_spriteVec[i];
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
		s_spriteVec[i]->PushBack(vertVec, colorVec, uvVec, indexVec);
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

	//printf("numTextureBinds = %d\n", numTextureBinds);
}

void QUADAPULT_Shutdown()
{
    // TODO: unref textures
    // TODO: delete sprites
}
