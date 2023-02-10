//
// main.cpp
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

#define BDD_CPP

#include "SAPPOROBDD/include/ZBDD.h"
#include "ZBDD_reconf.hpp"
#include "sbdd_helper/SBDD_helper.h"

#include "tdzdd/DdSpec.hpp"
#include "tdzdd/DdStructure.hpp"
#include "tdzdd/eval/ToZBDD.hpp"
#include "tdzdd/spec/SapporoZdd.hpp"
#include "tdzdd/util/Graph.hpp"

#include <vector>
#include <list>
#include <set>
#include <string>
#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <functional>
#include <time.h>
#include <sys/time.h>

using namespace tdzdd;
using namespace sbddh;

enum Model {TJ, TS, TAR};

#include "RandomSample.hpp"
#include "Utility.hpp"
#include "Option.hpp"

#include "IndependentSet.hpp"
#include "Clique.hpp"
#include "DominatingSet.hpp"
#include "Matching.hpp"
#include "Path.hpp"
#include "ForestOrTree.hpp"

#include "Reconf.hpp"

#include "MaxEval.hpp"

int main(int argc, char** argv) {

#ifndef STAND_ALONE
    if (argc <= 1) {
        std::cout << "Usage: " << argv[0]
                  << " <graph_file> [option] [model] [sol_kind]" << std::endl;
        std::cout << "[option]" << std::endl;
        std::cout << "  --randstart: randomly generates"
                  << " a start set" << std::endl;
        std::cout << "  --st: computes the reconf sequence from s to t"
                  << std::endl;
        std::cout << "  --stb: computes the reconf sequence from s to t in the bidirection"
                  << std::endl;
        std::cout << "  --stfile=<file_name>: specifies s and t"
                  << " in file <file_name>" << std::endl;
        std::cout << "  --longest: computes the longest reconf seq"
                  << std::endl;
        std::cout << "  --gc: force to run GC periodically"
                  << std::endl;
        std::cout << "  --rainbow: for rainbow spanning trees"
                  << std::endl;
        std::cout << "  --info: outputs info to stderr (default on)"
                  << std::endl;
        std::cout << "  -q: suppresses output info"
                  << std::endl;
        std::cout << "[model]" << std::endl;
        std::cout << "  --tj: token jumping model (default)" << std::endl;
        std::cout << "  --ts: token sliding model "
                  << "(currently not implemented)" << std::endl;
        std::cout << "  --tar=<k>: token addition/removal model "
                  << "(<k> is an integer)" << std::endl;
        std::cout << "[sol_kind]" << std::endl;
        std::cout << "  --indset: independent set (default)" << std::endl;
        std::cout << "  --clique: clique" << std::endl;
        std::cout << "  --vc: vertex cover" << std::endl;
        std::cout << "  --ds: dominating set" << std::endl;
        std::cout << "  --matching: matching" << std::endl;
        std::cout << "  --path: path" << std::endl;
        std::cout << "  --tree: tree" << std::endl;
        std::cout << "  --sptree: spanning tree" << std::endl;
        std::cout << "  --forest: forest" << std::endl;
        std::cout << "  --rspforest: rooted spanning forest" << std::endl;
        std::cout << "  --sttree: Steiner tree" << std::endl;
        return 1;
    }
#endif

    Option option;
    int num_vertices = -1;

#ifdef STAND_ALONE

    option.st_mode = true;
    option.show_info = false;

#else
    option.parse(argc, argv);
#endif

    BigIntegerRandom random;

    BDD_Init(1024llu * 1024llu, 3llu * 1024llu * 1024llu * 1024llu * 1024llu);

    std::set<bddvar> start_set;
    std::set<bddvar> goal_set;
    std::set<std::string> root_set;
    std::vector<int> colors;

    Graph graph;

#ifdef STAND_ALONE
    num_vertices = parse_DIMACS(std::cin, &graph, &start_set, &goal_set,
                                &root_set, &colors, option.isEdgeVariable());
#else
    num_vertices = parse_DIMACS(option.graph_filename.c_str(), &graph,
                                &start_set, &goal_set, &root_set, &colors,
                                option.isEdgeVariable());
#endif

    if (option.st_file) {
        start_set.clear();
        //start_set.shrink_to_fit();
        goal_set.clear();
        //goal_set.shrink_to_fit();
        parse_stfile(option.st_filename.c_str(), &graph,
                     &start_set, &goal_set, option.isEdgeVariable());
    }

    if (option.show_info) {
        std::cerr << "Input graph file parsed. "
                  << "# of vertices = " << num_vertices
                  << ", # of edges = " << graph.edgeSize() << std::endl;
        if (!option.rand_start) {
            std::cerr << "s ";
            printSet(std::cerr, start_set, graph, option.isEdgeVariable());
        }
        if (!option.longest_mode) {
            std::cerr << "t ";
            printSet(std::cerr, goal_set, graph, option.isEdgeVariable());
        }
    }

    if (option.st_mode && start_set == goal_set) {
        // Output for 0 step
        std::cout << "a ";
        printSet(std::cout, start_set, graph, option.isEdgeVariable());
        return 0;
    }

    if (option.isEdgeVariable()) {
        // change edge IDs to SAPPOROBDD variable numbers
        // from 1,...,m to m,...,1
        start_set = inverseSet(start_set, graph.edgeSize() + 1);
        goal_set = inverseSet(goal_set, graph.edgeSize() + 1);
    }

    int col = 1;
    SolutionSpace* space;
    ZBDD solution_space_zdd;
    switch (option.sol_kind) {
    case IND_SET:
        space = new IndependentSet(graph, num_vertices, true, option.show_info);
        break;
    case CLIQUE:
        space = new Clique(graph, num_vertices, option.show_info);
        break;
    case VERTEX_COVER:
        // use IndependentSet class with argument is_independent_set == false
        space = new IndependentSet(graph, num_vertices, false, option.show_info);
        break;
    case DOMINATING_SET:
        space = new DominatingSet(graph, num_vertices, option.show_info);
        break;
    case MATCHING:
        space = new Matching(graph, option.show_info);
        break;
    case PATH:
        space = new Path(graph, option.show_info);
        break;
    case FOREST:
        space = new ForestOrTree(graph, false, false, false, false,
                                 root_set, option.is_rainbow, colors,
                                 option.show_info);
        break;
    case TREE:
        space = new ForestOrTree(graph, true, false, false, false,
                                 root_set, option.is_rainbow, colors,
                                 option.show_info);
        break;
    case SP_TREE:
        space = new ForestOrTree(graph, true, true, false, false,
                                 root_set, option.is_rainbow, colors,
                                 option.show_info);
        break;
    case R_SP_FOREST:
        for (std::set<std::string>::iterator itor = root_set.begin();
             itor != root_set.end(); ++itor) {
            graph.setColor(*itor, col);
            ++col;
        }
        graph.update();
        space = new ForestOrTree(graph, false, true, true, false,
                                 root_set, option.is_rainbow, colors,
                                 option.show_info);
        break;
    case ST_TREE:
        for (std::set<std::string>::iterator itor = root_set.begin();
             itor != root_set.end(); ++itor) {
            graph.setColor(*itor, 1);
        }
        graph.update();
        space = new ForestOrTree(graph, true, false, false, true,
                                 root_set, option.is_rainbow, colors,
                                 option.show_info);
        break;
    }

    if (option.show_info) {
        std::cerr << "Solution space ZDD construction start" << std::endl;
    }

    double start_time = getTime();

    solution_space_zdd = space->createSolutionSpaceZdd();

    double end_time = getTime();

    if (option.show_info) {
        std::cerr << "Solution space ZDD construction end" << std::endl;

        std::cerr << std::fixed;
        std::cerr << std::setprecision(6);

        std::cerr << "Solution space ZDD construction time = "
                  << (end_time - start_time) << std::endl;

        std::cerr << "Solution space ZDD size = "
                  << solution_space_zdd.Size() << std::endl;
        std::cerr << "# of elements in the solution space = "
                  << getCard(solution_space_zdd) << std::endl;
    }

    end_time = start_time;

    if (option.is_enum) {
        int num_elements = (option.isEdgeVariable()
                                ? graph.edgeSize() : num_vertices);
        sbddh::printZBDDElementsAsValueList(std::cout,
                                            solution_space_zdd,
                                            "\n", " ",
                                            num_elements);
        std::cout << std::endl;
    }

    if (!(option.st_mode || option.longest_mode)) {
        return 0;
    }

    // Randomly generate the start set.
    if (option.rand_start) {
        if (option.rand_max) {
            SapporoZdd sz(solution_space_zdd);
            DdStructure<2> dds(sz);
            int max_value = dds.evaluate(MaxEval());
            int num_elements = (option.isEdgeVariable()
                                ? graph.edgeSize() : num_vertices);
            IntRange at_least_k(max_value, num_elements);
            SizeConstraint sc(num_elements, at_least_k);
            DdStructure<2> ddm(sc);
            ZBDD sczdd = ddm.evaluate(ToZBDD());
            ZBDD solution_space_max_zdd = solution_space_zdd & sczdd;
            start_set = pickRandomly(solution_space_max_zdd, random);
        } else {
            start_set = pickRandomly(solution_space_zdd, random);
        }
        if (option.show_info) {
            std::cerr << "Randomly generated the following" << std::endl;
            std::cerr << "s ";
            if (option.isEdgeVariable()) {
                std::set<bddvar> start_set_rev =
                    inverseSet(start_set,graph.edgeSize() + 1);
                printSet(std::cerr, start_set_rev, graph,
                         option.isEdgeVariable());
            } else {
                printSet(std::cerr, start_set, graph,
                         option.isEdgeVariable());
            }
        }
    }

    start_time = getTime();

    Reconf reconf(random, space->getNumElements(), graph,
                  option.isEdgeVariable(),
                  option.show_info, option.is_gc);

    if (option.zdd_dir) {
        reconf.setZddDir(option.zdd_dirname);
    }

    if (option.model == TJ) {
        reconf.setSolutionSpaceZdd(solution_space_zdd);
    } else if (option.model == TAR) {
        reconf.setModelTar();
        reconf.setSolutionSpaceZddTar(solution_space_zdd, option.tar_k);
    }

    if (option.stb_mode) {
        if (option.show_info) {
            std::cerr << "Start searching a reconfiguration sequence "
                      << "from s to t in the bidirection"
                      << std::endl;
        }
        reconf.findReconfSeqBoth(start_set, goal_set);
    } else if (option.st_mode) {
        if (option.show_info) {
            std::cerr << "Start searching a reconfiguration sequence "
                      << "from s to t"
                      << std::endl;
        }
        reconf.findReconfSeq(start_set, goal_set);
    } else if (option.longest_mode) {
        if (option.show_info) {
            std::cerr << "Start searching the longest "
                      << "reconfiguration sequence"
                      << std::endl;
        }
        reconf.findReconfLongestSeq(start_set);
    }

    end_time = getTime();

    if (option.show_info) {
        std::cerr << "Reconfiguration time = "
                  << (end_time - start_time) << std::endl;
    }
    return 0;
}
