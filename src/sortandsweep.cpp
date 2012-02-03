#include "sortandsweep.h"
#include <string.h>
#include <float.h>

#include <list>
#include <set>

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
	/*
	printf("check overlap\n");
	printf("  a = (%.5f, %.5f) (%.5f, %.5f)\n", a->min.value[0], a->min.value[1], a->max.value[0], a->max.value[1]);
	printf("  b = (%.5f, %.5f) (%.5f, %.5f)\n", b->min.value[0], b->min.value[1], b->max.value[0], b->max.value[1]);
	*/

	return (a->max.value[0] > b->min.value[0] && b->min.value[0] < a->max.value[0] &&
			a->max.value[1] > b->min.value[1] && b->min.value[1] < a->max.value[1]);
}

void SortAndSweep::AddOverlapPair(const AABB* a, const AABB* b)
{

	printf("overlap! %p, %p\n", a, b);
	printf("  a = (%.5f, %.5f) (%.5f, %.5f)\n", a->min.value[0], a->min.value[1], a->max.value[0], a->max.value[1]);
	printf("  b = (%.5f, %.5f) (%.5f, %.5f)\n", b->min.value[0], b->min.value[1], b->max.value[0], b->max.value[1]);

	m_overlapPairVec.push_back(OverlapPair(a, b));
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
		else if (pElem->value[0] > pAabb->min.value[0])
			break;

	}
}

void SortAndSweep::TSort()
{
/*
	// l <- Empty list that will contain the sorted elements
	std::vector<AABB*> l;

	// s <- Set of all nodes with no incoming edges
	std::set<AABB*> s;
	s.insert(&m_aabbVec[1]);  // 0 is the singleton, 1 is the first aabb, which is the "screen"

	while (!s.empty())
	{
		// remove a node n from S
		std::set<AABB*>::iterator setIter = s.begin();
		AABB* n = (*setIter);
		s.erase(setIter);

		// insert n into L
		l.push_back(n);

		// for each node m with an edge e from n to m do
		std::vector<AABB*>::iterator vecIter;
		for (vecIter = n->edges.begin(); vecIter != n->edges.end(); ++vecIter)
		{

		}
	}
*/

/*
L <- Empty list that will contain the sorted elements
S <- Set of all nodes with no incoming edges
while S is non-empty do
    remove a node n from S
    insert n into L
    for each node m with an edge e from n to m do
        remove edge e from the graph
        if m has no other incoming edges then
            insert m into S
if graph has edges then
    return error (graph has at least one cycle)
else 
    return L (a topologically sorted order)
 */

}
