#pragma once
#include "builder.hpp"

namespace CpuPG
{
    template <typename id_t = int32_t>
    class CagraBuilder : public Builder<id_t>
    {

    public:
        CagraBuilder(GraphInfo info)
        {
            this->info = info;
            graph.resize(info.N);
        }
        ~CagraBuilder() {}

    private:
        Graph<id_t> graph;
    };
} // namespace CpuPG
