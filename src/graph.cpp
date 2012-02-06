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

int Node::LevelUp(int levelIn)
{
    level = levelIn;
    int maxLevel = level;
	NodeList::iterator listIter = childList.begin();
	NodeList::iterator listEnd = childList.end();
	for(; listIter != listEnd; ++listIter)
		maxLevel = std::max(maxLevel, (*listIter)->LevelUp(levelIn + 1));
    return maxLevel;
}

Graph::Graph(Sprite* root)
{
    m_rootIter = FindOrInsertSprite(root);
	m_rootIter->second->level = 0;
	m_maxLevel = 0;
}

Graph::~Graph()
{
	NodeMap::iterator mapIter = m_nodeMap.begin();
	NodeMap::iterator mapEnd = m_nodeMap.end();
	for (; mapIter != mapEnd; ++mapIter)
	{
		Node* n = mapIter->second;
        delete n;
    }
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
		na->AddChild(nb);
        m_maxLevel = std::max(m_maxLevel, nb->LevelUp(na->level + 1));
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
	for (int i = 0; i < m_maxLevel + 1; ++i)
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
