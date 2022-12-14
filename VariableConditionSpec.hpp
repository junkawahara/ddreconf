//
// VariableConditionSpec.hpp
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

#ifndef VARIABLE_CONDITION_SPEC_HPP
#define VARIABLE_CONDITION_SPEC_HPP

#include <set>
#include <algorithm>

enum VariableConditionKind { AT_LEAST_ONE, NOT_ALL };

class VariableConditionSpec : public tdzdd::DdSpec<VariableConditionSpec, int, 2> {
private:
    const std::set<int> vs_;
    const int vmin_;
    const int n_;

    const bool kind_; // AT_LEAST_ONE -> at least one of the variables must be contained
                      // NOT_ALL -> not all variables must be contained (not yet supported)

public:
    VariableConditionSpec(const std::set<int>& vs, int n, VariableConditionKind kind)
        : vs_(vs), vmin_(*std::min_element(vs.begin(), vs.end())), n_(n), kind_(kind)
    { }

    int getRoot(int& state) const {
        switch (kind_) {
        case AT_LEAST_ONE:
            state = 0;
            break;
        default:
            std::cerr << "not implemented" << std::endl;
            exit(1);
        }
        return n_;
    }

    int getChild(int& state, int level, int value) const {
        if (level == vmin_) {
            switch (kind_) {
            case AT_LEAST_ONE:
                if (state == 0 && value == 0) {
                    return 0;
                }
                break;
            default:
                std::cerr << "not implemented" << std::endl;
                exit(1);
            }
        } else if (vs_.count(level) > 0) {
            switch (kind_) {
            case AT_LEAST_ONE:
                if (value == 1) {
                    state = 1;
                }
                break;
            default:
                std::cerr << "not implemented" << std::endl;
                exit(1);
            }
        }

        if (level == 1) {
            return -1;
        } else {
            return level - 1;
        }
    }
};

#endif // VARIABLE_CONDITION_SPEC_HPP
