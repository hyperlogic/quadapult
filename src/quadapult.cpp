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
#include "timer.h"

std::vector<const Sprite*> s_spriteVec;
std::vector<Texture*> s_textureVec;
SortAndSweep s_sortAndSweep;
NodeVecVec s_nodeVecVec;
Sprite* s_screenSprite = 0;

Graph* BuildGraph(Sprite* root)
{
	Graph* graph = new Graph(root);

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

/*
static bool SpriteCompare(Sprite* i, Sprite* j)
{
	return i->GetDepth() < j->GetDepth();
}
*/

void QUADAPULT_Init(const char* path)
{
//	if (!SortAndSweep::UnitTest())
//		exit(1);

    RenderInit();

    const int NUM_TEXTURES = 26; // TEXTURES!
    static const char* textureArray[NUM_TEXTURES + 1] = {
		"texture/a.tga", "texture/b.tga", "texture/c.tga", "texture/d.tga", "texture/e.tga", "texture/f.tga", "texture/g.tga", "texture/h.tga",
		"texture/i.tga", "texture/j.tga", "texture/k.tga", "texture/l.tga", "texture/m.tga", "texture/n.tga", "texture/o.tga", "texture/p.tga",
		"texture/q.tga", "texture/r.tga", "texture/s.tga", "texture/t.tga", "texture/u.tga", "texture/v.tga", "texture/w.tga", "texture/x.tga",
		"texture/y.tga", "texture/z.tga", 0};

	srand(10);
	//srand(12);

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
	const float SIZE = 30;

    const int NUM_SPRITES = 26;

	// special case this is meant to indicate the "screen"!
	s_screenSprite = new Sprite();
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

        sprite->SetColor(Vector4f(RandomScalar(0.0f, 1.0f), RandomScalar(0.0f, 1.0f), RandomScalar(0.0f, 1.0f), 0.9f));
        sprite->SetPosition(pos);
        sprite->SetSize(size);
        sprite->SetDepth(depth);
		//int texNum = RandomInt(0, NUM_TEXTURES-1);
		int texNum = i % NUM_TEXTURES;
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
}

static void DumpNodeVecVec()
{
    // nodeVecVec is topologically sorted.
    printf("nodeVecVec = [\n");
    NodeVecVec::iterator vecVecIter = s_nodeVecVec.begin();
    NodeVecVec::iterator vecVecEnd = s_nodeVecVec.end();
    for(; vecVecIter != vecVecEnd; ++vecVecIter)
    {
		NodeVec* v = (*vecVecIter);
		assert(v);
		printf("    [ ");
		NodeVec::iterator vecIter = v->begin();
		NodeVec::iterator vecEnd = v->end();
		for(; vecIter != vecEnd; ++vecIter)
		{
			printf("%s ", (*vecIter)->sprite->GetName().c_str());
		}
		printf("]\n");
    }
    printf("]\n");
}

static void StateSort(NodeVecVec& nodeVecVec, std::vector<const Sprite*>& spriteVec)
{
    // no sorting, just copy sprites from nodeVecVec to spriteVec
    NodeVecVec::iterator vecVecIter = nodeVecVec.begin();
    NodeVecVec::iterator vecVecEnd = nodeVecVec.end();
    for(; vecVecIter != vecVecEnd; ++vecVecIter)
    {
		NodeVec* v = (*vecVecIter);
		NodeVec::iterator vecIter = v->begin();
		NodeVec::iterator vecEnd = v->end();
		for(; vecIter != vecEnd; ++vecIter)
        {
            const Sprite* sprite = (*vecIter)->sprite;
            if (sprite != s_screenSprite)
                spriteVec.push_back(sprite);
        }
    }
}

void QUADAPULT_Update(float dt)
{
    // TODO: update sort and sweep. for moving sprites.
	//s_sortAndSweep.Dump();
	//s_sortAndSweep.DumpOverlaps();

    static int frameTimer = 0;
    TIMER_DEF(BuildGraph);
    TIMER_DEF(TSort);
    TIMER_DEF(StateSort);

    TIMER_START(BuildGraph);
	Graph* graph = BuildGraph(s_screenSprite);
    TIMER_STOP(BuildGraph, frameTimer);

    TIMER_START(TSort);
    graph->TSort(s_nodeVecVec);
    TIMER_STOP(TSort, frameTimer);

    TIMER_START(StateSort);
    StateSort(s_nodeVecVec, s_spriteVec);
    TIMER_STOP(StateSort, frameTimer);

    TIMER_REPORT(BuildGraph, frameTimer);
    TIMER_REPORT(TSort, frameTimer);
    TIMER_REPORT(StateSort, frameTimer);

// TODO: we leak a graph every frame!

    frameTimer++;

	//graph->Dump();

    //DumpNodeVecVec(s_nodeVecVec);

	//printf("numVecs = %lu\n", s_nodeVecVec.size());

    // Clean up s_nodeVecVec!
    NodeVecVec::iterator vecVecIter = s_nodeVecVec.begin();
    NodeVecVec::iterator vecVecEnd = s_nodeVecVec.end();
    for (; vecVecIter != vecVecEnd; ++vecVecIter)
		delete (*vecVecIter);
    s_nodeVecVec.clear();
}

void QUADAPULT_Draw()
{
    DrawSprites(&s_spriteVec[0], s_spriteVec.size());
}

void QUADAPULT_Shutdown()
{
    // TODO: unref textures
    // TODO: delete sprites
}
