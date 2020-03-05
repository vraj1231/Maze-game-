#ifndef GRAPH_H
#define GRAPH_H
#include <player.h>
#include <list>
#include <unordered_map>
#include <stack>
//#include <queue>
#include <vector>
#include <utility>
#include <iostream>
using namespace std;

class graph
{
public:

    graph(int verticesCount);       // Constructor
    virtual ~graph();

    bool checkKey(int);             // Returns true if key exists in unordered map
    int returnKey(int, int);        // Returns key associated with coordinate pair from unordered map
    bool checkPair(int, int);       // Returns true if pair exists in unordered map

    void initGraph(int **, int);    // Initializes graph using matrix
    void addEdge(int v, int w);     // to add an edge to graph
    void DFS(int s, Player *);      // prints all vertices in DFS manner

    pair<int, int> returnAction();  // Returns first step in path from enemy to player
    void clearVector();             // Clears vector contaning path(keys) from enemy to player
private:

    vector<int> vPath;              // Contains path from enemy to player in keys
    unordered_map<int, pair<int,int> > nodeMap; // Used to store keys and pairs of coordinates
    int verticesCount;              // Number of vertices
    list<int> *adjList;             // adjacency lists
};

#endif // GRAPH_H
