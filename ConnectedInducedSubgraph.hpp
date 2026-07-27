//
// ConnectedInducedSubgraph.hpp
//
// Copyright (c) 2020 -- 2022 Jun Kawahara and project "Fusion of Computer Science, Engineering
//     and Mathematics Approaches for Expanding Combinatorial Reconfiguration"
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef CONNECTED_INDUCED_SUBGRAPH_HPP
#define CONNECTED_INDUCED_SUBGRAPH_HPP

#include "SolutionSpace.hpp"
//#include "tdzdd/util/IntSubset.hpp"
//#include "tdzdd/spec/DegreeConstraint.hpp"
#include "tdzdd/eval/ToZBDD.hpp"

#include "ConnectedInducedSubgraphSpec.hpp"
#include "ConvEVDD.hpp"

class ConnectedInducedSubgraph : public SolutionSpace {
private:
    const tdzdd::Graph& graph_;
    bool is_vertex_variable_ = false;
    bool show_info_ = false;

public:
    // num_vertices is the number of vertices in the input graph and is
    // used only when is_vertex_variable is true.
    ConnectedInducedSubgraph(const tdzdd::Graph& graph, int num_vertices,
         bool is_vertex_variable,
         bool show_info)
        : SolutionSpace(is_vertex_variable ? num_vertices
                                           : graph.edgeSize()),
          graph_(graph),
          is_vertex_variable_(is_vertex_variable),
          show_info_(show_info) { }

    virtual ZBDD createSolutionSpaceZdd()
    {
        // The frontier-based spec assumes at least one edge. Without an
        // edge, no connected induced subgraph has an edge, so the
        // edge-variable ZDD is the empty family.
        const bool has_edge = (graph_.edgeSize() > 0);

        DdStructure<2> dd_E;
        if (has_edge) {
            ConnectedInducedSubgraphSpec cis_spec(graph_, true);
            dd_E = DdStructure<2>(cis_spec);
            dd_E.zddReduce();
        }

        if (is_vertex_variable_) {
            while (BDD_VarUsed() < num_elements_) {
                BDD_NewVar();
            }
            ZBDD dd_V = ZBDD(0);
            if (has_edge) {
                // translate E-DD to V-DD
                ConvEVDD::VariableList vlist(graph_);
                dd_V = ConvEVDD::eToVZdd(dd_E, graph_, vlist);
            }
            // The subgraph induced by a single vertex is connected but
            // contains no edge, so it is not represented by the
            // edge-variable ZDD. Add it here.
            for (int v = 1; v <= num_elements_; ++v) {
                dd_V += sbddh::getSingleton(v);
            }
            return dd_V;
        } else {
            return (has_edge ? dd_E.evaluate(ToZBDD()) : ZBDD(0));
        }
    }
};

#endif // CONNECTED_INDUCED_SUBGRAPH_HPP
