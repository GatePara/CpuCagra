#include <iostream>
#include <chrono>
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
    cpupg::Graph<int32_t> cagraG = builder.build(knnG);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "cost time: " << diff.count() << "s" << std::endl;

    cagraG.saveKnng(config.save_path.c_str());
    return 0;
}