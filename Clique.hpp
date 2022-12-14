//
// Clique.hpp
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

#ifndef CLIQUE_HPP
#define CLIQUE_HPP

#include "SolutionSpace.hpp"
#include "AdjacentSpec.hpp"

class Clique : public SolutionSpace {
private:
    bool show_info_ = false;
    const tdzdd::Graph& graph_;

public:
    Clique(const tdzdd::Graph& graph, int num_vertices, bool show_info)
        : SolutionSpace(num_vertices), graph_(graph), show_info_(show_info) { }

    virtual ZBDD createSolutionSpaceZdd()
    {
        const int n = graph_.vertexSize();
        const int m = graph_.edgeSize();

        std::vector<bddvar> vararr;
        for (int i = 0; i < num_elements_; ++i) {
            BDD_NewVar();
            vararr.push_back(i + 1);
        }

        double start_time = getTime();

        ZBDD clique_zdd = sbddh::getPowerSet(vararr);

        // Construct the ZDD representing all the families of cliques.
        for (int vv1 = 1; vv1 <= n; ++vv1) {
            for (int vv2 = vv1 + 1; vv2 <= n; ++vv2) {
                bool found = false;
                for (int i = 0; i < m; ++i) {
                    const Graph::EdgeInfo& edge = graph_.edgeInfo(i);
                    if ((edge.v1 == vv1 && edge.v2 == vv2)
                        || (edge.v1 == vv2 && edge.v2 == vv1)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    int v1 = getVertexNumber(graph_, vv1);
                    int v2 = getVertexNumber(graph_, vv2);
                    AdjacentSpec aspec(v1, v2, num_elements_, true);
                    DdStructure<2> dd(aspec);
                    ZBDD zx = dd.evaluate(ToZBDD());
                    clique_zdd &= zx;
                    if (clique_zdd == ZBDD(-1)) {
                        std::cerr << "Cannot construct the clique set ZDD due to memory shortage." << std::endl;
                        exit(1);
                    }
                }
            }
        }
        double end_time = getTime();

        if (show_info_) {
            std::cout << "ZDD for cliques time = " << (end_time - start_time) << std::endl;
        }

        std::cout << std::fixed;
        std::cout << std::setprecision(6);

        if (show_info_) {
            std::cout << "clique ZDD size = "
                      << clique_zdd.Size() << std::endl;
            std::cout << "# of cliques = "
                      << getCard(clique_zdd) << std::endl;
        }
        return clique_zdd;
    }
};

#endif // CLIQUE_HPP
