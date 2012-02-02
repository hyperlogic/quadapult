#ifndef SORTANDSWEEP_H
#define SORTANDSWEEP_H

#include "abaci.h"

#define MAX_AABBS 10000

struct Box {
	Box(const Vector2f& minIn, const Vector2f& maxIn, void* userPtrIn) : userPtr(userPtrIn)
	{
		min[0] = minIn.x;
		min[1] = minIn.y;
		max[0] = maxIn.x;
		max[1] = maxIn.y;
	}

	float min[2];
	float max[2];
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
		Elem min;
		Elem max;
		void* userPtr;
	};

	SortAndSweep();

	void Insert(const Box& box);
	void Dump() const;

protected:
	Elem* m_listHead[2];
	AABB m_aabbArray[MAX_AABBS];
	int m_aabbArraySize;
};

#endif
