#include <cpupg/builder_cagra.hpp>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <omp.h>
#include <assert.h>

constexpr int lines = 128 / 8;

namespace cpupg
{
    CagraBuilder::CagraBuilder(GraphInfo info) : Builder(info) {}

    const Graph<int32_t> &CagraBuilder::build(Graph<int32_t> &knnG)
    {
        reorder(knnG);
        reverse();
        merge();
        return graph;
    }

    void CagraBuilder::reorder(Graph<int32_t> &knnG)
    {
        assert(info.R_INIT <= info.R_KNNG);
        Graph<unsigned short> countG(info.N, info.R_INIT); // 统计可绕行边数量

#pragma omp parallel for schedule(dynamic, 100)
        for (int id_x = 0; id_x < countG.N; id_x++)
        {

            knnG.prefetch(id_x, lines);
            knnG.prefetch(knnG.at(id_x, 0), lines); // 可能并没有什么用哦

            std::unordered_map<int, int> neighbors_x;
            for (int32_t i = 0; i < countG.K; ++i)
            {
                int id_y = knnG.at(id_x, i);
                neighbors_x[id_y] = i;
            }

            for (int32_t dist_x_y = 0; dist_x_y < countG.K; dist_x_y++)
            {

                knnG.prefetch(knnG.at(id_x, dist_x_y + 1), lines);

                int32_t id_y = knnG.at(id_x, dist_x_y);

                for (int32_t dist_y_z = 0; dist_y_z < countG.K; dist_y_z++)
                {
                    int32_t id_z = knnG.at(id_y, dist_y_z);
                    auto it = neighbors_x.find(id_z);
                    if (it != neighbors_x.end())
                    {
                        int32_t dist_x_z = it->second;
                        bool detourable = std::max(dist_x_y, dist_y_z) < dist_x_z;
                        if (detourable)
                        {
                            countG.at(id_x, dist_x_z)++;
                        }
                    }
                }
            }
        }
        reorderG.init(info.N, info.R);
// 根据每个点的可绕行边数量重新排序
#pragma omp parallel for schedule(dynamic, 100)
        for (int32_t id_x = 0; id_x < reorderG.N; id_x++)
        {
            std::vector<std::pair<int, int>> count;
            for (int32_t i = 0; i < reorderG.K; ++i)
            {
                count.push_back(std::make_pair(countG.at(id_x, i), knnG.at(id_x, i)));
            }
            std::sort(count.begin(), count.end());
            for (int32_t i = 0; i < reorderG.K; ++i)
            {
                // if (id_x == 0)
                // {
                //     std::cout << count[i].first << " " << count[i].second << std::endl;
                // }
                reorderG.at(id_x, i) = count[i].second;
            }
        }
        // reorderG.debug(0);
    }

    void CagraBuilder::reverse()
    {
        std::vector<std::unordered_set<int>> neighbors(reorderG.N);
#pragma omp parallel for schedule(dynamic, 100)
        for (int32_t id_x = 0; id_x < reorderG.N; id_x++)
        {
            for (int32_t i = 0; i < reorderG.K; ++i)
            {
                neighbors[id_x].insert(reorderG.at(id_x, i));
            }
        }

        reversedG.init(reorderG.N, reorderG.K);
        edgeCount.resize(reversedG.N, 0);
#pragma omp parallel for schedule(dynamic, 100)
        for (int32_t id_x = 0; id_x < reversedG.N; id_x++)
        {
            for (int32_t i = 0; i < reversedG.K; ++i)
            {
                int32_t id_y = reorderG.at(id_x, i);

                // 这里做了去重，保证同一个反向边只出现一次
                if (neighbors[id_y].find(id_x) == neighbors[id_y].end())
                {
                    unsigned short pos;
#pragma omp atomic capture
                    pos = edgeCount[id_y]++;

                    if (pos < reversedG.K)
                    {
                        reversedG.at(id_y, pos) = id_x;
                    }
                }
            }
        }

#pragma omp parallel for schedule(dynamic, 100)
        for (int32_t id_x = 0; id_x < reversedG.N; id_x++)
        {
            for (int32_t i = edgeCount[id_x]; i < reversedG.K; ++i)
            {
                reversedG.at(id_x, i) = -1;
            }
        }

        // reversedG.debug(0);
    }

    void CagraBuilder::merge()
    {
        graph.init(reorderG.N, info.R);
#pragma omp parallel for schedule(dynamic, 100)
        for (int32_t id_x = 0; id_x < reversedG.N; id_x++)
        {
            unsigned rSize = edgeCount[id_x]; // 反向图的边数
            unsigned sSize = reorderG.K;      // 正向图的边数
            unsigned rUse = 0;                // 反向图使用的边数
            unsigned sUse = 0;                // 正向图使用的边数
            if (rSize < sSize / 2)
            {
                rUse = rSize;
                sUse = sSize - rSize;
            }
            else
            {
                rUse = sSize / 2;
                sUse = sSize - rUse;
            }
            for (unsigned i = 0; i < sUse; i++)
            {
                graph.at(id_x, i) = reorderG.at(id_x, i);
            }
            for (unsigned i = 0; i < rUse; i++)
            {
                graph.at(id_x, i + sUse) = reversedG.at(id_x, i);
            }
        }

        // graph.debug(0);
    }
    CagraBuilder::~CagraBuilder() {}
}