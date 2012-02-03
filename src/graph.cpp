#include "graph.h"
#include <stdio.h>
#include "sprite.h"
#include "assert.h"

void Node::AddChild(Node* child)
{
    assert(child);
    childList.push_front(child);
    child->parentList.push_front(this);
}

void Node::RemChild(Node* child)
{
    childList.remove(child);
    child->parentList.remove(this);
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
	printf("AddEdge %s -> %s\n", a->GetName().c_str(), b->GetName().c_str());

	NodeMap::iterator mapIterA = FindOrInsertSprite(a);
    NodeMap::iterator mapIterB = FindOrInsertSprite(b);

	// a -> b
    mapIterA->second->AddChild(mapIterB->second);
}

void Graph::SetRoot(const Sprite* root)
{
    m_rootIter = FindOrInsertSprite(root);
}

void Graph::Dump() const
{
	NodeMap::const_iterator mapIter = m_nodeMap.begin();
	NodeMap::const_iterator mapEnd = m_nodeMap.end();
	for(; mapIter != mapEnd; ++mapIter)
	{
		Node* n = mapIter->second;
		printf("%s -> [\n", n->sprite->GetName().c_str());
		NodeList::iterator listIter = n->childList.begin();
		NodeList::iterator listEnd = n->childList.end();
		for(; listIter != listEnd; ++listIter)
		{
			printf("    %s\n", (*listIter)->sprite->GetName().c_str());
		}
		printf("]\n");
	}
}

void Graph::TSort(NodeVec& nodeVec)
{
    /*
L = Empty list that will contain the sorted elements
S = Set of all nodes with no incoming edges
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
    nodeVec.clear();

    NodeSet s;
    s.insert(m_rootIter->second);
    while (!s.empty())
    {
        NodeSet::iterator setIter = s.begin();
        Node* n = (*setIter);
        s.erase(setIter);

        nodeVec.push_back(n);

        NodeList childList = n->childList;
        NodeList::iterator listIter = childList.begin();
        NodeList::iterator listEnd = childList.end();
        for (; listIter != listEnd; ++listIter)
        {
            Node* m = (*listIter);
            n->RemChild(m);
            if (m->parentList.empty())
                s.insert(m);
        }
    }

    // l is topologically sorted.
    printf("sorted list = [\n");
    NodeVec::iterator vecIter = nodeVec.begin();
    NodeVec::iterator vecEnd = nodeVec.end();
    for(; vecIter != vecEnd; ++vecIter)
    {
        printf("%s\n", (*vecIter)->sprite->GetName().c_str());
    }
    printf("]\n");
}
