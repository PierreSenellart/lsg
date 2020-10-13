# lsg: Large sparse graph library

The Large Sparse Graph library is a C++ library for working with large sparse graphs (on disk, without loading them in memory). In particular, it includes methods used for computing related nodes in a graph, as discussed in a [AAAI 2007 paper](https://pierre.senellart.com/publications/ollivier2007finding/).

## Prerequisites

GNU make, a reasonably recent C++ compiler. The library has only been
tested on i386-linux, x86_64-linux, and sun4u-sunos9 architectures, but
should work with other architectures, possibly with minor changes.

## Compilation

"make" should be enough to compile the graph library lsg/lsg.a, as well
as the executables.

## Tests

./RunTests run all test units. Every test should pass. Note that the
compilation of RunTests uses libtut, the Test Unit Framework (provided).

## Executables
### BuildGraphFromEdgeList

  Build a PackedGraph from a file containing edges of the graph and a
file with node labels (one per line). The edge file must be in either of
the following format:

```
3
no values
0 1 2
1 2
2 0
```
  
or:
```
3
with values
0 1,1 2,3
1 2,3
2 0,1
```
(the second format allows weighted edges)

### ComputeInvariantMeasure
  Compute the equilibrium measure of a strongly connected stochastic
graph.

### DumpSampleFiles
  Test program for dumping XML graphs of the different steps of each
"Related Nodes" method.

### ExtractFirstSCC
  Extract the main strongly component of a graph.

### Idftrans
  Modify a graph by amplifying transition probabilities by log(1/nu_i),
where nu is the equilibrium measure.
  
### Normalize
  Stochastify a graph.

### PageRank
  Compute PageRank over a graph.

### RelatedPages
  Computed "Related Nodes" over a graph, through various different
methods.

### Reverse
  Compute the reversed Markov chain (with respect to a measure).

### Statistics
  Give some basic statistics about a graph.

### Symmetrize
  Compute the symmetrized Markov chain (with respect to a measure).

### TextVector2BinaryVector
  Convert between the two different representations of a (Row)Vector.

## License

ProvSQL is provided as open-source software under the MIT License. See [LICENSE](LICENSE).

## Contact

https://github.com/PierreSenellart/lsg

[Pierre Senellart](https://pierre.senellart.com/) <pierre@senellart.com>

This is joint work with [Yann Ollivier](http://www.yann-ollivier.org/)

Bug reports and feature requests are
preferably sent through the *Issues* feature of GitHub.
