//
// SteinerSubgraph.hpp
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

#ifndef STEINER_SUBGRAPH_HPP
#define STEINER_SUBGRAPH_HPP

#include "SolutionSpace.hpp"
#include "tdzdd/util/IntSubset.hpp"
#include "tdzdd/spec/DegreeConstraint.hpp"
#include "tdzdd/spec/FrontierBasedSearch.hpp"
#include "tdzdd/eval/ToZBDD.hpp"

// Steiner subgraph or cycle.
// Steiner tree is provided by ForestOrTree class.
class SteinerSubgraph : public SolutionSpace {
private:
    bool show_info_ = false;
    const tdzdd::Graph& graph_;

    bool is_cycle_;

public:
    SteinerSubgraph(const tdzdd::Graph& graph, bool is_cycle,
                    bool show_info)
        : SolutionSpace(graph.edgeSize()), graph_(graph),
          is_cycle_(is_cycle),
          show_info_(show_info) { }

    virtual ZBDD createSolutionSpaceZdd()
    {
        const int m = graph_.edgeSize();

        DdStructure<2> dd;

        int nuc = 0;

        //if (show_info_) {
        //    tdzdd::MessageHandler::showMessages(true);
        //}

        // 2nd argument: number of (uncolored edge) components
        // 3rd argument: no_loop
        FrontierBasedSearch fbs(graph_, nuc, false);

        if (is_cycle_) {
            IntRange zero_or_two(0, 2, 2);
            DegreeConstraint dc(graph_);
            for (int v = 1; v <= graph_.vertexSize(); ++v) {
                std::ostringstream oss;
                oss << v;
                std::string vs = oss.str();
                dc.setConstraint(vs, &zero_or_two);
            }
            dd = DdStructure<2>(dc);
            dd.zddSubset(fbs);
        } else {
            dd = DdStructure<2>(fbs);
        }

        ZBDD z = dd.evaluate(ToZBDD());
        if (z == ZBDD(-1)) {
            std::cerr << "Cannot construct the Steiner subgraph/cycle ZDD due to memory shortage." << std::endl;
            exit(1);
        }
        return z;
    }
};

#endif // STEINER_SUBGRAPH_HPP
