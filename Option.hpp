//
// Option.hpp
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

#ifndef OPTION_HPP
#define OPTION_HPP

enum SolKind {IND_SET, CLIQUE, VERTEX_COVER, DOMINATING_SET,
              MATCHING, PATH,
              TREE, SP_TREE, FOREST, R_SP_FOREST, ST_TREE};

struct Option {
public:
    bool show_info = true;
    bool rand_start = false;
    bool rand_max = false;
    bool is_enum = false;
    bool st_mode = false;
    bool stb_mode = false;
    bool st_file = false;
    bool zdd_dir = false;
    bool longest_mode = false;
    bool is_gc = false;
    bool is_rainbow = false;
    int tar_k = 0;
    std::string graph_filename;
    std::string st_filename;
    std::string zdd_dirname;

    enum SolKind sol_kind = IND_SET;
    enum Model model = TJ;

public:

    bool isEdgeVariable() const
    {
        return sol_kind == MATCHING
            || sol_kind == TREE
            || sol_kind == SP_TREE
            || sol_kind == FOREST
            || sol_kind == R_SP_FOREST
            || sol_kind == ST_TREE;
    }

    void parse(int argc, char** argv)
    {
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == std::string("--indrand")) {
                std::cerr << "'--indrand' option is changed into ";
                std::cerr << "'--randstart' option." << std::endl;
                exit(1);
            } else if (std::string(argv[i]) == std::string("--randstart")) {
                rand_start = true;
            } else if (std::string(argv[i]) == std::string("--randmax")) {
                rand_max = true;
            } else if (std::string(argv[i]) == std::string("--enum")) {
                is_enum = true;
            } else if (std::string(argv[i]) == std::string("--st")) {
                st_mode = true;
            } else if (std::string(argv[i]) == std::string("--stb")) {
                st_mode = true;
                stb_mode = true;
            } else if (std::string(argv[i]).find(std::string("--stfile=")) == 0) {
                st_file = true;
                st_filename = std::string(argv[i]).substr(9);
            } else if (std::string(argv[i]).find(std::string("--zdddir=")) == 0) {
                zdd_dir = true;
                zdd_dirname = std::string(argv[i]).substr(9);
            } else if (std::string(argv[i]) == std::string("--longest")) {
                longest_mode = true;
            } else if (std::string(argv[i]) == std::string("--gc")) {
                is_gc = true;
            } else if (std::string(argv[i]) == std::string("--info")) {
                show_info = true;
            } else if (std::string(argv[i]) == std::string("-q")
                       || std::string(argv[i]) == std::string("--quiet")) {
                show_info = false;
            } else if (std::string(argv[i]) == std::string("--tj")
                       || std::string(argv[i]) == std::string("--tokenjump")) {
                model = TJ;
            } else if (std::string(argv[i]) == std::string("--ts")
                       || std::string(argv[i]) == std::string("--tokenslide")) {
                model = TS;
            } else if (std::string(argv[i]).find(std::string("--tar=")) == 0) {
                model = TAR;
                tar_k = atoi(std::string(argv[i]).substr(6).c_str());
            } else if (std::string(argv[i]) == std::string("--rainbow")) {
                is_rainbow = true;
            } else if (std::string(argv[i]) == std::string("--indset")
                       || std::string(argv[i]) == std::string("--independentset")) {
                sol_kind = IND_SET;
            } else if (std::string(argv[i]) == std::string("--clique")) {
                sol_kind = CLIQUE;
            } else if (std::string(argv[i]) == std::string("--vertexcover")
                       || std::string(argv[i]) == std::string("--vc")) {
                sol_kind = VERTEX_COVER;
            } else if (std::string(argv[i]) == std::string("--dominatingset")
                       || std::string(argv[i]) == std::string("--ds")) {
                sol_kind = DOMINATING_SET;
            } else if (std::string(argv[i]) == std::string("--matching")) {
                sol_kind = MATCHING;
            } else if (std::string(argv[i]) == std::string("--path")) {
                sol_kind = PATH;
            } else if (std::string(argv[i]) == std::string("--tree")) {
                sol_kind = TREE;
            } else if (std::string(argv[i]) == std::string("--sptree")
                       || std::string(argv[i]) == std::string("--spanningtree")) {
                sol_kind = SP_TREE;
            } else if (std::string(argv[i]) == std::string("--forest")) {
                sol_kind = FOREST;
            } else if (std::string(argv[i]) == std::string("--rspforest")
                       || std::string(argv[i]) == std::string("--rootedspanningforest")) {
                sol_kind = R_SP_FOREST;
            } else if (std::string(argv[i]) == std::string("--sttree")
                       || std::string(argv[i]) == std::string("--steinertree")) {
                sol_kind = ST_TREE;

            } else if (argv[i][0] == '-') {
                std::cerr << "Unknown option: " << argv[i] << std::endl;
                exit(1);
            } else {
                if (graph_filename.empty()) {
                    graph_filename = std::string(argv[i]);
                } else {
                    std::cerr << "The input graph cannot be speficied twice." << std::endl;
                }
            }
        }
        if (graph_filename.empty()) {
            std::cerr << "The input graph file must be specified." << std::endl;
            exit(1);
        }
    }
};

#endif // OPTION_HPP
