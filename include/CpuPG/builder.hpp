#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>

namespace CpuPG
{
    template <typename id_t = int32_t>
    using Graph = std::vector<std::vector<id_t>>;

    struct GraphInfo
    {
        size_t N;
        size_t R;
        size_t R_INIT;
    };

    template <typename id_t = int32_t>
    class Builder
    {
    public:
        Builder(GraphInfo info);
        virtual ~Builder();

    private:
        Graph<id_t> graph;
        GraphInfo info;
    };
} // namespace CpuPG