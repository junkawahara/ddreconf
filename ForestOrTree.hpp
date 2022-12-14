//
// ForestOrTree.hpp
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

#ifndef FOREST_OR_TREE_HPP
#define FOREST_OR_TREE_HPP

#include "SolutionSpace.hpp"
#include "tdzdd/util/IntSubset.hpp"
#include "tdzdd/spec/DegreeConstraint.hpp"
#include "tdzdd/spec/FrontierBasedSearch.hpp"
#include "tdzdd/eval/ToZBDD.hpp"

#include "RainbowSpec.hpp"

class ForestOrTree : public SolutionSpace {
private:
    bool show_info_ = false;
    const tdzdd::Graph& graph_;

    bool is_tree_;
    bool is_spanning_;
    bool is_root_;
    bool is_steiner_;
    std::set<std::string> root_set_;
    bool is_rainbow_;
    std::vector<int> colors_;

public:
    ForestOrTree(const tdzdd::Graph& graph, bool is_tree,
                 bool is_spanning,
                 bool is_root, bool is_steiner,
                 std::set<std::string> root_set,
                 // for rainbow spanning trees
                 bool is_rainbow, const std::vector<int>& colors,
                 bool show_info)
        : SolutionSpace(graph.edgeSize()), graph_(graph),
          is_tree_(is_tree),
          is_spanning_(is_spanning),
          is_root_(is_root), is_steiner_(is_steiner),
          root_set_(root_set),
          is_rainbow_(is_rainbow), colors_(colors),
          show_info_(show_info) { }

    virtual ZBDD createSolutionSpaceZdd()
    {
        const int m = graph_.edgeSize();

        DdStructure<2> dd;

        int nuc = 0;
        if (is_tree_) {
            if (is_steiner_) {
                nuc = 0;
            } else {
                nuc = 1;
            }
        } else {
            if (is_root_) {
                nuc = 0;
            } else {
                nuc = -1;
            }
        }

        //if (show_info_) {
        //    tdzdd::MessageHandler::showMessages(true);
        //}

        // 2nd argument: number of (uncolored edge) components
        // When the support graph is a forest, it is arbitrary (-1).
        // When the support graph is a tree, it is 1.
        // When the support graph is a steiner tree or a rooted spanning forest,
        //    it is 0.
        // 3rd argument: no_loop
        FrontierBasedSearch fbs(graph_, nuc, true);

        if (is_spanning_) {
            IntRange at_least_one(1, graph_.vertexSize());
            // dc means the degree of each vertex is at least one.
            DegreeConstraint dc(graph_);
            for (int v = 1; v <= graph_.vertexSize(); ++v) {
                std::ostringstream oss;
                oss << v;
                std::string vs = oss.str();
                // The degree of the roots can be zero.
                if (!is_root_ || root_set_.count(vs) == 0) { // not found
                    dc.setConstraint(vs, &at_least_one);
                }
            }

            dd = DdStructure<2>(dc);
            // compute the intersection of dc and fbs
            dd.zddSubset(fbs);
        } else {
            dd = DdStructure<2>(fbs);
        }

        if (is_rainbow_) { // for rainbow (spanning) trees
            RainbowSpec rainbow(colors_, m);
            dd.zddSubset(rainbow);
        }
        ZBDD z = dd.evaluate(ToZBDD());
        if (z == ZBDD(-1)) {
            std::cerr << "Cannot construct the forest/tree set ZDD due to memory shortage." << std::endl;
            exit(1);
        }
        return z;
    }
};

#endif // FOREST_OR_TREE_HPP
