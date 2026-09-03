# ddreconf

A ZDD-based solver for various reconfiguration problems.

Given a graph and two solutions s and t of a combinatorial problem
(independent set, matching, spanning tree, ...), ddreconf builds a ZDD
representing the whole solution space and then searches the
reconfiguration graph with ZDD operations.

## Usage

To clone the repository, use `git clone` with `--recursive` option as follows because the program uses git submodule.

```bash
git clone --recursive https://github.com/junkawahara/ddreconf.git
```

To compile the program, run the following:

```bash
make
```

To run the program,

```bash
./ddreconf <graph_file> [option] [model] [sol_kind]
```

Running `./ddreconf` without an argument prints the list of the options.

## Input format

`<graph_file>` is a file in [DIMACS format](https://core-challenge.github.io/2022/#file-format).
Empty lines are ignored, and so are lines beginning with `c`.

|Line|Meaning|
----|----
|`p <n> <m>`|The graph has `n` vertices and `m` edges. The standard DIMACS header `p edge <n> <m>` is also accepted. This line must come first.|
|`e <v1> <v2>`|An edge between `v1` and `v2`. The vertices are numbered 1 to `n`, and the `i`-th `e` line is edge `i` (edge numbers begin at 1).|
|`s <e1> <e2> ...`|The starting set.|
|`t <e1> <e2> ...`|The target set.|
|`r <v1> <v2> ...`|The roots of a rooted spanning forest, or the terminals of a Steiner tree/subgraph/cycle. e.g. `r 3 6 12`|
|`y <edge> <color>`|The color of an edge, used by `--rainbow`. At most 64 colors are supported. An edge with no `y` line has no color and is never rejected.|

The elements of the `s` and `t` lines are vertex numbers for the
vertex-variable objects and edge numbers for the edge-variable ones (see
the table of the reconfiguration objects below). They can also be given
in a separate file with `--stfile=<file_name>`, which contains the `s`
and `t` lines only; the `s` and `t` lines of `<graph_file>` are then
ignored.

## Output

The reconfiguration sequence goes to the standard output, and the
progress information to the standard error.

```
a YES                  the sequence exists ("a NO" if it does not)
a 3 6 7                s
a 1 6 7
a 1 5 7
a 4 5 7                t
```

Every `a` line but the first lists one solution; consecutive solutions
differ by one move of the chosen model. `--longest` prints the sequence
without the `a YES` line, and `--stwidth` also prints the width to the
standard error.

## Options

* `--randstart` Generates s (starting set) randomly.
* `--randmax` When generating s (the starting set) randomly, generates the starting set with the maximum number of elements. Must be used with the `--randstart` option.
* `--st` Finds the shortest reconfiguration sequence from s to t.
* `--bidirectional` `--bidir` Finds the shortest reconfiguration sequence from s to t, like `--st`, but searches from both ends at once. It alternately advances one step from the s side and one step from the t side, and stops as soon as the two sides reach a common solution. Each side then covers only about half of the distance, which is usually faster and uses less memory than `--st`. `--stb` is the old name of this option and still works.
* `--stw` `--stwidth` Finds the shortest reconfiguration sequence from s to t and computes the width of the solution space graph, that is, the largest number of solutions that are on a shortest sequence and at the same distance from s.
* `--stfile=<file_name>` Specifies a file containing the data for s and t.
* `--longest` Finds a longest shortest-path from s, that is, a shortest sequence from s to a solution farthest from s (t is ignored).
* `--enum` Prints all the elements of the solution space, one per line, as the vertex/edge numbers of the input file.
* `--rainbow` Requires the edges of a solution to have distinct colors, given by the `y` lines. Supported for `--forest`, `--tree`, `--sptree`, `--rspforest` and `--sttree`.
* `--zdddir=<dir>` Saves the intermediate ZDDs of a long `--st` search into files to reduce the memory usage. `<dir>` is prefixed to the file names, so it normally ends with a separator (e.g. `--zdddir=/tmp/zdd/`). Ignored for `--bidirectional` and `--stwidth`.
* `--gc` Runs the garbage collection of the ZDD library periodically.
* `--vorder=<order>` Specifies the ZDD variable order of vertices. `<order>` is one of `leave` (default; the order in which vertices leave the frontier), `asc` (ascending order of vertex numbers from the ZDD root), and `desc` (descending order). Supported only for `--indset`, `--clique`, `--vc`, and `--ds`.
* `--vorderfile=<file>` Specifies the ZDD variable order of vertices by a file. The file contains a permutation of the vertex numbers 1, ..., n, separated by whitespace or newlines, listed from the ZDD root side (that is, the first listed vertex is decided first). Lines beginning with `c` are ignored as comments. Supported only for `--indset`, `--clique`, `--vc`, and `--ds`.
* `--vorderout=<file>` Writes the vertex order actually used to `<file>` in the same format as `--vorderfile`. This is useful for obtaining the default (`leave`) order as a starting point for experiments. Supported only for `--indset`, `--clique`, `--vc`, and `--ds`.
* `--info` Outputs detailed information about the program execution. On by default.
* `-q` `--quiet` Does not output detailed information about the program execution.

If none of `--st`, `--bidirectional`, `--stwidth` and `--longest` is
specified, only the solution space ZDD is constructed (which `--enum`
can print).

### Model

|Option|Model|
----|----
|`--tj`|Token jumping model (default): one element is replaced by another one at each step|
|`--ts`|Token sliding model (not implemented yet)|
|`--tar=<k>`|Token addition/removal model: one element is added or removed at each step, and every solution must have at least `k` elements (`<k>` is an integer at least 0)|

### Reconfiguration object (sol_kind)

Specify as an option. The second column says whether the solution is a
set of vertices or of edges, which is also how the `s` and `t` lines are
read.

|Option|Variables|Object|
----|----|----
|`--indset`|vertex|Independent set (default)|
|`--clique`|vertex|Clique|
|`--vc`|vertex|Vertex cover|
|`--cvc`|vertex|Connected vertex cover|
|`--ds`|vertex|Dominating set|
|`--cds`|vertex|Connected dominating set|
|`--cisv`|vertex|Vertex set inducing a connected subgraph (the empty set excluded)|
|`--matching`|edge|Matching|
|`--cmatching`|edge|Complete (perfect) matching|
|`--path`|edge|Path (at least one edge)|
|`--tree`|edge|Tree (at least one edge)|
|`--sptree`|edge|Spanning tree|
|`--forest`|edge|Forest (the empty set included)|
|`--rspforest`|edge|Rooted spanning forest: every component contains exactly one of the roots given by the `r` line|
|`--sttree`|edge|Steiner tree over the terminals given by the `r` line|
|`--stsub`|edge|Steiner subgraph|
|`--stcycle`|edge|Steiner cycle|
|`--cise`|edge|Edge set of a connected induced subgraph (the empty set excluded)|
|`--sptree --rainbow`|edge|Rainbow spanning tree|

## Limitations

* The number of variables (the number of vertices for the vertex-variable
  objects, the number of edges for the edge-variable ones) must be less
  than 8192, which is a restriction of the SAPPOROBDD library.
* The edge-variable objects do not accept a graph with a duplicated edge,
  because the edge numbering of the input file would no longer match the
  one the solver uses. A reversed pair such as `e 1 2` and `e 2 1` counts
  as a duplicate.
* The token sliding model (`--ts`) is not implemented.

## Example

The following finds a reconfiguration sequence from a given s to t.

```bash
./ddreconf sample/hc-toyyes-01.col --indset --st --stfile=sample/hc-toyyes-01_01.dat
```

It finds a longest reconfiguration sequence from a given s (t in the dat file is ignored).

```bash
./ddreconf sample/hc-square-01.col --indset --longest --stfile=sample/hc-square-01_01.dat
```

It finds a longest reconfiguration sequence from a random s.

```bash
./ddreconf sample/hc-square-01.col --indset --randstart --longest
```

It prints all the spanning trees of a graph.

```bash
./ddreconf sample/hc-square-01.col --sptree --enum -q
```

## License

[MIT License](LICENSE)

This work was supported by JSPS KAKENHI Grant Numbers JP18H04091, JP20H05792, and JP20H05794.
