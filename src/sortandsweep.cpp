#include "sortandsweep.h"
#include <string.h>
#include <float.h>

#include <list>
#include <set>
#include <utility>

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
	m_overlapPairSet.insert(OverlapPair(a, b));
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

    OverlapPairSet::const_iterator setIter = m_overlapPairSet.begin();
    OverlapPairSet::const_iterator setEnd = m_overlapPairSet.end();
	for (; setIter != setEnd; ++setIter)
	{
		const OverlapPair& p = *setIter;
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

static void MoveElement(int i, SortAndSweep::Elem* elem, SortAndSweep::Elem* dst)
{
    // Unlink element...
    elem->prev[i]->next[i] = elem->next[i];
    elem->next[i]->prev[i] = elem->prev[i];
    // ...and relink it _after_ the destination element
    elem->prev[i] = dst;
    elem->next[i] = dst->next[i];
    dst->next[i]->prev[i] = elem;
    dst->next[i] = elem;
}

void SortAndSweep::UpdateBounds(AABB* aabb)
{
    for (int i = 0; i < 2; i++)
    {
        Elem* min = &aabb->min;
        Elem* max = &aabb->max;
        Elem* t;

        // Try to move min element to the left.
        // Move the roaming pointer t left for as long as it points to elem with value larger than pMin’s.
        // While doing so, keep track of the update status of any AABBs passed over.
        for (t = min->prev[i]; min->value[i] < t->value[i]; t = t->prev[i])
        {
            if (t->IsMax() && AABBOverlap(aabb, t->GetAABB()) && !HasOverlapPair(aabb, t->GetAABB()))
                AddOverlapPair(aabb, t->GetAABB());
        }

        // If t moves from its original position, move min into new place
        if (t != min->prev[i])
            MoveElement(i, min, t);

        // Similarly to above, try to move max element to the right
        for (t = max->next[i]; max->value[i] > t->value[i]; t = t->next[i])
        {
            if (t->IsMin() && AABBOverlap(aabb, t->GetAABB()) && !HasOverlapPair(aabb, t->GetAABB()))
                AddOverlapPair(aabb, t->GetAABB());
        }

        if (t != max->next[i])
            MoveElement(i, max, t->next[i]);

        // Similarly to above, try to move min element to the right
        for (t = min->next[i]; min->value[i] > t->value[i]; t = t->next[i])
        {
            if (t->IsMax() && HasOverlapPair(aabb, t->GetAABB()))
                DeleteOverlapPair(aabb, t->GetAABB());
        }

        if (t != min->next[i])
            MoveElement(i, min, t->prev[i]);

        // Similarly to above, try to move max element to the left
        for (t = max->prev[i]; max->value[i] < t->value[i]; t = t->prev[i])
        {
            if (t->IsMin() && HasOverlapPair(aabb, t->GetAABB()))
                DeleteOverlapPair(aabb, t->GetAABB());
        }

        if (t != max->prev[i])
            MoveElement(i, max, t);
    }
}

bool SortAndSweep::HasOverlapPair(const AABB* a, const AABB* b) const
{
    return (m_overlapPairSet.find(OverlapPair(a, b)) != m_overlapPairSet.end() ||
            m_overlapPairSet.find(OverlapPair(b, a)) != m_overlapPairSet.end());
}

void SortAndSweep::DeleteOverlapPair(const AABB* a, const AABB* b)
{
    OverlapPairSet::iterator iter = m_overlapPairSet.find(OverlapPair(a, b));
    if (iter != m_overlapPairSet.end())
        m_overlapPairSet.erase(iter);
}

// UNIT TEST:

struct OverlapCheck
{
    SortAndSweep::AABB* a;
    SortAndSweep::AABB* b;
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

	const int numChecks = 9;
	static OverlapCheck checks[] = {{box[0], box[1]},
									{box[0], box[2]},
									{box[1], box[2]},
									{box[1], box[3]},
									{box[2], box[3]},
									{box[4], box[0]},
									{box[4], box[1]},
									{box[4], box[2]},
									{box[4], box[3]}};
    bool result = true;
    for (int i = 0; i < numChecks; ++i)
    {
        if (!ss.HasOverlapPair(checks[i].a, checks[i].b))
        {
            printf("FAIL\n");
            result = false;
            break;
        }
    }

    // clean up
    for (int i = 0; i < numBoxes; ++i)
        ss.FreeAABB(box[i]);

	return result;
}
