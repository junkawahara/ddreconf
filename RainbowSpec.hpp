//
// RainbowSpec.hpp
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

#ifndef RAINBOW_SPEC_HPP
#define RAINBOW_SPEC_HPP

typedef unsigned long long int uint64;

class RainbowSpec : public tdzdd::DdSpec<RainbowSpec, uint64, 2> {
private:
    std::vector<int> colors_;
    int m_;

public:
    // colors[i]: i-th edge color number (1 origin)
    // colors[0] is never used.
    RainbowSpec(const std::vector<int>& colors, int m) : colors_(colors), m_(m)
    {
        for (size_t i = 1; i < colors.size(); ++i) {
            if (colors[i] > 64) {
                std::cerr << "Currently support at most 64 colors" << std::endl;
                exit(1);
            }
        }
    }

    int getColorBit(uint64 used_color, int color_number) const
    {
        return ((used_color >> (color_number - 1)) & 1llu);
    }

    void setColorBit(uint64& used_color, int color_number) const
    {
        used_color |= 1llu << (color_number - 1);
    }

    int getRoot(uint64& used_color) const
    {
        used_color = 0;
        return m_;
    }

    int getChild(uint64& used_color, int level, int value) const
    {
        if (value == 1) {
            if (colors_[level] > 0) {
                if (getColorBit(used_color, colors_[level])) {
                    return 0; // The subgraph includes two edges with the same color.
                              // Then, return 0-terminal
                }
                setColorBit(used_color, colors_[level]);
            }
        }
        if (level == 1) {
            return -1; // 1-terminal
        } else {
            return level - 1;
        }
    }
};

#endif // RAINBOW_SPEC_HPP
