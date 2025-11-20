# ParallelRoutingDynamicGraph

## Project Overview
Please refer to this doc for what this project is about: https://docs.google.com/document/d/1eNZh5MjM0KwcqvB14qrfGHoRefIECy8Bp2h_8brXHOQ/

## How to run
Run `make` to compile the dynamicgraph.o library and all test cases in the `./test` directory.

To run k-core test cases with `k = K` and `NUM_PROC` number of threads, run `./testcase -k K -n NUM_PROC`. For example, `./tests/graph_size/kcore/4096_all -k 10000 -n 8` finds all subgraphs with degree at least 10000 in a fully-connected graph with 4096 vertices using 8 threads in parallel.

We are yet to publish the parallel version for multiple packages routing.

Run `make clean` to clean up all compiled files.

## Test Cases
All test cases we provided can be found in the `./tests` subdirectory. Note that some test cases tests k-core, while some others tests multiple packages routing, and they should be ran using different flags.
