#include <cctype>
#include <climits>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "dijkstra.h"
#include "graph.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

// 权值的含义：本次规划按费用、时间还是距离比较。
enum class Metric { COST, TIME, DISTANCE };

const char* metricName(Metric metric) {
    switch (metric) {
        case Metric::COST:
            return "费用";
        case Metric::TIME:
            return "时间";
        case Metric::DISTANCE:
            return "距离";
    }
    return "费用";
}

// 结果行的措辞。按指导书的运行示例，按费用规划时输出“最短距离”。
const char* metricResultLabel(Metric metric) {
    switch (metric) {
        case Metric::COST:
            return "最短距离";
        case Metric::TIME:
            return "最短时间";
        case Metric::DISTANCE:
            return "最短距离";
    }
    return "最短距离";
}

std::string trim(const std::string& text) {
    std::size_t begin = 0;
    while (begin < text.size() &&
           std::isspace(static_cast<unsigned char>(text[begin]))) {
        ++begin;
    }
    std::size_t end = text.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return text.substr(begin, end - begin);
}

std::vector<std::string> splitWhitespace(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool parseInt(const std::string& text, int& value) {
    std::istringstream stream(text);
    long long parsed = 0;
    stream >> parsed;
    if (stream.fail() || !stream.eof()) {
        return false;
    }
    if (parsed < INT_MIN || parsed > INT_MAX) {
        return false;
    }
    value = static_cast<int>(parsed);
    return true;
}

bool readCityCount(int& cityCount) {
    for (;;) {
        std::cout << "请输入城市个数：";
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            return false;
        }

        int count = 0;
        if (parseInt(trim(line), count) && count > 0 && count <= 1000) {
            cityCount = count;
            return true;
        }
        std::cout << "城市个数必须是 1~1000 之间的整数，请重新输入。\n";
    }
}

bool parseMetricName(const std::string& name, Metric& metric) {
    if (name == "cost" || name == "1") {
        metric = Metric::COST;
        return true;
    }
    if (name == "time" || name == "2") {
        metric = Metric::TIME;
        return true;
    }
    if (name == "distance" || name == "3") {
        metric = Metric::DISTANCE;
        return true;
    }
    return false;
}

void printUsage() {
    std::cout << "用法：transport.exe [cost|time|distance]\n"
                 "  cost     按费用规划（默认）\n"
                 "  time     按时间规划\n"
                 "  distance 按距离规划\n";
}

// 读取某种交通方式的边，直到输入“0 0 0”。返回 false 表示输入流已结束。
bool readEdges(TransportGraph& graph, TransportMode mode, Metric metric) {
    std::cout << "请输入" << transportModeName(mode) << metricName(metric)
              << "，输入方式为点 点 权值，权值为0，则输入结束\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        const std::vector<std::string> tokens = splitWhitespace(line);
        if (tokens.empty()) {
            continue;
        }
        if (tokens.size() != 3) {
            std::cout << "输入格式错误，应为：点 点 权值\n";
            continue;
        }

        int from = 0, to = 0, weight = 0;
        if (!parseInt(tokens[0], from) || !parseInt(tokens[1], to) ||
            !parseInt(tokens[2], weight)) {
            std::cout << "输入格式错误，应为：点 点 权值\n";
            continue;
        }

        if (from == 0 && to == 0 && weight == 0) {
            return true;
        }
        if (from < 0 || from >= graph.size() || to < 0 || to >= graph.size()) {
            std::cout << "节点编号应在 0 到 " << graph.size() - 1 << " 之间\n";
            continue;
        }
        if (from == to) {
            std::cout << "起点和终点不能相同\n";
            continue;
        }
        if (weight <= 0) {
            std::cout << "权值必须为正整数\n";
            continue;
        }

        graph.setEdge(mode, from, to, weight);
    }
    return false;
}

