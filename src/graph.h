#ifndef GRAPH_H
#define GRAPH_H

#include <map>
#include <list>

struct Node;
class Sprite;

struct Node {
	Node(const Sprite* spriteIn) : sprite(spriteIn) {}
	const Sprite* sprite;

	typedef std::list<Node*> EdgeList;
	EdgeList edgeList;
};

struct Graph {
	void AddEdge(const Sprite* a, const Sprite* b);

	void Dump() const;

	typedef std::map<const Sprite*, Node*> NodeMap;
	typedef std::pair<const Sprite*, Node*> NodeMapPair;
	NodeMap m_nodeMap;
};

#endif
