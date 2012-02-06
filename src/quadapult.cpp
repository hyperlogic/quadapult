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

typedef std::vector<const Sprite*> SpriteVec;
SpriteVec s_spriteVec;
std::vector<Texture*> s_textureVec;
SortAndSweep s_sortAndSweep;
NodeVecVec s_nodeVecVec;
Sprite* s_screenSprite = 0;

bool s_useBatching = true;
bool s_useGraph = true;
bool s_useStateSort = true;

typedef std::map<const Texture*, SpriteVec> TexMap;
TexMap s_texMap;

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

void QUADAPULT_Init(const char* path)
{
	if (!SortAndSweep::UnitTest())
		exit(1);

    RenderInit();

    const int NUM_TEXTURES = 10;
    static const char* textureArray[NUM_TEXTURES] = {
		"texture/a.tga", "texture/b.tga", "texture/c.tga", "texture/d.tga", "texture/e.tga",
        "texture/f.tga", "texture/g.tga", "texture/h.tga", "texture/i.tga", "texture/j.tga", };
/*
        "texture/k.tga", "texture/l.tga", "texture/m.tga", "texture/n.tga", "texture/o.tga",
        "texture/p.tga", "texture/q.tga", "texture/r.tga", "texture/s.tga", "texture/t.tga",
        "texture/u.tga", "texture/v.tga", "texture/w.tga", "texture/x.tga",	"texture/y.tga",
        "texture/z.tga"};
*/

	srand(10);
	//srand(12);

    Texture::SetSearchPath(path);
    for (int i = 0; i < NUM_TEXTURES; ++i)
    {
        Texture* texture = new Texture();
        texture->SetMinFilter(GL_LINEAR);
        texture->SetMagFilter(GL_LINEAR);
        texture->SetSWrap(GL_CLAMP_TO_EDGE);
        texture->SetTWrap(GL_CLAMP_TO_EDGE);
        bool ret = texture->LoadFromFile(textureArray[i], Texture::FlipVertical);
        assert(ret);
        s_textureVec.push_back(texture);
    }

    // init the s_texMap (used by StateSort)
    for (unsigned int i = 0; i < s_textureVec.size(); ++i)
    {
        const Texture* texture = s_textureVec[i];
        assert(texture);
        s_texMap[texture] = SpriteVec();
    }

	const float WIDTH = 320.0f;
	const float HEIGHT = 480.0f;
	const float SIZE = 5;//30;

    const int NUM_SPRITES = 1000;

	// special case this is meant to indicate the "screen"!
	s_screenSprite = new Sprite();
	Vector2f pos(RandomScalar(0.0f, WIDTH - SIZE), RandomScalar(0.0f, HEIGHT - SIZE));
	Vector2f size(Vector2f(SIZE, SIZE));
	float depth = -FLT_MAX;
	s_screenSprite->SetPosition(pos);
	s_screenSprite->SetSize(size);
	s_screenSprite->SetDepth(depth);
	s_screenSprite->SetName("screen");

	SortAndSweep::AABB* screenBox = s_sortAndSweep.AllocAABB();
    screenBox->Set(Vector2f(0, 0), Vector2f(WIDTH, HEIGHT), reinterpret_cast<void*>(s_screenSprite));
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

		SortAndSweep::AABB* spriteBox = s_sortAndSweep.AllocAABB();
        spriteBox->Set(pos, pos + size, reinterpret_cast<void*>(sprite));
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

static void NoStateSort(NodeVecVec& nodeVecVec, std::vector<const Sprite*>& spriteVec)
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

static void TextureStateSort(NodeVecVec& nodeVecVec, std::vector<const Sprite*>& spriteVec)
{
    // use the nodeVecVec and state sort each sub vec by texture.
    NodeVecVec::iterator vecVecIter = nodeVecVec.begin();
    NodeVecVec::iterator vecVecEnd = nodeVecVec.end();
    for(; vecVecIter != vecVecEnd; ++vecVecIter)
    {
        // Each NodeVec can be sorted by texture.
        // iterate over each sprite and insert into s_texMap.
		NodeVec* v = (*vecVecIter);
		NodeVec::iterator vecIter = v->begin();
		NodeVec::iterator vecEnd = v->end();
		for(; vecIter != vecEnd; ++vecIter)
        {
            const Sprite* sprite = (*vecIter)->sprite;
            if (sprite != s_screenSprite)
            {
                TexMap::iterator mapIter = s_texMap.find(sprite->GetTexture());
                assert(mapIter != s_texMap.end());
                mapIter->second.push_back(sprite);
            }
        }

        // Now iterate over s_texMap and insert into spriteVec
        TexMap::iterator mapIter = s_texMap.begin();
        TexMap::iterator mapEnd = s_texMap.end();
        for (; mapIter != mapEnd; ++mapIter)
        {
            SpriteVec::iterator sVecIter = mapIter->second.begin();
            SpriteVec::iterator sVecEnd = mapIter->second.end();
            for (; sVecIter != sVecEnd; ++sVecIter)
                spriteVec.push_back(*sVecIter);
        }

        // clean s_texMap
        mapIter = s_texMap.begin();
        for (; mapIter != mapEnd; ++mapIter)
            mapIter->second.clear();
    }
}

static bool SpriteCompare(const Sprite* i, const Sprite* j)
{
	return i->GetDepth() < j->GetDepth();
}

static void UpdateGraph()
{
    // TODO: update sort and sweep. for moving sprites.

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
    if (s_useStateSort)
        TextureStateSort(s_nodeVecVec, s_spriteVec);
    else
        NoStateSort(s_nodeVecVec, s_spriteVec);
    delete graph;
    TIMER_STOP(StateSort, frameTimer);

    TIMER_REPORT(BuildGraph, frameTimer);
    TIMER_REPORT(TSort, frameTimer);
    TIMER_REPORT(StateSort, frameTimer);

    frameTimer++;

    // Clean up s_nodeVecVec!
    NodeVecVec::iterator vecVecIter = s_nodeVecVec.begin();
    NodeVecVec::iterator vecVecEnd = s_nodeVecVec.end();
    for (; vecVecIter != vecVecEnd; ++vecVecIter)
		delete (*vecVecIter);
    s_nodeVecVec.clear();
}

void QUADAPULT_Update(float dt)
{
    if (s_useGraph)
    {
        UpdateGraph();
    }
    else
    {
        sort(s_spriteVec.begin(), s_spriteVec.end(), SpriteCompare);
    }
}

void QUADAPULT_Draw()
{
    if (s_useGraph)
    {
        if (s_useBatching)
            DrawSpritesBatched(&s_spriteVec[0], s_spriteVec.size());
        else
            DrawSprites(&s_spriteVec[0], s_spriteVec.size());
        s_spriteVec.clear();
    }
    else
    {
        if (s_useBatching)
            DrawSpritesBatched(&s_spriteVec[1], s_spriteVec.size() - 1);
        else
            DrawSprites(&s_spriteVec[0], s_spriteVec.size());
    }
}

void QUADAPULT_Shutdown()
{
    // TODO: unref textures
    // TODO: delete sprites
}
