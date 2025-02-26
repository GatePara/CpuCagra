#include <iostream>
#include <chrono>
#include <unordered_set>
#include <cpupg/builder_cagra.hpp>
#include <cpupg/parameters.hpp>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <json_path>" << std::endl;
        exit(-1);
    }

    cpupg::CagraConfig config = cpupg::loadCagraConfig(argv[1]);

    cpupg::Graph knnG;
    if (config.knng_format == "efanna")
    {
        std::cout << "Loading efanna knng from " << config.knng_path << std::endl;
        knnG.loadKnng(config.knng_path.c_str());
    }
    else if (config.knng_format == "fbin")
    {
        std::cout << "Loading fbin knng from " << config.knng_path << std::endl;
        knnG.loadKnngFbin(config.knng_path.c_str());
    }
    else
    {
        std::cerr << config.knng_format << " is not supported!" << std::endl;
        exit(-1);
    }
    std::cout << "Loaded!" << std::endl;

    cpupg::GraphInfo info;

    info.N = knnG.N;
    info.R_KNNG = knnG.K;
    info.R_INIT = config.r_init;
    info.R = config.r;
    info.print();

    auto start = std::chrono::high_resolution_clock::now();
    cpupg::CagraBuilder builder(info);
    cpupg::Graph cagraG = builder.build(knnG); // knnG will be destroyed!
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Cost time: " << diff.count() << " s" << std::endl;

// 检查是否有重复边
#ifdef DEBUG
    if (1)
    {
        size_t edges = 0;
        size_t deDupEdges = 0;
#pragma omp parallel for reduction(+ : edges, deDupEdges) schedule(dynamic, 100)
        for (int32_t i = 0; i < cagraG.N; i++)
        {
            std::unordered_set<int> s;
            for (uint64_t j = 0; j < cagraG.K; j++)
            {
                if (cagraG.at(i, j) != -1)
                {
                    edges++;
                    s.insert(cagraG.at(i, j));
                }
            }
            deDupEdges += s.size();
        }
        float ratio = (float)deDupEdges / (float)edges;
        std::cout << "Total edges: " << edges << " Deduplicate edges: " << deDupEdges << " ratio: " << ratio << std::endl;
    }
#endif
    std::cout << "Saving cagra to " << config.save_path << std::endl;
    cagraG.saveNsg(config.save_path.c_str());
    return 0;
}