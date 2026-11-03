#include "graph.h"

#include <stdexcept>

namespace {

constexpr int kTransportModeCount = 3;

int modeIndex(TransportMode mode) {
    return static_cast<int>(mode);
}

}  // namespace

const char* transportModeName(TransportMode mode) {
    switch (mode) {
        case TransportMode::ROAD:
            return "公路";
        case TransportMode::RAILWAY:
            return "铁路";
        case TransportMode::AIR:
            return "航空";
    }
    return "未知";
}

TransportGraph::TransportGraph(int cityCount) : cityCount_(cityCount) {
    if (cityCount <= 0) {
        throw std::invalid_argument("城市个数必须大于 0");
    }
    weights_.assign(
        kTransportModeCount,
        std::vector<std::vector<int>>(cityCount, std::vector<int>(cityCount, 0)));
    names_.resize(cityCount);
}

void TransportGraph::setCity(int nodeId, const std::string& name) {
    if (nodeId < 0 || nodeId >= cityCount_) {
        throw std::out_of_range("节点编号超出范围");
    }
    if (name.empty()) {
        throw std::invalid_argument("城市名称不能为空");
    }
    if (!names_[nodeId].empty()) {
        throw std::invalid_argument("该节点编号已经设置过城市");
    }
    if (nameToId_.count(name) != 0) {
        throw std::invalid_argument("城市名称重复");
    }
    names_[nodeId] = name;
    nameToId_[name] = nodeId;
}

int TransportGraph::cityId(const std::string& name) const {
    auto it = nameToId_.find(name);
    return it == nameToId_.end() ? -1 : it->second;
}

const std::string& TransportGraph::cityName(int nodeId) const {
    if (nodeId < 0 || nodeId >= cityCount_) {
        throw std::out_of_range("节点编号超出范围");
    }
    return names_[nodeId];
}

void TransportGraph::setEdge(TransportMode mode, int from, int to, int weight) {
    if (from < 0 || from >= cityCount_ || to < 0 || to >= cityCount_) {
        throw std::out_of_range("节点编号超出范围");
    }
    if (weight <= 0) {
        throw std::invalid_argument("权值必须为正整数");
    }
    int& slot = weights_[modeIndex(mode)][from][to];
    if (slot == 0) {
        ++edgeCount_;
    }
    slot = weight;
}

int TransportGraph::edgeWeight(TransportMode mode, int from, int to) const {
    if (from < 0 || from >= cityCount_ || to < 0 || to >= cityCount_) {
        return 0;
    }
    return weights_[modeIndex(mode)][from][to];
}
