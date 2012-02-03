#include "graph.h"
#include <stdio.h>
#include "sprite.h"

void Graph::AddEdge(const Sprite* a, const Sprite* b)
{
	printf("AddEdge %s -> %s\n", a->GetName().c_str(), b->GetName().c_str());

	NodeMap::iterator mapIterA = m_nodeMap.find(a);
	if (mapIterA == m_nodeMap.end())
	{
		// insert a new node into the map
		std::pair<NodeMap::iterator, bool> pair = m_nodeMap.insert(NodeMapPair(a, new Node(a)));
		assert(pair.second);
		mapIterA = pair.first;
	}

	NodeMap::iterator mapIterB = m_nodeMap.find(b);
	if (mapIterB == m_nodeMap.end())
	{
        // insert a new node into the map
		std::pair<NodeMap::iterator, bool> pair = m_nodeMap.insert(NodeMapPair(b, new Node(b)));
		assert(pair.second);
		mapIterB = pair.first;
	}

	// a -> b
	mapIterA->second->edgeList.push_front(mapIterB->second);
}

void Graph::Dump() const
{
	NodeMap::const_iterator mapIter = m_nodeMap.begin();
	NodeMap::const_iterator mapEnd = m_nodeMap.end();
	for(; mapIter != mapEnd; ++mapIter)
	{
		Node* n = mapIter->second;
		printf("%s -> [ ", n->sprite->GetName().c_str());
		Node::EdgeList::iterator listIter = n->edgeList.begin();
		Node::EdgeList::iterator listEnd = n->edgeList.end();
		for(; listIter != listEnd; ++listIter)
		{
			printf("%s ", (*listIter)->sprite->GetName().c_str());
		}
		printf("]\n");
	}
}
