//
// Utility.hpp
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

#ifndef UTILITY_HPP
#define UTILITY_HPP

inline double getTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

std::string getCard(ZBDD f)
{
    SapporoZdd s(f);
    DdStructure<2> dd(s);
    return dd.zddCardinality();
}

//inline ZBDD removeElement(const ZBDD& f)
//{
//    return ZBDD_ID(remove_element(f.GetID()));
//}

//inline ZBDD addElement(const ZBDD& f, int n)
//{
//    return ZBDD_ID(add_element(f.GetID(), n));
//}

//inline ZBDD swapElement(const ZBDD& f, int n)
//{
//    return ZBDD_ID(swap_element(f.GetID(), n));
//}

bool checkVertexFormat(const std::string& vertex, int num_vertices)
{
    for (size_t i = 0; i < vertex.length(); ++i) {
        if (!std::isdigit(vertex[i])) { // not digit
            return false;
        }
    }
    std::istringstream iss(vertex);
    int v;
    iss >> v;
    if (!(1 <= v && v <= num_vertices)) { // not in [1, n]
        return false;
    }
    return true;
}

// translate an inner vertex number (in tdzdd::Graph)
// into the corresponding outer vertex number
int getVertexNumber(const Graph& graph, int v)
{
    std::string s = graph.vertexName(v);
    std::istringstream iss(s);
    int c;
    iss >> c;
    return c;
}

// translate an outer vertex number
// into the corresponding inner vertex number (in tdzdd::Graph)
int outerVertexToInner(const Graph& graph, int v)
{
    std::stringstream ss;
    ss << v;
    return graph.getVertex(ss.str());
}

// graph: output graph
// returned value: number of vertices
int parse_DIMACS(std::istream& ist, Graph* graph,
                 std::set<bddvar>* start_set,
                 std::set<bddvar>* goal_set,
                 std::set<std::string>* root_set,
                 std::vector<int>* colors,
                 bool is_edge_variable)
{
    int num_vertices = -1;
    int num_edges = -1;
    bool read_s = false;
    bool read_t = false;

    std::string s;
    int line_number = 0;
    while (ist && std::getline(ist, s)) {
        ++line_number;
        if (s[0] == 'c') {
            // skip comment line
        } else if (s[0] == 'p') {
            std::string st;
            std::istringstream iss(s);
            iss >> st; // skip first char
            iss >> num_vertices; // obtain the number of vertices
            iss >> num_edges; // obtain the number of edges
            colors->resize(0);
            colors->resize(num_edges + 1, 0); // +1 for colors[0]. zero padding
        } else if (s[0] == 'e') {
            if (num_vertices < 0) {
                std::cerr << "'e' line must appear after 'p' line"
                          << std::endl;
                exit(1);
            }
            if (num_edges < 0 || graph->edgeSize() < num_edges) {
                std::istringstream iss(s);
                std::string kind, e1, e2;
                iss >> kind >> e1 >> e2;
                if (!checkVertexFormat(e1, num_vertices)
                    || !checkVertexFormat(e2, num_vertices)) {
                    std::cerr << "illegal input format in line "
                              << line_number << std::endl;
                    exit(1);
                }
                graph->addEdge(e1, e2);
                graph->update();
            }
        } else if (s[0] == 's' || s[0] == 't') {
            std::set<bddvar>* vec = (s[0] == 's' ? start_set : goal_set);
            std::istringstream iss(s);
            std::string st;
            bddvar bv;
            iss >> st; // skip first char
            while (iss >> bv) {
                if (is_edge_variable) {
                    vec->insert(bv);
                } else {
                    vec->insert(outerVertexToInner(*graph, bv));
                }
            }
            if (s[0] == 's') {
                read_s = true;
            } else {
                read_t = true;
            }
        } else if (s[0] == 'r') {
            std::istringstream iss(s);
            std::string dummy;
            std::string root;
            iss >> dummy; // skip first char
            while (iss >> root) {
                root_set->insert(root);
            }
        } else if (s[0] == 'y') {
            std::string dummy;
            std::istringstream iss(s);
            int edge_number;
            int color;
            iss >> dummy >> edge_number >> color;
            if (static_cast<int>(colors->size()) < edge_number + 1) {
                colors->resize(edge_number + 1);
            }
            (*colors)[edge_number] = color;
        } else {
            std::cerr << "illegal input format in line "
                      << line_number << std::endl;
            exit(1);
        }
        if (read_s && read_t &&
            num_edges >= 0 && graph->edgeSize() >= num_edges) {
            // We will not read input anymore.
            //std::cerr << "break" << std::endl;
            break;
        }
    }
    graph->update();

    return num_vertices;
}

int parse_DIMACS(const char* filename, Graph* graph, std::set<bddvar>* start_set,
                 std::set<bddvar>* goal_set, std::set<std::string>* root_set,
                 std::vector<int>* colors, bool is_edge_variable)
{
    std::ifstream ifs;
    ifs.open(filename);
    if (!ifs) {
        std::cerr << "File " << filename << " cannot be opened." << std::endl;
        exit(1);
    }
    return parse_DIMACS(ifs, graph, start_set, goal_set, root_set, colors, is_edge_variable);
}

void parse_stfile(const char* filename, Graph* graph, std::set<bddvar>* start_set,
                  std::set<bddvar>* goal_set, bool is_edge_variable)
{
    std::ifstream ifs;
    ifs.open(filename);
    if (!ifs) {
        std::cerr << "File " << filename << " cannot be opened." << std::endl;
        exit(1);
    }

    std::string s;
    while (ifs && std::getline(ifs, s)) {
        if (s[0] == 's' || s[0] == 't') {
            std::set<bddvar>* vec = (s[0] == 's' ? start_set : goal_set);
            std::istringstream iss(s);
            std::string st;
            bddvar bv;
            iss >> st; // skip first char
            while (iss >> bv) {
                if (is_edge_variable) {
                    vec->insert(bv);
                } else {
                    vec->insert(outerVertexToInner(*graph, bv));
                }
            }
        }
    }
}

std::set<bddvar> pickRandomly(const ZBDD& f, BigIntegerRandom& random)
{
    ZBDD_CountMap cmap;
    ZBDD_CountSolutions(f, &cmap);
    return ZBDD_SampleRandomly(f, cmap, random);
}

void printSet(std::ostream& ost, const std::set<bddvar>& s,
              const tdzdd::Graph& graph, bool is_edge_variable)
{
    std::set<bddvar>::iterator itor = s.begin();
    for ( ; itor != s.end(); ++itor) {
        if (itor != s.begin()) {
            ost << " ";
        }
        if (is_edge_variable) {
            ost << *itor;
        } else {
            ost << graph.vertexName(*itor);
        }
    }
    ost << std::endl;
}

template<typename T>
T inverseSet(const T& s, int n)
{
    T ns;
    for (typename T::iterator itor = s.begin();
         itor != s.end(); ++itor) {
        ns.insert(n - *itor);
    }
    return ns;
}

#endif // UTILITY_HPP
