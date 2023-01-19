# moosetree package

The "moosetree" package is a general purpose for creating tree structures in python. The need for
this package stems from the [MooseDocs](MooseDocs/index.md) package, which uses tree objects
extensively.  Originally, MooseDocs used the [anytree](https://anytree.readthedocs.io/en/latest/)
package for these structures. As the MooseDocs system evolved as well as the amount of documentation,
in particular the amount of generated HTML output, the speed in creating the tree nodes became
critical. The anytree package is robust and well designed, but the construction of the nodes was not
fast enough.

This object mimics the behavior of anytree but does not perform any sanity checks, the resulting
construction speedup is 2 to 3 times that of anytree. At the time of writing this document,
"moosetree" was able to instantiate 1e6 nodes in 2.5 sec., whereas "anytree" required 6.0 sec. This
test can be executed by running the tests for the "moosetree" package with "anytree" installed.

## Node

The "moosetree" package contains a single `Node` object from which to create a tree structure. The
complete documentation for this class is given on the [python/source/moosetree/Node.md] page.

## Search and Iteration

The package includes various functions for locating and iterating through the tree, please
refer to the [python/source/moosetree/search.md] for details.
