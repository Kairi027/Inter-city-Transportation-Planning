"""生成 20 个城市、70+ 条有向道路的规模测试输入 tests/large_input.txt。"""

import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "tests", "large_input.txt")

CITIES = [
    "北京", "上海", "广州", "深圳", "成都",
    "杭州", "武汉", "西安", "南京", "重庆",
    "天津", "苏州", "长沙", "郑州", "青岛",
    "沈阳", "昆明", "厦门", "济南", "哈尔滨",
]
N = len(CITIES)


def ring_edges(offset, direction=1):
    edges = []
    for i in range(N):
        j = (i + direction) % N
        weight = offset + (i * 37 + j * 11) % 50
        edges.append((i, j, weight))
    return edges


road = ring_edges(10, 1) + ring_edges(15, -1) + [
    (0, 10, 25), (5, 15, 40), (10, 0, 30), (15, 5, 35),
]
rail = ring_edges(8, 1) + [(0, 5, 22), (5, 10, 18), (10, 15, 26), (15, 0, 31)]
air = [
    (0, 10, 100), (0, 11, 120), (5, 15, 90),
    (10, 0, 100), (15, 5, 90), (1, 11, 150),
]

assert len(set((0, u, v) for u, v, _ in road)) == len(road)
lines = [str(N)]
for edges in (road, rail, air):
    for u, v, w in edges:
        lines.append(f"{u} {v} {w}")
    lines.append("0 0 0")
for i, name in enumerate(CITIES):
    lines.append(f"{name} {i}")
lines += ["北京 哈尔滨", "上海 昆明", "广州 西安", "哈尔滨 北京", "@ @"]

with open(OUT, "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(lines) + "\n")

total = len(road) + len(rail) + len(air)
print(f"已生成 {OUT}")
print(f"城市 {N} 个，有向道路 {total} 条（公路 {len(road)}、铁路 {len(rail)}、航空 {len(air)}）")
