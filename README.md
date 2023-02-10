# ddreconf

A ZDD-based solver for various reconfiguration problems.

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

<graph_file> is the graph file path in [DIMACS format](https://core-challenge.github.io/2022/#file-format).

## Options

* `--randstart` Generates s (starting set) randomly.
* `--randmax` When generating s (the starting set) randomly, generates the starting set with the maximum number of elements. Must be used with the `--randstart` option.
* `--st` Finds the reconfiguration sequence from s to t.
* `--stfile=<file_name>` Specifies a file containing the data for s and t.
* `--longest` Finds the longest reconfiguration sequence from s (t is ignored).
* `--rainbow` Option for rainbow-spanning tree.
* `--info` Outputs detailed information about the program execution. On by default.
* `-q` `--quiet` Does not output detailed information about the program execution.

s (start set) and t (target set) can be specified with `--stfile=<file_name>` or directly in <graph_file> directly.

If neither `--st` nor `--longest` is specified, the
Only solution space ZDDs are constructed.

### Model

|Option|Model|
----|----
|`--tj`|Token jumping model (Default)|
|`--ts`|Token sliding model (Not implemented yet)|
|`--tar=<k>`|Token addition/removal model (<k> is an integer at least 0)|

### Reconfiguration object (sol_kind)

Specify as an option.

|Option|Object|
----|----
|`--indset`|Independent set (Default)|
|`--clique`|Clique|
|`--matching`|Matching|
|`--tree`|Tree|
|`--sptree`|Spanning tree|
|`--forest`|Forest|
|`--rspforest`|Rooted spanning forest|
|`--sttree`|Steiner tree|
|`--sptree --rainbow`|Rainbow spanning tree|

The roots of the rooted spanning forest and the terminal of the Steiner tree are
<graph_file> file, specified in the 'r' line of the file,
followed by the vertex number.
e.g., `r 3 6 12`

If the vertices are variables (independent set), s (starting set) and t (target set) are arranged in columns of vertex numbers.
If the edges are variables (matching, tree, forest, etc.), s (the starting set) and t (the target set) are arranged in columns of edge numbers.
The i-th edge is represented by the i-th `e <start point> <end point>` row. Edge numbers begin at 1.

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

## License

[MIT License](LICENSE)

This work was supported by JSPS KAKENHI Grant Numbers JP18H04091, JP20H05792, and JP20H05794.
