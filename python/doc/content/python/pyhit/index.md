# pyhit package

The [input file syntax](input_syntax.md optional=True) MOOSE uses the [!ac](HIT) format. The "pyhit" package
provides tools for reading, writing, and manipulating these files from within python. The package
uses the same underlying C library used by MOOSE. It also relies on the
["moosetree"](python/moosetree/index.md) package.

The complete source documentation for the package is provided in
[pyhit source documentation](python/source/pyhit/pyhit.md) page.

## Example

The example in demonstrates the use of the module to read an input file ("input.i"; see [pyhit-input]), modify
the a parameter, retrieve and modify a comment, and write the modified content to a file using the
built-in format function.

!listing pyhit/tests/input.i id=pyhit-input caption=Input file used for the "pyhit" package example.

The example begins by loading two modules: "pyhit" and "moosetree". The pyhit package provides
a wrapper class (`pyhit.Node`) of the C bindings to the MOOSE [!ac](HIT) library. The wrapper
inherits from `moosetree.Node`, which is a generic python-base tree structure. As such much of
the functionality, including the ability to search the tree, exist in the moosetree package.

After importing the necessary modules the desired input file (see [pyhit-input]) is loaded using
the load function, which returns the root `pyhit.Node` to the tree.

!listing pyhit/tests/test_examples.py id=pyhit-example caption=Example use of "pyhit" package to read, modify, and write MOOSE a moose input file.
         start=MOOSEDOCS:example-begin end=MOOSEDOCS:example-end include-start=False

Once the content is loaded the tree can be search using the `moosetree.find` function. This
function requires the node to search from and a function used for searching. For this example that
function is a simple [lambda expression](https://docs.python.org/3/tutorial/controlflow.html?highlight=lambda#lambda-expressions)
that looks for a node named "gen" that is a child of a node "Mesh".

!alert note
The leading backslash ("/") in the name that is being searched indicates that the node containing
the "Mesh" node is unnamed, which is typical of root nodes.

After this node is located it is modified in two ways: the "x_max" parameter is altered and
the comment for this parameter is changed to note the alteration.

Finally, the modified output is written to a new file using the `pyhit.write` function. The
modifyied content (the "Mesh" block) is shown in [pyhit-output].


!listing id=pyhit-output caption=The modified "Mesh" block that is output to the "input_modified.i" file.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    xmax = 4 # Changed from 3 to 4
  []
[]
