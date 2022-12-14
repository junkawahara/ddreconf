//
// FrontierDegreeSpecified.hpp
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

#ifndef FRONTIER_DEGREE_SPECIFIED_HPP
#define FRONTIER_DEGREE_SPECIFIED_HPP

#include <vector>
#include <climits>

#include "FrontierManager.hpp"
#include "tdzdd/util/IntSubset.hpp"

using namespace tdzdd;

typedef unsigned char uchar;
typedef unsigned char FrontierDSData;

class FrontierDegreeSpecifiedSpec
    : public tdzdd::PodArrayDdSpec<FrontierDegreeSpecifiedSpec, FrontierDSData, 2> {
private:
    // input graph
    const tdzdd::Graph& graph_;
    // number of vertices
    const int n_;
    // number of edges
    const int m_;

    const FrontierManager fm_;

    const int fixedDegStart_;
    const std::vector<IntSubset*> degRanges_;
    const std::vector<bool> storingList_;

    // This function gets deg of v.
    int getDeg(FrontierDSData* data, int v) const {
        return static_cast<int>(data[fm_.vertexToPos(v) * 2]);
    }

    // This function sets deg of v to be d.
    void setDeg(FrontierDSData* data, int v, int d) const {
        data[fm_.vertexToPos(v) * 2] = static_cast<uchar>(d);
    }

    // This function gets comp of v.
    int getComp(FrontierDSData* data, int v, int index) const {
        return fm_.posToVertex(index, data[fm_.vertexToPos(v) * 2 + 1]);
        //return data[fm_.vertexToPos(v) * 2 + 1];
    }

    // This function sets comp of v to be c.
    void setComp(FrontierDSData* data, int v, int c) const {
        data[fm_.vertexToPos(v) * 2 + 1] = fm_.vertexToPos(c);
        //data[fm_.vertexToPos(v) * 2 + 1] = c;
    }

    void incrementFixedDeg(FrontierDSData* data, int d) const {
        ++data[fixedDegStart_ + d];
    }

    bool checkFixedDegUpper(FrontierDSData* data, int d) const {
        return (data[fixedDegStart_ + d] < degRanges_[d]->upperBound());
    }

    bool checkFixedDeg(FrontierDSData* data) const {
        for (size_t deg = 0; deg < degRanges_.size(); ++deg) {
            if (!degRanges_[deg]->contains(data[fixedDegStart_ + deg])) {
                //std::cerr << "d: " << deg << " " << fixedDegStart_ + deg << " " << data[fixedDegStart_ + deg] << std::endl;
                return false;
            }
        }
        return true;
    }

    int getDegUpper(FrontierDSData* data) const {
        int deg;
        for (deg = static_cast<int>(degRanges_.size()) - 1; deg >= 0; --deg) {
            if (data[fixedDegStart_ + deg] < degRanges_[deg]->upperBound()) {
                break;
            }
        }
        return deg;
    }

    void initializeData(FrontierDSData* data) const {
        for (int i = 0; i < fixedDegStart_ + static_cast<int>(degRanges_.size()); ++i) {
            data[i] = 0;
        }
    }

    std::vector<bool> getStoringList(const std::vector<IntSubset*>& degRanges) const {
        std::vector<bool> storingList;
        for (size_t i = 0; i < degRanges.size(); ++i) {
            if (degRanges[i]->lowerBound() == 0 &&
                degRanges[i]->upperBound() >= n_) {
                storingList.push_back(false);
            } else {
                storingList.push_back(true);
            }
        }
        return storingList;
    }

public:
    FrontierDegreeSpecifiedSpec(const tdzdd::Graph& graph,
                                const std::vector<IntSubset*>& degRanges)
        : graph_(graph),
          n_(static_cast<short>(graph_.vertexSize())),
          m_(graph_.edgeSize()),
          fm_(graph_),
          fixedDegStart_(fm_.getMaxFrontierSize() * 2),
          degRanges_(degRanges),
          storingList_(getStoringList(degRanges))
    {
        if (graph_.vertexSize() > SHRT_MAX) { // SHRT_MAX == 32767
            std::cerr << "The number of vertices should be at most "
                      << SHRT_MAX << std::endl;
            exit(1);
        }

        // todo: check all the degrees is at most 256

        setArraySize(fixedDegStart_ + degRanges_.size());
    }

    int getRoot(FrontierDSData* data) const {
        initializeData(data);
        return m_;
    }

    int getChild(FrontierDSData* data, int level, int value) const {
        assert(1 <= level && level <= m_);

        //std::cerr << "level = " << level << ", value = " << value << std::endl;

        // edge index (starting from 0)
        const int edge_index = m_ - level;
        // edge that we are processing.
        // The endpoints of "edge" are edge.v1 and edge.v2.
        const Graph::EdgeInfo& edge = graph_.edgeInfo(edge_index);

        // initialize deg and comp of the vertices newly entering the frontier
        const std::vector<int>& entering_vs = fm_.getEnteringVs(edge_index);
        for (size_t i = 0; i < entering_vs.size(); ++i) {
            int v = entering_vs[i];
            // initially the value of deg is 0
            setDeg(data, v, 0);
            // initially the value of comp is the vertex number itself
            setComp(data, v, v);
        }

        // vertices on the frontier
        const std::vector<int>& frontier_vs = fm_.getFrontierVs(edge_index);

        if (value == 1) { // if we take the edge (go to 1-arc)
            // increment deg of v1 and v2 (recall that edge = {v1, v2})

            int upper = getDegUpper(data);
            if (getDeg(data, edge.v1) + 1 > upper) {
                return 0;
            }
            if (getDeg(data, edge.v2) + 1 > upper) {
                return 0;
            }
            setDeg(data, edge.v1, getDeg(data, edge.v1) + 1);
            setDeg(data, edge.v2, getDeg(data, edge.v2) + 1);

            short c1 = getComp(data, edge.v1, edge_index);
            short c2 = getComp(data, edge.v2, edge_index);
            if (c1 != c2) { // connected components c1 and c2 become connected
                short cmin = std::min(c1, c2);
                short cmax = std::max(c1, c2);

                // replace component number cmin with cmax
                for (size_t i = 0; i < frontier_vs.size(); ++i) {
                    int v = frontier_vs[i];
                    if (getComp(data, v, edge_index) == cmin) {
                        setComp(data, v, cmax);
                    }
                }
            }
        }

        // vertices that are leaving the frontier
        const std::vector<int>& leaving_vs = fm_.getLeavingVs(edge_index);
        for (size_t i = 0; i < leaving_vs.size(); ++i) {
            int v = leaving_vs[i];

            int d = getDeg(data, v);
            if (!checkFixedDegUpper(data, d)) {
                return 0;
            }
            assert(0 <= d && d < storingList_.size());
            if (storingList_[d]) {
                incrementFixedDeg(data, d);
            }

            bool samecomp_found = false;
            bool nonisolated_found = false;

            // Search a vertex that has the component number same as that of v.
            // Also check whether a vertex whose degree is at least 1 exists
            // on the frontier.
            for (size_t j = 0; j < frontier_vs.size(); ++j) {
                int w = frontier_vs[j];
                if (w == v) { // skip if w is the leaving vertex
                    continue;
                }
                // skip if w is one of the vertices that
                // has already leaved the frontier
                bool found_leaved = false;
                for (size_t k = 0; k < i; ++k) {
                    if (w == leaving_vs[k]) {
                        found_leaved = true;
                        break;
                    }
                }
                if (found_leaved) {
                    continue;
                }
                // w has the component number same as that of v
                if (getComp(data, w, edge_index) == getComp(data, v, edge_index)) {
                    samecomp_found = true;
                }
                // The degree of w is at least 1.
                if (getDeg(data, w) > 0) {
                    nonisolated_found = true;
                }
                if (nonisolated_found && samecomp_found) {
                    break;
                }
            }
            // There is no vertex that has the component number
            // same as that of v. That is, the connected component
            // of v becomes determined.
            if (!samecomp_found) {
                // Check whether v is isolated.
                // If v is isolated (deg of v is 0), nothing occurs.
                if (d > 0) {
                    // Check whether there is a
                    // connected component other than that of v,
                    // that is, the generated subgraph is not connected.
                    // If so, we return the 0-terminal.
                    if (nonisolated_found) {
                        return 0; // return the 0-terminal.
                    } else {
                        if (checkFixedDeg(data)) {
                            return -1;
                        } else {
                            return 0;
                        }
                    }
                }
            }
            // Since deg and comp of v are never used until the end,
            // we erase the values.
            setDeg(data, v, -1);
            setComp(data, v, -1);
        }
        if (level == 1) {
            // If we come here, the edge set is empty (taking no edge).
            return 0;
        }
        assert(level - 1 > 0);
        return level - 1;
    }
};

#endif // FRONTIER_DEGREE_SPECIFIED_HPP
