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

enum SolKind {IND_SET, CLIQUE, VERTEX_COVER, CONNECTED_VERTEX_COVER,
              DOMINATING_SET, CONNECTED_DOMINATING_SET,
              MATCHING, CMATCHING, PATH,
              TREE, SP_TREE, FOREST, R_SP_FOREST, ST_TREE,
              STEINER_SUB, STEINER_CYCLE, CIS_E, CIS_V};

struct Option {
public:
    bool show_info = true;
    bool rand_start = false;
    bool rand_max = false;
    bool is_enum = false;
    bool st_mode = false;
    bool stb_mode = false;
    bool stw_mode = false;
    bool st_file = false;
    bool zdd_dir = false;
    bool longest_mode = false;
    bool is_gc = false;
    bool is_rainbow = false;
    bool vorder_out = false;
    int tar_k = 0;
    std::string graph_filename;
    std::string st_filename;
    std::string zdd_dirname;
    std::string vorder_filename;
    std::string vorder_out_filename;

    enum SolKind sol_kind = IND_SET;
    enum Model model = TJ;
    enum VertexOrder vertex_order = VO_LEAVE;

public:

    bool isEdgeVariable() const
    {
        return sol_kind == MATCHING
            || sol_kind == CMATCHING
            || sol_kind == PATH
            || sol_kind == TREE
            || sol_kind == SP_TREE
            || sol_kind == FOREST
            || sol_kind == R_SP_FOREST
            || sol_kind == ST_TREE
            || sol_kind == STEINER_SUB
            || sol_kind == STEINER_CYCLE
            || sol_kind == CIS_E;
    }

    // The solution space of these problems is filtered by (or is) the
    // connected induced subgraph ZDD, whose vertex variable order is
    // determined by ConvEVDD.
    bool usesConnectivityFilter() const
    {
        return sol_kind == CIS_V
            || sol_kind == CONNECTED_VERTEX_COVER
            || sol_kind == CONNECTED_DOMINATING_SET;
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
            } else if (std::string(argv[i]) == std::string("--stw")
                       || std::string(argv[i]) == std::string("--stwidth")) {
                st_mode = true;
                stw_mode = true;
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
                std::cerr << "The token sliding model is not implemented yet."
                          << std::endl;
                exit(1);
            } else if (std::string(argv[i]).find(std::string("--tar=")) == 0) {
                model = TAR;
                tar_k = atoi(std::string(argv[i]).substr(6).c_str());
            } else if (std::string(argv[i]).find(std::string("--vorderfile=")) == 0) {
                vertex_order = VO_FILE;
                vorder_filename = std::string(argv[i]).substr(13);
            } else if (std::string(argv[i]).find(std::string("--vorderout=")) == 0) {
                vorder_out = true;
                vorder_out_filename = std::string(argv[i]).substr(12);
            } else if (std::string(argv[i]).find(std::string("--vorder=")) == 0
                       || std::string(argv[i]).find(std::string("--vertexorder=")) == 0) {
                std::string value = std::string(argv[i]);
                value = value.substr(value.find('=') + 1);
                if (value == "leave") {
                    vertex_order = VO_LEAVE;
                } else if (value == "asc") {
                    vertex_order = VO_ASC;
                } else if (value == "desc") {
                    vertex_order = VO_DESC;
                } else {
                    std::cerr << "Unknown vertex order: " << value
                              << " (must be leave, asc, or desc)" << std::endl;
                    exit(1);
                }
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
            } else if (std::string(argv[i]) == std::string("--connectedvertexcover")
                       || std::string(argv[i]) == std::string("--cvc")) {
                sol_kind = CONNECTED_VERTEX_COVER;
            } else if (std::string(argv[i]) == std::string("--dominatingset")
                       || std::string(argv[i]) == std::string("--ds")) {
                sol_kind = DOMINATING_SET;
            } else if (std::string(argv[i]) == std::string("--connecteddominatingset")
                       || std::string(argv[i]) == std::string("--cds")) {
                sol_kind = CONNECTED_DOMINATING_SET;
            } else if (std::string(argv[i]) == std::string("--matching")) {
                sol_kind = MATCHING;
            } else if (std::string(argv[i]) == std::string("--cmatching")
                       || std::string(argv[i]) == std::string("--completematching")) {
                sol_kind = CMATCHING;
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
            } else if (std::string(argv[i]) == std::string("--stsub")
                       || std::string(argv[i]) == std::string("--steinersubgraph")) {
                sol_kind = STEINER_SUB;
            } else if (std::string(argv[i]) == std::string("--stcycle")
                       || std::string(argv[i]) == std::string("--steinercycle")) {
                sol_kind = STEINER_CYCLE;
            } else if (std::string(argv[i]) == std::string("--cise")
                       || std::string(argv[i]) == std::string("--connectedinducedsubgraphedge")) {
                sol_kind = CIS_E;
            }else if (std::string(argv[i]) == std::string("--cisv")
                       || std::string(argv[i]) == std::string("--connectedinducedsubgraphvertex")) {
                sol_kind = CIS_V;
            } else if (argv[i][0] == '-') {
                std::cerr << "Unknown option: " << argv[i] << std::endl;
                exit(1);
            } else {
                if (graph_filename.empty()) {
                    graph_filename = std::string(argv[i]);
                } else {
                    std::cerr << "The input graph cannot be specified twice."
                              << std::endl;
                    exit(1);
                }
            }
        }
        if (graph_filename.empty()) {
            std::cerr << "The input graph file must be specified." << std::endl;
            exit(1);
        }
        if (rand_max && !rand_start) {
            std::cerr << "--randmax must be used with --randstart."
                      << std::endl;
            exit(1);
        }
        if (zdd_dir && (stb_mode || stw_mode)) {
            // Only the one-directional search saves the ZDDs into files.
            std::cerr << "--zdddir is ignored for --stb and --stw."
                      << std::endl;
            zdd_dir = false;
        }
        if ((vertex_order != VO_LEAVE || vorder_out)
            && sol_kind != IND_SET && sol_kind != CLIQUE
            && sol_kind != VERTEX_COVER && sol_kind != DOMINATING_SET) {
            // The frontier-based vertex-variable problems (e.g. --cisv,
            // --cvc, --cds) assume that the variable order is the
            // frontier-leaving order, and the vertex order is meaningless
            // for the edge-variable problems.
            std::cerr << "--vorder, --vorderfile, and --vorderout are "
                      << "supported only for --indset, --clique, --vc, "
                      << "and --ds." << std::endl;
            exit(1);
        }
    }
};

#endif // OPTION_HPP
