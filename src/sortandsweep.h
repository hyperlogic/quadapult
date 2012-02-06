#ifndef SORTANDSWEEP_H
#define SORTANDSWEEP_H

#include "abaci.h"
#include <vector>
#include <utility>

#define MAX_AABBS 10000

class SortAndSweep
{
public:

	struct AABB;

	struct Elem {
		Elem* prev[2];
		Elem* next[2];
		float value[2];
		void SetMax() { ismax = true; }
		void SetMin() { ismax = false; }
		bool IsMax() const { return ismax; };
		bool IsMin() const { return !ismax; };
        AABB* GetAABB() const 
        { 
            return (AABB*)(ismax ? this - 1 : this);
        }
	protected:
		bool ismax;
	};

	struct AABB {
		AABB()
		{
			min.prev[0] = 0; min.prev[1] = 0;
			min.next[0] = 0; min.next[1] = 0;
			min.value[0] = 0; min.value[1] = 0;
			min.SetMin();

			max.prev[0] = 0; max.prev[1] = 0;
			max.next[0] = 0; max.next[1] = 0;
			max.value[0] = 0; max.value[1] = 0;
			max.SetMax();
		}

		void Set(const Vector2f& minIn, const Vector2f& maxIn, void* userPtrIn)
		{
            userPtr = userPtrIn;
			min.prev[0] = 0; min.prev[1] = 0;
			min.next[0] = 0; min.next[1] = 0;
			min.value[0] = minIn.x; min.value[1] = minIn.y;

			max.prev[0] = 0; max.prev[1] = 0;
			max.next[0] = 0; max.next[1] = 0;
			max.value[0] = maxIn.x; max.value[1] = maxIn.y;
		}

		Elem min;
		Elem max;

		void* userPtr;

        // for AABB pool
        AABB* next;
        AABB* prev;
	};

	SortAndSweep();

    AABB* AllocAABB();
    void FreeAABB(AABB* aabb);

	void Insert(AABB* aabb);
	void Dump() const;
	void DumpOverlaps() const;

	typedef std::pair<const AABB*, const AABB*> OverlapPair;
	typedef std::vector<OverlapPair> OverlapPairVec;
	const OverlapPairVec& GetOverlapPairVec() const { return m_overlapPairVec; }

	static bool UnitTest();

protected:

	void AddOverlapPair(const AABB* a, const AABB* b);

	Elem* m_listHead[2];

    // AABB pool
	AABB m_pool[MAX_AABBS];
    AABB* m_used;
    size_t m_numUsed;
    AABB* m_free;
    size_t m_numFree;

	std::vector<OverlapPair> m_overlapPairVec;
};

#endif
