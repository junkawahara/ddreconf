//
// DominatingSet.hpp
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

#ifndef DOMINATING_SET_HPP
#define DOMINATING_SET_HPP

#include "SolutionSpace.hpp"
#include "VariableConditionSpec.hpp"

class DominatingSet : public SolutionSpace {
private:
    bool show_info_ = false;
    const tdzdd::Graph& graph_;

public:
    DominatingSet(const tdzdd::Graph& graph, int num_vertices,
                  bool show_info)
        : SolutionSpace(num_vertices), graph_(graph),
          show_info_(show_info) { }

    virtual ZBDD createSolutionSpaceZdd()
    {
        const int m = graph_.edgeSize();

        std::vector<bddvar> vararr;
        for (int i = 0; i < num_elements_; ++i) {
            BDD_NewVar();
            vararr.push_back(i + 1);
        }

        ZBDD dominating_set_zdd = sbddh::getPowerSet(vararr);

        for (int v = 1; v <= num_elements_; ++v) {
            std::set<int> s;
            s.insert(v);

            // insert all the neighbors of v into s
            for (int j = 0; j < m; ++j) {
                const Graph::EdgeInfo& edge = graph_.edgeInfo(j);
                if (edge.v1 == v) {
                    s.insert(edge.v2);
                } else if (edge.v2 == v) {
                    s.insert(edge.v1);
                }
            }
            VariableConditionSpec vcspec(s, num_elements_,
                                         VariableConditionKind::AT_LEAST_ONE);
            DdStructure<2> dd(vcspec);
            ZBDD zx = dd.evaluate(ToZBDD());
            dominating_set_zdd &= zx;
            if (dominating_set_zdd == ZBDD(-1)) {
                std::cerr << "Cannot construct the dominating set ZDD due to memory shortage." << std::endl;
                exit(1);
            }
        }

        return dominating_set_zdd;
    }
};

#endif // DOMINATING_SET_HPP
