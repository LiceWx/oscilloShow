#include "constructor.h"
#include "canny.h"
#include <bits/stdc++.h>
#include <cstdlib>

vector<pii> signalXY;
vector<pii> points;
vector<vector<pii>> edges; // bracket order of dfs

matrix<bool> visited;
matrix<int> belong;
matrix<int> dfn;

const int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

void dfs(int x, int y, int e) {
    points.push_back(std::make_pair(x, y));
    dfn[x][y] = points.size() - 1; 
    visited[x][y] = true;
    
    edges[e].push_back(std::make_pair(x, y));
    belong[x][y] = e;

    int height = grayMatrix.size();
    int width = grayMatrix[0].size();

    for (int d = 0; d < 8; d++) {
        int nx = x + dx[d];
        int ny = y + dy[d];
        if (nx >= 0 && nx < height && ny >= 0 && ny < width && grayMatrix[nx][ny] == 255 && !visited[nx][ny]) {
            dfs(nx, ny, e);
        }
    }
    
    edges[e].push_back(std::make_pair(x, y));
}

distance::distance() {
    d = 1e9;
}

distance::distance(pii p1, pii p2) : p1(p1), p2(p2) {
    d = (p1.first - p2.first) * (p1.first - p2.first) + (p1.second - p2.second) * (p1.second - p2.second);
}

bool distance::operator<(const distance& other) const {
    return d < other.d;
}

int distance::from_edge() {
    return belong[p1.first][p1.second];
}

int distance::to_edge() {
    return belong[p2.first][p2.second];
}

matrix<distance> dist;
vector<pii> dist_sorted;

// Prim
int root;
vector<vector<distance>> mst;

void prim_MST() {
    int n = edges.size();
    if (n == 0) return;
    
    vector<bool> inMST(n, false);
    vector<double> key(n, 1e18);
    vector<int> parent(n, -1);
    
    // Start from random root
    root = rand() % n;
    key[root] = 0;
    
    mst.resize(n);
    
    for (int count = 0; count < n; count++) {
        // Find minimum key vertex not in MST
        int u = -1;
        for (int v = 0; v < n; v++) {
            if (!inMST[v] && (u == -1 || key[v] < key[u])) {
                u = v;
            }
        }
        
        inMST[u] = true;
        
        // Add edge to MST if not root
        if (parent[u] != -1) {
            mst[u].push_back(dist[u][parent[u]]);
            mst[parent[u]].push_back(dist[parent[u]][u]);
        }
        
        // Update key values of adjacent vertices
        for (int v = 0; v < n; v++) {
            if (!inMST[v] && dist[u][v].d < key[v]) {
                key[v] = dist[u][v].d;
                parent[v] = u;
            }
        }
    }
}

// int max_cnt = 0;
void travel(int u, int fa, pii starting_point) {
    for (auto it = mst[u].begin(); it != mst[u].end(); it++) {
        if (it->to_edge() == fa) {
            mst[u].erase(it);
            break;
        }
    }

    std::sort(mst[u].begin(), mst[u].end(), [&](const distance& a, const distance& b) {
        pii pa = a.p1, pb = b.p1;
        return dfn[pa.first][pa.second] < dfn[pb.first][pb.second];
    });
    
    // int cnt = 0;
    auto cur_edge_it = edges[u].begin();
    auto go_to_next = [&]() {
        ++cur_edge_it;
        if (cur_edge_it == edges[u].end()) {
            cur_edge_it = edges[u].begin();
            // ++cnt;
        }
    };

    while (*cur_edge_it != starting_point) {
        go_to_next();
    }
    auto starting_it = cur_edge_it;

    // cnt = 0;
    for (auto v : mst[u]) {
        while (*cur_edge_it != v.p1) {
            signalXY.push_back(*cur_edge_it);
            go_to_next();
        }
        signalXY.push_back(*cur_edge_it);
        travel(v.to_edge(), u, v.p2);
    }

    do {
        signalXY.push_back(*cur_edge_it);
        go_to_next();
    } while (cur_edge_it != starting_it);

    // max_cnt = std::max(max_cnt, cnt);
}

void construct_signal() {
    int height = grayMatrix.size();
    int width = grayMatrix[0].size();
    visited = matrix<bool>(height, vector<bool>(width, false));
    belong = matrix<int>(height, vector<int>(width, -1));
    dfn = matrix<int>(height, vector<int>(width, -1));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (grayMatrix[i][j] == 255) {
                if (visited[i][j]) {
                    continue;
                }
                edges.push_back(vector<pii>());
                dfs(i, j, edges.size() - 1);
            }
        }
    }
    std::cout << "points : " << points.size() << std::endl;
    std::cout << "edges : " << edges.size() << std::endl;

    dist = matrix<distance>(edges.size(), vector<distance>(edges.size()));
    // for (int i = 0; i < points.size(); i++) {
    //     for (int j = i + 1; j < points.size(); j++) {
    //         distance d(points[i], points[j]);
    //         int u = belong[points[i].first][points[i].second];
    //         int v = belong[points[j].first][points[j].second];
    //         if (d < dist[u][v]) {
    //             dist[u][v] = d;
    //             std::swap(d.p1, d.p2);
    //             dist[v][u] = d;
    //         }
    //     }
    // }
    for (int i = 0; i < edges.size(); i++) {
        for (int j = i + 1; j < edges.size(); j++) {
            for (int u = 0; u < edges[i].size(); u++) {
                for (int v = 0; v < edges[j].size(); v++) {
                    distance d(edges[i][u], edges[j][v]);
                    if (d < dist[i][j]) 
                        dist[i][j] = d;
                }
            }
            dist[j][i] = dist[i][j];
            swap(dist[j][i].p1, dist[j][i].p2);
        }
    }
    
    prim_MST();
    travel(root, -1, edges[root][0]);

    std::cout << "edges : " << edges.size() << std::endl;
    std::cout << "signal length: " << signalXY.size() << std::endl;
}
