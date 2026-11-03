#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// 三种交通方式，与 TransportGraph 内部权值数组的下标一一对应。
enum class TransportMode { ROAD = 0, RAILWAY = 1, AIR = 2 };

// 返回交通方式的中文名称（公路 / 铁路 / 航空）。
const char* transportModeName(TransportMode mode);

// 有向带权图：节点为城市，边为道路。
// 每种交通方式各有一张 n x n 的权值矩阵，权值 0 表示该方式没有这条直达线路。
// 权值具体代表费用、时间还是距离，由使用者在输入时确定。
class TransportGraph {
public:
    explicit TransportGraph(int cityCount);

    int size() const { return cityCount_; }

    // 城市名称与节点编号的双向映射。
    void setCity(int nodeId, const std::string& name);
    int cityId(const std::string& name) const;  // 找不到返回 -1
    const std::string& cityName(int nodeId) const;

    void setEdge(TransportMode mode, int from, int to, int weight);
    int edgeWeight(TransportMode mode, int from, int to) const;  // 0 表示无直达线路
    int edgeCount() const { return edgeCount_; }

private:
    int cityCount_;
    // weights_[mode][from][to]
    std::vector<std::vector<std::vector<int>>> weights_;
    std::vector<std::string> names_;
    std::unordered_map<std::string, int> nameToId_;
    int edgeCount_ = 0;
};