// 读取 cityCount 行“城市 节点编号”。返回 false 表示输入流已结束。
bool readCities(TransportGraph& graph) {
    std::cout << "输入" << graph.size()
              << "个城市与节点的对应关系，输入结构为城市 节点编号\n";

    std::vector<char> used(graph.size(), 0);
    for (int i = 0; i < graph.size(); ++i) {
        for (;;) {
            std::string line;
            if (!std::getline(std::cin, line)) {
                return false;
            }
            const std::vector<std::string> tokens = splitWhitespace(line);
            if (tokens.size() != 2) {
                std::cout << "输入格式错误，应为：城市 节点编号\n";
                continue;
            }

            int nodeId = 0;
            if (!parseInt(tokens[1], nodeId) || nodeId < 0 ||
                nodeId >= graph.size()) {
                std::cout << "节点编号应在 0 到 " << graph.size() - 1
                          << " 之间\n";
                continue;
            }
            if (used[nodeId]) {
                std::cout << "节点编号 " << nodeId << " 重复，请重新输入\n";
                continue;
            }
            if (graph.cityId(tokens[0]) != -1) {
                std::cout << "城市名称 " << tokens[0] << " 重复，请重新输入\n";
                continue;
            }

            graph.setCity(nodeId, tokens[0]);
            used[nodeId] = 1;
            break;
        }
    }
    return true;
}

void printRoute(const TransportGraph& graph, int start, int end, Metric metric) {
    const PathResult result = shortestPath(graph, start, end);
    const std::string& from = graph.cityName(start);
    const std::string& to = graph.cityName(end);

    if (!result.reachable) {
        std::cout << from << "到" << to << "不可达\n";
        return;
    }

    std::cout << from << "到" << to << "的" << metricResultLabel(metric) << "为"
              << result.totalWeight << "\n";
    std::cout << "具体行进路线为\n";

    if (result.nodes.size() == 1) {
        std::cout << from << "\n";
        return;
    }

    for (std::size_t i = 0; i < result.modes.size(); ++i) {
        std::cout << graph.cityName(result.nodes[i]) << "通过"
                  << transportModeName(result.modes[i]) << "到达";
    }
    std::cout << graph.cityName(result.nodes.back()) << "\n";
}

void queryLoop(const TransportGraph& graph, Metric metric) {
    std::cout << "输入两个城市，用空格隔开,当输入@ @时退出\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        const std::vector<std::string> tokens = splitWhitespace(line);
        if (tokens.empty()) {
            continue;
        }
        if (tokens.size() != 2) {
            std::cout << "每行应输入两个城市，用空格隔开\n";
            continue;
        }
        if (tokens[0] == "@" && tokens[1] == "@") {
            return;
        }

        const int start = graph.cityId(tokens[0]);
        const int end = graph.cityId(tokens[1]);
        if (start == -1) {
            std::cout << "未找到城市：" << tokens[0] << "\n";
            continue;
        }
        if (end == -1) {
            std::cout << "未找到城市：" << tokens[1] << "\n";
            continue;
        }

        printRoute(graph, start, end, metric);
    }
}

int run(Metric metric) {
    int cityCount = 0;
    if (!readCityCount(cityCount)) {
        return 0;
    }

    TransportGraph graph(cityCount);

    if (!readEdges(graph, TransportMode::ROAD, metric) ||
        !readEdges(graph, TransportMode::RAILWAY, metric) ||
        !readEdges(graph, TransportMode::AIR, metric) ||
        !readCities(graph)) {
        std::cout << "\n输入结束。\n";
        return 0;
    }

    queryLoop(graph, metric);
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // 让控制台按 UTF-8 显示和读入中文。
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    Metric metric = Metric::COST;
    for (int i = 1; i < argc; ++i) {
        std::string argument = argv[i];
        if (argument == "-h" || argument == "--help") {
            printUsage();
            return 0;
        }
        if (argument.rfind("--metric=", 0) == 0) {
            argument = argument.substr(9);
        }
        if (!parseMetricName(argument, metric)) {
            std::cerr << "未知参数：" << argv[i] << "\n";
            printUsage();
            return 2;
        }
    }

    try {
        return run(metric);
    } catch (const std::exception& error) {
        std::cerr << "程序错误：" << error.what() << "\n";
        return 1;
    }
}
