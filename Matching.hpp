//
// Matching.hpp
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

#ifndef MATCHING_HPP
#define MATCHING_HPP

#include "SolutionSpace.hpp"
#include "tdzdd/util/IntSubset.hpp"
#include "tdzdd/spec/DegreeConstraint.hpp"
#include "tdzdd/eval/ToZBDD.hpp"

class Matching : public SolutionSpace {
private:
    bool show_info_ = false;
    const bool is_complete_ = false;
    const tdzdd::Graph& graph_;
    const int num_vertices_;

public:
    Matching(const tdzdd::Graph& graph, int num_vertices, bool is_complete,
             bool show_info)
        : SolutionSpace(graph.edgeSize()), graph_(graph),
          num_vertices_(num_vertices), is_complete_(is_complete),
          show_info_(show_info) { }

    virtual ZBDD createSolutionSpaceZdd()
    {
        const int m = graph_.edgeSize();

        // A vertex appearing in no edge is not in tdzdd::Graph and cannot
        // be matched, so no complete matching exists.
        if (is_complete_ && graph_.vertexSize() < num_vertices_) {
            if (show_info_) {
                std::cerr << "The graph has a vertex incident to no edge; "
                          << "the solution space is empty." << std::endl;
            }
            while (BDD_VarUsed() < m) {
                BDD_NewVar();
            }
            return ZBDD(0);
        }

        IntRange range((is_complete_ ? 1 : 0), 1);
        DegreeConstraint dc(graph_, &range);

        DdStructure<2> dd(dc);
        ZBDD z = dd.evaluate(ToZBDD());
        if (z == ZBDD(-1)) {
            std::cerr << "Cannot construct the matching set ZDD due to memory shortage." << std::endl;
            exit(1);
        }
        return z;
    }
};

#endif // MATCHING_HPP
