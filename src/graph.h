#ifndef GRAPH_H
#define GRAPH_H

#include <map>
#include <list>
#include <set>
#include <vector>

struct Node;
class Sprite;

typedef std::list<Node*> NodeList;
typedef std::set<Node*> NodeSet;
typedef std::vector<Node*> NodeVec;

struct Node {
	Node(const Sprite* spriteIn) : sprite(spriteIn) {}
	const Sprite* sprite;

	NodeList parentList;
    NodeList childList;

    void AddChild(Node* child);
    void RemChild(Node* child);
};

struct Graph {
	void AddEdge(const Sprite* a, const Sprite* b);

	void Dump() const;
    void SetRoot(const Sprite* root);

    void TSort(NodeVec& nodeVec);

protected:
	typedef std::map<const Sprite*, Node*> NodeMap;
	typedef std::pair<const Sprite*, Node*> NodeMapPair;

    NodeMap::iterator FindOrInsertSprite(const Sprite* sprite);

	NodeMap m_nodeMap;
    NodeMap::iterator m_rootIter;
};

#endif
