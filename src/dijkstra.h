#pragma once

#include <vector>

#include "graph.h"

// 记录最短路中每个节点的前驱，以及“从前驱到当前节点”所用的交通方式。
struct PreInfo {
    int preVex = -1;
    TransportMode transportMode = TransportMode::ROAD;
};

struct PathResult {
    bool reachable = false;
    long long totalWeight = 0;
    std::vector<int> nodes;              // 起点 -> ... -> 终点
    std::vector<TransportMode> modes;    // modes[i] 表示 nodes[i] -> nodes[i+1] 的方式
};

// 在公路、铁路、航空三种方式混搭的所有边中，求权值和最小的路径（Dijkstra）。
PathResult shortestPath(const TransportGraph& graph, int start, int end);
