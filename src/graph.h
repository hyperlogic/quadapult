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
	Node(const Sprite* spriteIn) : sprite(spriteIn), level(-1) {}
	const Sprite* sprite;

	NodeList parentList;
    NodeList childList;
	int level;

    void AddChild(Node* child);
    void RemChild(Node* child);
};

struct Graph {
	void AddEdge(const Sprite* a, const Sprite* b);

	void Dump() const;
    void SetRoot(const Sprite* root);

	// NOTE: caller needs to delete NodeVec* elements.
    void TSort(NodeVecVec& nodeVecVec);

	void TSort2(NodeVecVec& nodeVecVec);

protected:
	typedef std::map<const Sprite*, Node*> NodeMap;
	typedef std::pair<const Sprite*, Node*> NodeMapPair;

    NodeMap::iterator FindOrInsertSprite(const Sprite* sprite);

	NodeMap m_nodeMap;
    NodeMap::iterator m_rootIter;
};

#endif
