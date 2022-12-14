//
// AdjacentSpec.hpp
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

#ifndef ADJACENTSPEC_HPP
#define ADJACENTSPEC_HPP

class AdjacentSpec : public tdzdd::DdSpec<AdjacentSpec, int, 2> {
private:
    const int v1_;
    const int v2_;
    const int n_;

    const bool cond_; // true -> both v1 and v2 must not be contained
                      // false -> at least one of v1 and v2 must be contained

public:
    AdjacentSpec(int v1, int v2, int n, bool cond) : v1_(std::max(v1, v2)),
                                                     v2_(std::min(v1, v2)),
                                                     n_(n),
                                                     cond_(cond) { }

    int getRoot(int& state) const {
        state = 0;
        return n_;
    }

    int getChild(int& state, int level, int value) const {
        if (level == v1_) {
            state = value;
        } else if (level == v2_) {
            if (cond_) {
                if (state == 1 && value == 1) {
                    return 0;
                } else {
                    state = 0;
                }
            } else {
                if (state == 0 && value == 0) {
                    return 0;
                } else {
                    state = 0;
                }
            }
        }
        if (level == 1) {
            return -1;
        } else {
            return level - 1;
        }
    }
};

#endif // ADJACENTSPEC_HPP
