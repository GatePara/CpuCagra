#pragma once
#include "builder.hpp"

namespace cpupg
{
    class CagraBuilder : public Builder
    {
    public:
        CagraBuilder(GraphInfo info);
        virtual ~CagraBuilder();
        const Graph<> &build(Graph<> &knnG);

    private:
        void reorder(Graph<> &knnG);
        void reverse();
        void merge();

        Graph<> reorderG;
        Graph<> reversedG;
        std::vector<uint64_t> edgeCount;
    };
} // namespace cpupg
