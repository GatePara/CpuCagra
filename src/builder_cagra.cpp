#include <cpupg/builder_cagra.hpp>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <omp.h>
#include <assert.h>
#include <sys/resource.h>

void printMemoryUsage()
{
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        std::cout << "Memory usage: " << usage.ru_maxrss / 1024.0 << " MB" << std::endl;
    }
    else
    {
        std::cerr << "Error in getting memory usage" << std::endl;
    }
}

constexpr int workloads = 100;

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
        // Graph<uint32_t> countG(info.N, info.R_INIT); // 统计可绕行边数量
        reorderG.init(info.N, info.R); // 重新排序后的图
        const int lines = std::max((info.R_INIT * sizeof(int) / CACHELINE), (size_t)1);
#pragma omp parallel for schedule(dynamic, 100)
        for (int id_x = 0; id_x < knnG.N; id_x++)
        {

            knnG.prefetch(id_x, lines); // 可能并没有什么用哦
            std::unordered_map<int, int> neighbors_x;
            std::vector<std::pair<uint32_t, int>> count(info.R_INIT);
            for (int32_t i = 0; i < info.R_INIT; ++i)
            {
                int id_y = knnG.at(id_x, i);
                neighbors_x[id_y] = i;
                count[i] = {0, id_y};
            }

            for (int32_t dist_x_y = 0; dist_x_y < info.R_INIT; dist_x_y++)
            {

                int32_t id_y = knnG.at(id_x, dist_x_y);
                knnG.prefetch(knnG.at(id_x, dist_x_y + 1), lines);
                for (int32_t dist_y_z = 0; dist_y_z < info.R_INIT; dist_y_z++)
                {
                    int32_t id_z = knnG.at(id_y, dist_y_z);
                    auto it = neighbors_x.find(id_z);
                    if (it != neighbors_x.end())
                    {
                        int32_t dist_x_z = it->second;
                        bool detourable = std::max(dist_x_y, dist_y_z) < dist_x_z;
                        if (detourable)
                        {
                            count[dist_x_z].first++;
                        }
                    }
                }
            }
            std::sort(count.begin(), count.end(), [](const auto &a, const auto &b)
                      { return a.first < b.first; });
            for (int32_t i = 0; i < reorderG.K; ++i)
            {
                reorderG.at(id_x, i) = count[i].second;
            }
        }
        knnG.destory();
        printMemoryUsage();
#ifdef DEBUG
        reorderG.debug(0);
#endif
    }

    void CagraBuilder::reverse()
    {
        reversedG.init(reorderG.N, reorderG.K);
        edgeCount.resize(reversedG.N, 0);
#pragma omp parallel for schedule(dynamic, workloads)
        for (int32_t id_x = 0; id_x < reorderG.N; id_x++)
        {
            for (int32_t i = 0; i < reorderG.K; ++i)
            {
                int32_t id_y = reorderG.at(id_x, i);

                // 这里做了去重，保证同一个反向边只出现一次
                bool flag = true;
                for (int32_t j = 0; j < reorderG.K; j++)
                {
                    if (reorderG.at(id_y, j) == id_x)
                    {
                        flag = false;
                        break;
                    }
                }
                if (flag)
                {
                    unsigned pos = 0;
#pragma omp atomic capture
                    pos = edgeCount[id_y]++;

                    if (pos < reversedG.K)
                    {
                        reversedG.at(id_y, pos) = id_x;
                    }
                }
            }
        }

#pragma omp parallel for schedule(dynamic, workloads)
        for (int32_t id_x = 0; id_x < reversedG.N; id_x++)
        {
            for (int32_t i = edgeCount[id_x]; i < reversedG.K; ++i)
            {
                reversedG.at(id_x, i) = -1;
            }
        }
        printMemoryUsage();
#ifdef DEBUG
        reversedG.debug(0);
#endif
    }

    void CagraBuilder::merge()
    {
        const int lines = std::max((info.R * sizeof(int) / CACHELINE / 2), (size_t)1);
        graph.init(reorderG.N, info.R);
        graph.prefetch(0, lines);
        reorderG.prefetch(0, lines);
        reversedG.prefetch(0, lines);
#pragma omp parallel for schedule(dynamic, workloads)
        for (int32_t id_x = 0; id_x < reversedG.N; id_x++)
        {
            graph.prefetch(id_x + 1, lines);
            reorderG.prefetch(id_x + 1, lines);
            reversedG.prefetch(id_x + 1, lines);
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
        printMemoryUsage();
#ifdef DEBUG
        graph.debug(0);
#endif
    }
    CagraBuilder::~CagraBuilder() {}
}