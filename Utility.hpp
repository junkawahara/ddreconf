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

// Manages the mapping between outer vertex numbers (in the DIMACS file)
// and ZDD variables for vertex-variable problems.
class VertexMapping {
private:
    std::vector<int> vertex_to_var_; // outer vertex number -> ZDD variable
    std::vector<int> var_to_vertex_; // ZDD variable -> outer vertex number

public:
    // graph must have been updated
    void initialize(const Graph& graph, int num_vertices,
                    VertexOrder vertex_order,
                    const std::string& order_filename)
    {
        vertex_to_var_.assign(num_vertices + 1, 0);
        var_to_vertex_.assign(num_vertices + 1, 0);

        switch (vertex_order) {
        case VO_ASC:
            // vertex 1 is nearest to the ZDD root (variable num_vertices)
            for (int v = 1; v <= num_vertices; ++v) {
                vertex_to_var_[v] = num_vertices + 1 - v;
            }
            break;
        case VO_DESC:
            for (int v = 1; v <= num_vertices; ++v) {
                vertex_to_var_[v] = v;
            }
            break;
        case VO_FILE:
            readOrderFile(order_filename, num_vertices);
            break;
        default: // VO_LEAVE
            // The ZDD variable of a vertex is its inner vertex number.
            // Vertices not appearing in any edge have no inner vertex
            // number and are assigned to the remaining variables in
            // ascending order.
            for (int v = 1; v <= graph.vertexSize(); ++v) {
                vertex_to_var_[getVertexNumber(graph, v)] = v;
            }
            {
                int var = graph.vertexSize();
                for (int v = 1; v <= num_vertices; ++v) {
                    if (vertex_to_var_[v] == 0) {
                        vertex_to_var_[v] = ++var;
                    }
                }
            }
            break;
        }
        for (int v = 1; v <= num_vertices; ++v) {
            var_to_vertex_[vertex_to_var_[v]] = v;
        }
    }

    // translate an outer vertex number into the corresponding ZDD variable
    int outerToVar(int v) const
    {
        if (!(1 <= v && v < static_cast<int>(vertex_to_var_.size()))) {
            std::cerr << "Vertex number " << v << " is out of range."
                      << std::endl;
            exit(1);
        }
        return vertex_to_var_[v];
    }

    // translate a ZDD variable into the corresponding outer vertex number
    int varToOuter(bddvar var) const
    {
        if (!(1 <= var && var < var_to_vertex_.size())) {
            std::cerr << "ZDD variable " << var << " is out of range."
                      << std::endl;
            exit(1);
        }
        return var_to_vertex_[var];
    }

    // translate an inner vertex number (in tdzdd::Graph)
    // into the corresponding ZDD variable
    int innerToVar(const Graph& graph, int v) const
    {
        return outerToVar(getVertexNumber(graph, v));
    }

    std::set<bddvar> outerSetToVarSet(const std::set<bddvar>& s) const
    {
        std::set<bddvar> ns;
        for (std::set<bddvar>::const_iterator itor = s.begin();
             itor != s.end(); ++itor) {
            ns.insert(outerToVar(*itor));
        }
        return ns;
    }

