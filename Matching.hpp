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
    const tdzdd::Graph& graph_;

public:
    Matching(const tdzdd::Graph& graph, bool show_info)
        : SolutionSpace(graph.edgeSize()), graph_(graph),
          show_info_(show_info) { }

    virtual ZBDD createSolutionSpaceZdd()
    {
        const int m = graph_.edgeSize();

        IntRange zero_or_one(0, 1);
        DegreeConstraint dc(graph_, &zero_or_one);

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
