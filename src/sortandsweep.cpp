#include "sortandsweep.h"
#include <string.h>
#include <float.h>

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

	m_aabbArraySize = 1;
}

static bool AABBOverlap(const SortAndSweep::AABB* a, const SortAndSweep::AABB* b)
{
	/*
	printf("check overlap\n");
	printf("  a = (%.5f, %.5f) (%.5f, %.5f)\n", a->min.value[0], a->min.value[1], a->max.value[0], a->max.value[1]);
	printf("  b = (%.5f, %.5f) (%.5f, %.5f)\n", b->min.value[0], b->min.value[1], b->max.value[0], b->max.value[1]);
	*/

	return (a->max.value[0] > b->min.value[0] && b->min.value[0] < a->max.value[0] &&
			a->max.value[1] > b->min.value[1] && b->min.value[1] < a->max.value[1]);
}

static void AddCollisionPair(const SortAndSweep::AABB* a, const SortAndSweep::AABB* b)
{
	printf("overlap!\n");
	printf("  a = (%.5f, %.5f) (%.5f, %.5f)\n", a->min.value[0], a->min.value[1], a->max.value[0], a->max.value[1]);
	printf("  b = (%.5f, %.5f) (%.5f, %.5f)\n", b->min.value[0], b->min.value[1], b->max.value[0], b->max.value[1]);
}

void SortAndSweep::Dump() const
{
	printf("SortAndSweepDump()\n");
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

void SortAndSweep::Insert(const Box& box)
{
	// grab a new aabb off end of array
	// TODO: pool
	AABB* pAabb = &m_aabbArray[m_aabbArraySize];
	m_aabbArraySize++;

	memset(pAabb, 0, sizeof(AABB));

	pAabb->min.owner = pAabb;
	pAabb->min.value[0] = box.min[0];
	pAabb->min.value[1] = box.min[1];
	pAabb->min.setMin();

	pAabb->max.owner = pAabb;
	pAabb->max.value[0] = box.max[0];
	pAabb->max.value[1] = box.max[1];
	pAabb->max.setMax();

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
				AddCollisionPair(pAabb, pElem->owner);
		}
		else if (pElem->value[0] > pAabb->min.value[0])
			break;

	}
}
