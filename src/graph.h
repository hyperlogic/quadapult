#ifndef GRAPH_H
#define GRAPH_H

#include <map>
#include <list>
#include <set>
#include <vector>
#include <queue>

struct Node;
class Sprite;

typedef std::list<Node*> NodeList;
typedef std::set<Node*> NodeSet;
typedef std::vector<Node*> NodeVec;
typedef std::queue<Node*> NodeQueue;
typedef std::vector<NodeVec*> NodeVecVec;

struct Node {
	Node(const Sprite* spriteIn) : sprite(spriteIn), parent(0), level(-1) {}
	const Sprite* sprite;

    NodeList childList;
	Node* parent;
	int level;

    void AddChild(Node* child);
    void RemChild(Node* child);
};

struct Graph {
	Graph(Sprite* root);
	void AddEdge(const Sprite* a, const Sprite* b);

	void Dump() const;

	// NOTE: caller needs to delete NodeVec* elements.
    void TSort(NodeVecVec& nodeVecVec);

protected:
	typedef std::map<const Sprite*, Node*> NodeMap;
	typedef std::pair<const Sprite*, Node*> NodeMapPair;

    NodeMap::iterator FindOrInsertSprite(const Sprite* sprite);

	void DumpRec(const Node*, int indent) const;

	NodeMap m_nodeMap;
    NodeMap::iterator m_rootIter;
	int m_maxLevel;
};

#endif
