#include "sortandsweep.h"
#include <string.h>
#include <float.h>

#include <list>
#include <set>

#include "sprite.h"  // for debugging

SortAndSweep::SortAndSweep()
{
    // init obj pool
    int i;
    for (i = 0; i < MAX_AABBS - 1; i++)
        m_pool[i].next = m_pool + i + 1;
    m_free = m_pool;
    m_numFree = MAX_AABBS;
    m_numUsed = 0;

	// setup sentinel
	AABB* sentinel = AllocAABB();
	for (int i = 0; i < 2; ++i)
	{
		sentinel->min.prev[i] = 0;
		sentinel->min.next[i] = &sentinel->max;
		sentinel->max.prev[i] = &sentinel->min;
		sentinel->max.next[i] = 0;
		sentinel->min.value[i] = -FLT_MAX;
		sentinel->max.value[i] = FLT_MAX;
		m_listHead[i] = &sentinel->min;
	}
	sentinel->min.SetMax();
	sentinel->max.SetMin();
	sentinel->userPtr = 0;
}

SortAndSweep::AABB* SortAndSweep::AllocAABB()
{
    // take from front of free list
    assert(m_free);
    AABB* aabb = m_free;
    m_free = m_free->next;
    if (m_free)
        m_free->prev = NULL;
    m_numFree--;

    // add to front of used list
    aabb->prev = NULL;
    aabb->next = m_used;
    if (m_used)
        m_used->prev = aabb;
    m_used = aabb;
    m_numUsed++;

    assert(m_numUsed + m_numFree == MAX_AABBS);

    return aabb;
}

void SortAndSweep::FreeAABB(AABB* aabb)
{
    assert(aabb);

    // remove from used list
    if (aabb->prev)
        aabb->prev->next = aabb->next;
    else
        m_used = aabb->next;
    if (aabb->next)
        aabb->next->prev = aabb->prev;
    m_numUsed--;

    // add to start of free list
    aabb->next = m_free;
    if (m_free)
        m_free->prev = aabb;
    m_free = aabb;
    m_numFree++;

    assert(m_numUsed + m_numFree == MAX_AABBS);
}

static bool AABBOverlap(const SortAndSweep::AABB* a, const SortAndSweep::AABB* b)
{
	if (a->max.value[0] < b->min.value[0] || a->min.value[0] > b->max.value[0])
		return false;
	if (a->max.value[1] < b->min.value[1] || a->min.value[1] > b->max.value[1])
		return false;
	return true;
}

void SortAndSweep::AddOverlapPair(const AABB* a, const AABB* b)
{
	m_overlapPairVec.push_back(OverlapPair(a, b));
}

void SortAndSweep::Dump() const
{
	printf("SortAndSweep::Dump()\n");
	for (int i = 0; i < 2; ++i)
	{
		printf("    [ ");
		const Elem* pElem = m_listHead[i];
		while (pElem) {
			printf("%.5f-%s ", pElem->value[i], pElem->IsMax() ? ")" : "(");
			pElem = pElem->next[i];
		}
		printf("]\n");
	}
}

void SortAndSweep::DumpOverlaps() const
{
	printf("SortAndSweep::DumpOverlaps()\n");

	for (int i = 0; i < (int)m_overlapPairVec.size(); ++i)
	{
		const OverlapPair& p = m_overlapPairVec[i];
		printf("    %s -> %s\n", ((Sprite*)(p.first->userPtr))->GetName().c_str(), ((Sprite*)(p.second->userPtr))->GetName().c_str());
	}
}

void SortAndSweep::Insert(AABB* aabb)
{
	for (int i = 0; i < 2; ++i)
	{
		// search from start of list
		Elem* pElem = m_listHead[i];

		// insert min cell at position where pElem to first larger element.
		// assumes large sentinel value guards from falling of end of list.
		while (pElem->value[i] < aabb->min.value[i])
			pElem = pElem->next[i];
		aabb->min.prev[i] = pElem->prev[i];
		aabb->min.next[i] = pElem;
		pElem->prev[i]->next[i] = &aabb->min;
		pElem->prev[i] = &aabb->min;

		// insert max cell in the same way.
		// Note: can continue searching from last pos, as list is sorted.
		// Also, assumes sentinel value is present.
		while (pElem->value[i] < aabb->max.value[i])
			pElem = pElem->next[i];
		aabb->max.prev[i] = pElem->prev[i];
		aabb->max.next[i] = pElem;
		pElem->prev[i]->next[i] = &aabb->max;
		pElem->prev[i] = &aabb->max;
	}

	// now scan thru list and add overlap pairs for all objects that this AABB intersects.
	// This pair tracking could be incorperated into the loops above, but is done here to simplify code.
	for (Elem* pElem = m_listHead[0]; ;	pElem = pElem->next[0] )
	{
		if (pElem->GetAABB() == aabb)
			continue;

		if (pElem->IsMin())
		{
			if (pElem->value[0] > aabb->max.value[0])
				break;
			if (AABBOverlap(aabb, pElem->GetAABB()))
				AddOverlapPair(aabb, pElem->GetAABB());
		}
		else if (pElem->value[0] > aabb->max.value[0])
			break;
	}
}

struct OverlapCheck
{
	void* a;
	void* b;
	bool pass;
};

bool SortAndSweep::UnitTest()
{
	printf("SortAndSweep::UnitTest\n");

	SortAndSweep ss;

	// Test
	const int numBoxes = 5;
	AABB* box[numBoxes];
    for (int i = 0; i < numBoxes; ++i)
        box[i] = ss.AllocAABB();

	box[0]->Set(Vector2f(1, 2), Vector2f(4, 5), (void*)0);
	box[1]->Set(Vector2f(3, 1), Vector2f(6, 4), (void*)1);
	box[2]->Set(Vector2f(3, 1), Vector2f(6, 4), (void*)2);
	box[3]->Set(Vector2f(5, 2), Vector2f(8, 5), (void*)3);
	box[4]->Set(Vector2f(0, 0), Vector2f(9, 9), (void*)4);

	for (int i = 0; i < numBoxes; ++i)
		ss.Insert(box[i]);

	const int numChecks = 5;
	static OverlapCheck checks[] = {{(void*)0, (void*)1, false},
									{(void*)0, (void*)2, false},
									{(void*)1, (void*)2, false},
									{(void*)1, (void*)3, false},
									{(void*)2, (void*)3, false},
									{(void*)4, (void*)0, false},
									{(void*)4, (void*)1, false},
									{(void*)4, (void*)2, false},
									{(void*)4, (void*)3, false}};

	for (int i = 0; i < (int)ss.m_overlapPairVec.size(); ++i)
	{
		OverlapPair& p = ss.m_overlapPairVec[i];
		for (int j = 0; j < numChecks; ++j)
		{
			if (!checks[i].pass &&
				(p.first->userPtr == checks[j].a && p.second->userPtr == checks[j].b) ||
				(p.first->userPtr == checks[j].b && p.second->userPtr == checks[j].a))
			{
				checks[i].pass = true;
			}
		}
	}

    bool result = true;

	// check
	for (int i = 0; i < numChecks; ++i)
	{
		if (!checks[i].pass) {
			printf("checks[%d] failed!\n", i);
			result = false;
            break;
		}
	}

    // clean up
    for (int i = 0; i < numBoxes; ++i)
        ss.FreeAABB(box[i]);

	return result;
}
