#include "sortandsweep.h"
#include <string.h>
#include <float.h>

#include <list>
#include <set>

#include "sprite.h"  // for debugging

SortAndSweep::SortAndSweep()
{
	// setup sentinel
	AABB* sentinel = &m_aabbArray[0];
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
	sentinel->min.setMax();
	sentinel->max.setMin();
	sentinel->min.owner = 0;
	sentinel->max.owner = 0;
	sentinel->userPtr = 0;

	m_aabbArraySize = 1;
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
			printf("%.5f-%s ", pElem->value[i], pElem->isMax() ? ")" : "(");
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


void SortAndSweep::Insert(const AABB& aabbIn)
{
	// grab a new aabb off end of array
	// TODO: make a pool
	AABB* pAabb = &m_aabbArray[m_aabbArraySize];
	m_aabbArraySize++;

	// init fresh AABB
	pAabb->min.value[0] = aabbIn.min.value[0];
	pAabb->min.value[1] = aabbIn.min.value[1];
	pAabb->min.owner = pAabb;
	pAabb->min.setMin();
	pAabb->max.value[0] = aabbIn.max.value[0];
	pAabb->max.value[1] = aabbIn.max.value[1];
	pAabb->max.owner = pAabb;
	pAabb->max.setMax();

	pAabb->userPtr = aabbIn.userPtr;

	for (int i = 0; i < 2; ++i)
	{
		// search from start of list
		Elem* pElem = m_listHead[i];

		// insert min cell at position where pElem to first larger element.
		// assumes large sentinel value guards from falling of end of list.
		while (pElem->value[i] < pAabb->min.value[i])
			pElem = pElem->next[i];
		pAabb->min.prev[i] = pElem->prev[i];
		pAabb->min.next[i] = pElem;
		pElem->prev[i]->next[i] = &pAabb->min;
		pElem->prev[i] = &pAabb->min;

		// insert max cell in the same way.
		// Note: can continue searching from last pos, as list is sorted.
		// Also, assumes sentinel value is present.
		while (pElem->value[i] < pAabb->max.value[i])
			pElem = pElem->next[i];
		pAabb->max.prev[i] = pElem->prev[i];
		pAabb->max.next[i] = pElem;
		pElem->prev[i]->next[i] = &pAabb->max;
		pElem->prev[i] = &pAabb->max;
	}

	// now scan thru list and add overlap pairs for all objects that this AABB intersects.
	// This pair tracking could be incorperated into the loops above, but is done here to simplify code.
	for (Elem* pElem = m_listHead[0]; ;	pElem = pElem->next[0] )
	{
		if (pElem->owner == pAabb)
			continue;

		if (pElem->isMin())
		{
			if (pElem->value[0] > pAabb->max.value[0])
				break;
			if (AABBOverlap(pAabb, pElem->owner))
				AddOverlapPair(pAabb, pElem->owner);
		}
		else if (pElem->value[0] > pAabb->max.value[0])
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

	// Test
	const int numBoxes = 4;
	AABB box[numBoxes];
	box[0] = AABB(Vector2f(1, 2), Vector2f(4, 5), (void*)0);
	box[1] = AABB(Vector2f(3, 1), Vector2f(6, 4), (void*)1);
	box[2] = AABB(Vector2f(3, 1), Vector2f(6, 4), (void*)2);
	box[3] = AABB(Vector2f(5, 2), Vector2f(8, 5), (void*)3);
	box[4] = AABB(Vector2f(0, 0), Vector2f(9, 9), (void*)4);

	SortAndSweep ss;
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

	// check
	for (int i = 0; i < numChecks; ++i)
	{
		if (!checks[i].pass) {
			printf("checks[%d] failed!\n", i);
			return false;
		}
	}
	return true;
}
