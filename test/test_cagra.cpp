#include <iostream>
#include <chrono>
#include <unordered_set>
#include <cpupg/builder_cagra.hpp>
#include <cpupg/parameters.hpp>

int main(int argc, char *argv[])
{
    cpupg::CagraConfig config = cpupg::loadCagraConfig(argv[1]);

    cpupg::Graph<int32_t> knnG;
    knnG.loadKnng(config.knng_path.c_str());
    cpupg::GraphInfo info;

    info.N = knnG.N;
    info.R_KNNG = knnG.K;
    info.R_INIT = config.r_init;
    info.R = config.r;
    info.print();

    auto start = std::chrono::high_resolution_clock::now();
    cpupg::CagraBuilder builder(info);
    cpupg::Graph<int32_t> cagraG = builder.build(knnG); // knnG will be destroyed!
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "cost time: " << diff.count() << "s" << std::endl;

    size_t edges = 0;
    size_t deDupEdges = 0;

    // 检查是否有重复边
    // #pragma omp parallel for reduction(+ : edges, deDupEdges) schedule(dynamic, 100)
    //     for (int i = 0; i < cagraG.N; i++)
    //     {
    //         std::unordered_set<int> s;
    //         for (int j = 0; j < cagraG.K; j++)
    //         {
    //             if (cagraG.at(i, j) != -1)
    //             {
    //                 edges++;
    //                 s.insert(cagraG.at(i, j));
    //             }
    //         }
    //         deDupEdges += s.size();
    //     }
    //     float ratio = (float)deDupEdges / (float)edges;
    //     std::cout << "edges: " << edges << " dupEdges: " << deDupEdges << " ratio: " << ratio << std::endl;

    cagraG.saveKnng(config.save_path.c_str());
    return 0;
}