#include "graph.h"
#include <stdio.h>
#include "sprite.h"
#include "assert.h"

void Node::AddChild(Node* child)
{
    assert(child);
    childList.push_front(child);
	if (child->parent)
		child->parent->RemChild(child);
	child->parent = this;
}

void Node::RemChild(Node* child)
{
	assert(child);
    childList.remove(child);
	child->parent = 0;
}

Graph::Graph(Sprite* root)
{
    m_rootIter = FindOrInsertSprite(root);
	m_rootIter->second->level = 0;
	m_maxLevel = 0;
}

Graph::NodeMap::iterator Graph::FindOrInsertSprite(const Sprite* sprite)
{
	NodeMap::iterator mapIter = m_nodeMap.find(sprite);
	if (mapIter == m_nodeMap.end())
	{
		// insert a new node into the map
		std::pair<NodeMap::iterator, bool> pair = m_nodeMap.insert(NodeMapPair(sprite, new Node(sprite)));
		assert(pair.second);
		mapIter = pair.first;
	}
    return mapIter;
}

void Graph::AddEdge(const Sprite* a, const Sprite* b)
{
	NodeMap::iterator mapIterA = FindOrInsertSprite(a);
    NodeMap::iterator mapIterB = FindOrInsertSprite(b);

	// a -> b
	Node* na = mapIterA->second;
	Node* nb = mapIterB->second;
	if (na->level >= nb->level)
	{
		//printf("AddEdge %s(%d) -> %s(%d)\n", a->GetName().c_str(), na->level, b->GetName().c_str(), nb->level);
		na->AddChild(nb);
		// AJT: TODO: this is quite correct, becasue it doesn't bump up the level count of the children of nb...
		nb->level = na->level + 1;
		m_maxLevel = std::max(m_maxLevel, na->level + 1);
	}
}

void Graph::Dump() const
{
	DumpRec(m_rootIter->second, 0);
}

void Graph::DumpRec(const Node* n, int indent) const
{
	for (int i = 0; i < indent; ++i)
		printf("\t");
	printf("%s, level = %d\n", n->sprite->GetName().c_str(), n->level);
	NodeList::const_iterator listIter = n->childList.begin();
	NodeList::const_iterator listEnd = n->childList.end();
	for(; listIter != listEnd; ++listIter)
		DumpRec(*listIter, indent + 1);
}

void Graph::TSort(NodeVecVec& nodeVecVec)
{
	NodeQueue q;
	q.push(m_rootIter->second);
	int numLevels = 0;
	while (!q.empty())
	{
		Node* n = q.front();
		q.pop();
		numLevels = n->level + 1;

		NodeList::const_iterator listIter = n->childList.begin();
		NodeList::const_iterator listEnd = n->childList.end();
		for(; listIter != listEnd; ++listIter)
		{
			Node* m = *listIter;
			m->level = n->level + 1;
			q.push(m);
		}
	}

	for (int i = 0; i < numLevels; ++i)
		nodeVecVec.push_back(new NodeVec());

	NodeMap::iterator mapIter = m_nodeMap.begin();
	NodeMap::iterator mapEnd = m_nodeMap.end();
	for (; mapIter != mapEnd; ++mapIter)
	{
		Node* n = mapIter->second;
		if (n->level >= 0)
			nodeVecVec[n->level]->push_back(n);
	}
}
