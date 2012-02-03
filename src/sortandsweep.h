#ifndef SORTANDSWEEP_H
#define SORTANDSWEEP_H

#include "abaci.h"
#include <vector>
#include <utility>

#define MAX_AABBS 10000

struct Box {
	Box(const Vector2f& minIn, const Vector2f& maxIn, float depthIn, void* userPtrIn) : depth(depthIn), userPtr(userPtrIn)
	{
		min[0] = minIn.x;
		min[1] = minIn.y;
		max[0] = maxIn.x;
		max[1] = maxIn.y;
	}

	float min[2];
	float max[2];
	float depth;
	void* userPtr;
};


class SortAndSweep
{
public:

	struct AABB;

	struct Elem {
		AABB* owner;
		Elem* prev[2];
		Elem* next[2];
		float value[2];
		void setMax() { ismax = true; }
		void setMin() { ismax = false; }
		bool isMax() const { return ismax; };
		bool isMin() const { return !ismax; };
	protected:
		bool ismax;
	};

	struct AABB {
		AABB()
		{
			min.prev[0] = 0; min.prev[1] = 0;
			min.next[0] = 0; min.next[1] = 0;
			min.value[0] = 0; min.value[1] = 0;
			min.setMin();
			min.owner = this;

			max.prev[0] = 0; max.prev[1] = 0;
			max.next[0] = 0; max.next[1] = 0;
			max.value[0] = 0; max.value[1] = 0;
			max.setMax();
			max.owner = this;
		}

		AABB(const Vector2f& minIn, const Vector2f& maxIn, void* userPtrIn) : userPtr(userPtrIn)
		{
			min.prev[0] = 0; min.prev[1] = 0;
			min.next[0] = 0; min.next[1] = 0;
			min.value[0] = minIn.x; min.value[1] = minIn.y;
			min.setMin();
			min.owner = this;

			max.prev[0] = 0; max.prev[1] = 0;
			max.next[0] = 0; max.next[1] = 0;
			max.value[0] = maxIn.x; max.value[1] = maxIn.y;
			max.setMax();
			max.owner = this;
		}

		Elem min;
		Elem max;
		void* userPtr;
	};

	SortAndSweep();

	void Insert(const AABB& aabb);
	void Dump() const;

	void TSort();

	typedef std::pair<const AABB*, const AABB*> OverlapPair;
	typedef std::vector<OverlapPair> OverlapPairVec;
	const OverlapPairVec& GetOverlapPairVec() const { return m_overlapPairVec; }

protected:

	void AddOverlapPair(const AABB* a, const AABB* b);

	Elem* m_listHead[2];
	AABB m_aabbArray[MAX_AABBS];
	int m_aabbArraySize;

	std::vector<OverlapPair> m_overlapPairVec;
};

#endif
