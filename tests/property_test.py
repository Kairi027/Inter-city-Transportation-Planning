"""随机图交叉验证：用独立的 Python Dijkstra 核对 C++ 程序的答案与路线。

用法（在 transport 目录下）：
    python tests/property_test.py [轮数]
"""

import os
import random
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE = os.path.join(ROOT, "transport.exe")
TMP = os.path.join(ROOT, "tests", "_tmp_input.txt")

MODE_CN = {0: "公路", 1: "铁路", 2: "航空"}
CN_MODE = {v: k for k, v in MODE_CN.items()}
METRIC_CN = {1: "费用", 2: "时间", 3: "距离"}
RESULT_LABEL = {1: "最短距离", 2: "最短时间", 3: "最短距离"}


def reference_dijkstra(weights, n, start, end):
    """weights[mode][u][v] = w，返回最小总权值（不可达返回 None）。"""
    inf = float("inf")
    dist = [inf] * n
    dist[start] = 0
    visited = [False] * n
    while True:
        u = -1
        best = inf
        for i in range(n):
            if not visited[i] and dist[i] < best:
                best = dist[i]
                u = i
        if u == -1:
            break
        visited[u] = True
        for mode in range(3):
            for v, w in weights[mode][u].items():
                if dist[u] + w < dist[v]:
                    dist[v] = dist[u] + w
    return None if dist[end] == inf else dist[end]


def generate(n, density):
    weights = [[{} for _ in range(n)] for _ in range(3)]
    for mode in range(3):
        for u in range(n):
            for v in range(n):
                if u != v and random.random() < density:
                    weights[mode][u][v] = random.randint(1, 30)
    return weights


def build_input(n, metric, weights):
    lines = [str(n)]
    for mode in range(3):
        for u in range(n):
            for v, w in sorted(weights[mode][u].items()):
                lines.append(f"{u} {v} {w}")
        lines.append("0 0 0")
    for i in range(n):
        lines.append(f"c{i} {i}")
    for u in range(n):
        for v in range(n):
            if u != v:
                lines.append(f"c{u} c{v}")
    lines.append("@ @")
    return "\n".join(lines) + "\n"


def parse_output(text, n, metric):
    """返回 {(u,v): (verdict, total, route_cities, route_modes)}。"""
    answers = {}
    lines = [ln.strip() for ln in text.splitlines()]
    result_re = re.compile(
        r"^c(\d+)到c(\d+)的" + re.escape(RESULT_LABEL[metric]) + r"为(\d+)$"
    )
    unreachable_re = re.compile(r"^c(\d+)到c(\d+)不可达$")

    i = 0
    while i < len(lines):
        line = lines[i]
        m = result_re.match(line)
        if m:
            u, v, total = int(m.group(1)), int(m.group(2)), int(m.group(3))
            assert i + 2 < len(lines), (u, v, "结果行后缺少路线")
            assert lines[i + 1] == "具体行进路线为", (u, v, lines[i + 1])
            route = lines[i + 2]
            parts = re.split(r"通过(公路|铁路|航空)到达", route)
            answers[(u, v)] = ("reachable", total, parts[0::2], parts[1::2])
            i += 3
            continue
        m = unreachable_re.match(line)
        if m:
            answers[(int(m.group(1)), int(m.group(2)))] = (
                "unreachable",
                None,
                None,
                None,
            )
        i += 1
    return answers


def check_route(weights, answer, start, end):
    verdict, total, cities, modes = answer
    assert cities is not None and modes is not None, (start, end, "缺少路线")
    assert len(cities) == len(modes) + 1, (start, end, cities, modes)
    assert cities[0] == f"c{start}", (start, end, cities)
    assert cities[-1] == f"c{end}", (start, end, cities)
    actual = 0
    for i, mode_cn in enumerate(modes):
        u = int(cities[i][1:])
        v = int(cities[i + 1][1:])
        w = weights[CN_MODE[mode_cn]][u].get(v)
        assert w is not None, (start, end, "路线使用了不存在的边", u, v, mode_cn)
        actual += w
    assert actual == total, (start, end, "路线权值之和与结果不符", actual, total)


def run_round(n, density, metric):
    weights = generate(n, density)
    with open(TMP, "w", encoding="utf-8", newline="\n") as f:
        f.write(build_input(n, metric, weights))
    metric_arg = {1: "cost", 2: "time", 3: "distance"}[metric]
    with open(TMP, "rb") as stdin_file:
        proc = subprocess.run(
            [EXE, metric_arg], stdin=stdin_file, capture_output=True, check=False
        )
    assert proc.returncode == 0, proc.stderr.decode("utf-8", "replace")
    text = proc.stdout.decode("utf-8", "replace")
    answers = parse_output(text, n, metric)

    reachable_count = 0
    for u in range(n):
        for v in range(n):
            if u == v:
                continue
            expected = reference_dijkstra(weights, n, u, v)
            key = (u, v)
            assert key in answers, (u, v, "缺少输出", text[-500:])
            answer = answers[key]
            if expected is None:
                assert answer[0] == "unreachable", (u, v, "应为不可达", answer)
            else:
                reachable_count += 1
                assert answer[0] == "reachable", (u, v, "应可达", answer)
                assert answer[1] == expected, (u, v, "总权值不符", answer[1], expected)
                check_route(weights, answer, u, v)
    return reachable_count


def main():
    rounds = int(sys.argv[1]) if len(sys.argv) > 1 else 40
    random.seed(20251101)
    total_checked = 0
    for r in range(rounds):
        n = random.choice([5, 6, 7, 8])
        density = random.choice([0.15, 0.3, 0.5])
        metric = random.choice([1, 2, 3])
        total_checked += run_round(n, density, metric)
    print(f"通过：{rounds} 张随机图，{total_checked} 条可达查询全部与参考实现一致")


if __name__ == "__main__":
    main()
