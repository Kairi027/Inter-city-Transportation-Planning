#include "dijkstra.h"

#include <limits>
#include <stack>

namespace {

constexpr long long kInfinity = std::numeric_limits<long long>::max() / 4;

constexpr TransportMode kTransportModes[] = {
    TransportMode::ROAD, TransportMode::RAILWAY, TransportMode::AIR};

}  // namespace

PathResult shortestPath(const TransportGraph& graph, int start, int end) {
    PathResult result;
    const int n = graph.size();
    if (start < 0 || start >= n || end < 0 || end >= n) {
        return result;
    }

    std::vector<long long> dist(n, kInfinity);
    std::vector<char> visited(n, 0);
    std::vector<PreInfo> preInfo(n);

    dist[start] = 0;

    for (int iter = 0; iter < n; ++iter) {
        // 选出当前未访问节点中距离最小的一个。
        int u = -1;
        for (int i = 0; i < n; ++i) {
            if (!visited[i] && dist[i] < kInfinity &&
                (u == -1 || dist[i] < dist[u])) {
                u = i;
            }
        }
        if (u == -1) {
            break;  // 剩余节点均不可达
        }

        visited[u] = 1;
        if (u == end) {
            break;
        }

        // 用 u 更新邻居：同一条边可能有三种交通方式，逐种比较后取更小的总权值。
        for (int v = 0; v < n; ++v) {
            if (visited[v]) {
                continue;
            }
            for (TransportMode mode : kTransportModes) {
                const int w = graph.edgeWeight(mode, u, v);
                if (w == 0) {
                    continue;
                }
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    preInfo[v].preVex = u;
                    preInfo[v].transportMode = mode;
                }
            }
        }
    }

    if (dist[end] >= kInfinity) {
        return result;  // 不可达
    }

    result.reachable = true;
    result.totalWeight = dist[end];

    if (start == end) {
        result.nodes.push_back(start);
        return result;
    }

    // 从终点沿前驱数组回溯到起点，回溯过程中压栈；
    // 之后依次出栈，就得到从起点到终点的正向路径（指导书“栈逆转路径”）。
    std::stack<PreInfo> path;
    int current = end;
    int steps = 0;
    while (current != start) {
        if (current < 0 || current >= n || steps++ > n) {
            return PathResult{};  // 理论上不会发生，防止前驱链异常
        }
        path.push(preInfo[current]);
        current = preInfo[current].preVex;
    }

    result.nodes.push_back(start);
    while (!path.empty()) {
        const PreInfo edge = path.top();
        path.pop();

        // edge.transportMode 是 edge.preVex -> 这条边终点 所使用的方式；
        // 该边终点是下一个栈元素的前驱，若栈已空，则终点就是查询的终点城市。
        const int next = path.empty() ? end : path.top().preVex;
        result.modes.push_back(edge.transportMode);
        result.nodes.push_back(next);
    }

    return result;
}
