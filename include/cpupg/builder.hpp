#pragma once
#include "graph.hpp"

namespace cpupg
{
    class Builder
    {
    public:
        Builder(GraphInfo info);
        virtual ~Builder();

    protected:
        Graph<> graph;
        GraphInfo info;
    };
} // namespace cpupg