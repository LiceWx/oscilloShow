#ifndef CONSTRUCTOR_H
#define CONSTRUCTOR_H

#include <vector>
#include <utility>
#include "canny.h"

using std::vector;
using std::pair;
using pii = pair<int, int>;

// External global variables
extern vector<pii> signalXY;
extern vector<pii> points;
extern vector<vector<pii>> edges;
extern matrix<bool> visited;
extern matrix<int> belong;
extern matrix<int> dfn;

// Distance structure
struct distance {
    pii p1, p2;
    int d;
    distance();
    distance(pii p1, pii p2);
    bool operator<(const distance& other) const;
    int from_edge();
    int to_edge();
};

extern matrix<distance> dist;
extern vector<pii> dist_sorted;

// MST variables
extern int root;
extern vector<vector<distance>> mst;

// Function declarations
void dfs(int x, int y, int e);
void prim_MST();
void travel(int u, int fa, pii starting_point);
void construct_signal();

#endif // CONSTRUCTOR_H