    // Write the vertex order to a file in the same format as the
    // --vorderfile option (from the ZDD root side).
    void writeOrderFile(const std::string& filename) const
    {
        std::ofstream ofs(filename.c_str());
        if (!ofs) {
            std::cerr << "File " << filename << " cannot be opened."
                      << std::endl;
            exit(1);
        }
        int n = static_cast<int>(var_to_vertex_.size()) - 1;
        for (int var = n; var >= 1; --var) {
            if (var != n) {
                ofs << " ";
            }
            ofs << var_to_vertex_[var];
        }
        ofs << std::endl;
    }

private:
    void readOrderFile(const std::string& filename, int num_vertices)
    {
        std::ifstream ifs(filename.c_str());
        if (!ifs) {
            std::cerr << "File " << filename << " cannot be opened."
                      << std::endl;
            exit(1);
        }
        std::vector<int> order;
        std::string line;
        while (ifs && std::getline(ifs, line)) {
            if (line.empty() || line[0] == 'c') { // skip comment line
                continue;
            }
            std::istringstream iss(line);
            int v;
            while (iss >> v) {
                order.push_back(v);
            }
        }
        if (static_cast<int>(order.size()) != num_vertices) {
            std::cerr << "The vertex order file must contain "
                      << num_vertices << " vertex numbers, but contains "
                      << order.size() << " numbers." << std::endl;
            exit(1);
        }
        for (size_t k = 0; k < order.size(); ++k) {
            int v = order[k];
            if (!(1 <= v && v <= num_vertices)) {
                std::cerr << "Vertex number " << v
                          << " in the vertex order file is out of range."
                          << std::endl;
                exit(1);
            }
            if (vertex_to_var_[v] != 0) {
                std::cerr << "Vertex number " << v << " appears twice "
                          << "in the vertex order file." << std::endl;
                exit(1);
            }
            // the first listed vertex is nearest to the ZDD root
            vertex_to_var_[v] = num_vertices - static_cast<int>(k);
        }
    }
};

// graph: output graph
// returned value: number of vertices
// The numbers in the s/t lines are stored in start_set/goal_set as is.
// The caller must translate them into ZDD variables
// (by VertexMapping::outerSetToVarSet or inverseSet).
int parse_DIMACS(std::istream& ist, Graph* graph,
                 std::set<bddvar>* start_set,
                 std::set<bddvar>* goal_set,
                 std::set<std::string>* root_set,
                 std::vector<int>* colors)
{
    int num_vertices = -1;
    int num_edges = -1;
    int current_num_edges = 0;

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
            if (num_edges < 0 || current_num_edges < num_edges) {
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
                ++current_num_edges;
                //graph->update(); // too slow
            }
        } else if (s[0] == 's' || s[0] == 't') {
            std::set<bddvar>* vec = (s[0] == 's' ? start_set : goal_set);
            std::istringstream iss(s);
            std::string st;
            bddvar bv;
            iss >> st; // skip first char
            while (iss >> bv) {
                vec->insert(bv);
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
            if (num_edges < 0) {
                std::cerr << "'y' line must appear after 'p' line"
                          << std::endl;
                exit(1);
            }
            std::string dummy;
            std::istringstream iss(s);
            int edge_number;
            int color;
            if (!(iss >> dummy >> edge_number >> color)) {
                std::cerr << "illegal input format in line "
                          << line_number << std::endl;
                exit(1);
            }
            if (!(1 <= edge_number && edge_number <= num_edges)) {
                std::cerr << "Edge number " << edge_number << " in line "
                          << line_number << " is out of range." << std::endl;
                exit(1);
            }
            (*colors)[edge_number] = color;
        } else {
            std::cerr << "illegal input format in line "
                      << line_number << std::endl;
            exit(1);
        }
    }
    graph->update();

    return num_vertices;
}

int parse_DIMACS(const char* filename, Graph* graph, std::set<bddvar>* start_set,
                 std::set<bddvar>* goal_set, std::set<std::string>* root_set,
                 std::vector<int>* colors)
{
    std::ifstream ifs;
    ifs.open(filename);
    if (!ifs) {
        std::cerr << "File " << filename << " cannot be opened." << std::endl;
        exit(1);
    }
    return parse_DIMACS(ifs, graph, start_set, goal_set, root_set, colors);
}

// The numbers in the s/t lines are stored as is (see parse_DIMACS).
void parse_stfile(const char* filename, std::set<bddvar>* start_set,
                  std::set<bddvar>* goal_set)
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
                vec->insert(bv);
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
              const VertexMapping& vertex_mapping, bool is_edge_variable)
{
    std::set<bddvar>::iterator itor = s.begin();
    for ( ; itor != s.end(); ++itor) {
        if (itor != s.begin()) {
            ost << " ";
        }
        if (is_edge_variable) {
            ost << *itor;
        } else {
            ost << vertex_mapping.varToOuter(*itor);
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
