//
// IndependentSet.hpp
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

#ifndef INDEPENDENT_SET_HPP
#define INDEPENDENT_SET_HPP

#include "SolutionSpace.hpp"
#include "AdjacentSpec.hpp"

class IndependentSet : public SolutionSpace {
private:
    bool show_info_ = false;
    const tdzdd::Graph& graph_;

    // If it is false, we make a ZDD
    // representing vertex covers instead of independent sets.
    const bool is_independent_set_;

public:
    IndependentSet(const tdzdd::Graph& graph, int num_vertices,
                   bool is_independent_set, bool show_info)
        : SolutionSpace(num_vertices), graph_(graph),
          is_independent_set_(is_independent_set), show_info_(show_info) { }

    virtual ZBDD createSolutionSpaceZdd()
    {
        const int m = graph_.edgeSize();

        std::vector<bddvar> vararr;
        for (int i = 0; i < num_elements_; ++i) {
            BDD_NewVar();
            vararr.push_back(i + 1);
        }

        ZBDD independent_set_zdd = sbddh::getPowerSet(vararr);

        // Construct the ZDD representing all the families of independent sets.
        for (int i = 0; i < m; ++i) {
            const Graph::EdgeInfo& edge = graph_.edgeInfo(i);

            //int v1 = getVertexNumber(graph_, edge.v1);
            //int v2 = getVertexNumber(graph_, edge.v2);
            AdjacentSpec aspec(edge.v1, edge.v2, num_elements_, is_independent_set_);
            DdStructure<2> dd(aspec);
            ZBDD zx = dd.evaluate(ToZBDD());
            independent_set_zdd &= zx;
            if (independent_set_zdd == ZBDD(-1)) {
                std::cerr << "Cannot construct the indepndent set ZDD due to memory shortage." << std::endl;
                exit(1);
            }
        }

        return independent_set_zdd;
    }
};

#endif // INDEPENDENT_SET_HPP
