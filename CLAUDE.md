# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

ddreconf is a ZDD-based solver for combinatorial reconfiguration problems, written in C++11. It reads a graph in DIMACS format, builds a ZDD representing the "solution space" (all independent sets, matchings, spanning trees, etc.), and then searches the reconfiguration graph (token jumping / token addition-removal models) using ZDD operations.

## Build

The project depends on three git submodules (SAPPOROBDD, TdZdd, sbdd_helper). They must be initialized before building:

```bash
git submodule update --init --recursive
make            # produces ./ddreconf
make clean
```

There is no test suite and no lint configuration. To smoke-test a change, run against the sample inputs:

```bash
./ddreconf sample/hc-toyyes-01.col --indset --st --stfile=sample/hc-toyyes-01_01.dat
./ddreconf sample/hc-square-01.col --indset --longest --stfile=sample/hc-square-01_01.dat
```

Running `./ddreconf` with no arguments prints the option list; README.md documents the same set in more detail. Keep both in sync when adding an option.

## Architecture

Almost the entire program is header-only and compiled as a single translation unit: main.cpp `#include`s all the `.hpp` files. The only separately compiled objects are `bddc_reconf.c`, `ZBDD_reconf.cpp`, and SAPPOROBDD's `BDD.cc` (see Makefile). Adding a new header means adding it to `HEADER` in the Makefile and including it from main.cpp.

The pipeline in main.cpp:

1. **Parsing** — `Utility.hpp` (`parse_DIMACS`, `parse_stfile`) reads the graph, optional s/t sets, root vertices (`r` lines), and colors. `Option.hpp` parses command-line flags into `Option`.
2. **Solution space construction** — `SolutionSpace.hpp` defines the abstract base class with one virtual method, `createSolutionSpaceZdd()`. Each problem kind subclasses it: `IndependentSet` (also handles vertex cover via a flag), `Clique`, `DominatingSet`, `Matching`, `Path`, `ForestOrTree` (forest/tree/spanning tree/rooted spanning forest/Steiner tree, all via constructor flags), `SteinerSubgraph`, `ConnectedInducedSubgraph`. Most are implemented as TdZdd frontier-based `DdSpec`s (`FrontierManager.hpp` manages frontier vertex bookkeeping; `*Spec.hpp` files hold the specs), then converted to a SAPPOROBDD `ZBDD` with TdZdd's `ToZBDD` / `SapporoZdd`.
3. **Reconfiguration search** — `Reconf.hpp` does BFS over the solution space graph entirely with ZDD operations: `findReconfSeq` (s→t), `findReconfSeqBoth` (bidirectional), `findReconfSeqAndWidth`, `findReconfLongestSeq`. The core step operator is `swapElement` (`ZBDD_reconf.hpp/cpp` on top of `bddc_reconf.c`), a custom operation added at SAPPOROBDD's C level that computes all sets reachable by one token move.

## Key conventions and constraints

- **Two variable regimes**: problems are either vertex-variable (indset, clique, vc, ds) or edge-variable (matching, trees, forests, Steiner). `option.isEdgeVariable()` switches behavior everywhere — parsing, output, and variable mapping. For edge-variable problems, edge i (1-based, in input order of `e` lines) is mapped to SAPPOROBDD variable m+1−i via `inverseSet`; remember to invert again before printing.
- **Size limit**: the number of variables (vertices or edges, per the regime) must be < 8192 (`recursion_limit`, a SAPPOROBDD restriction enforced in main.cpp).
- **`STAND_ALONE` define**: main.cpp has an alternate mode (graph from stdin, forced `--st`, quiet) compiled with `-DSTAND_ALONE`; keep both paths working when editing main.cpp.
- Solver progress/info goes to **stderr**; the reconfiguration sequence itself (`a` lines) goes to **stdout**.
- Token sliding (`--ts`) is not implemented; `Option::parse` rejects it.
