#pragma once
#include "builder.hpp"

namespace cpupg
{
    class CagraBuilder : public Builder
    {
    public:
        CagraBuilder(GraphInfo info);
        virtual ~CagraBuilder();
        const Graph<int32_t> &build(Graph<int32_t> &knnG);

    private:
        void reorder(Graph<int32_t> &knnG);
        void reverse();
        void merge();
        
        Graph<int32_t> reorderG;
        Graph<int32_t> reversedG;
        std::vector<unsigned> edgeCount;
    };
} // namespace cpupg
