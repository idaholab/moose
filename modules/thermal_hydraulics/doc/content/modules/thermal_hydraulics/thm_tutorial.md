# Execution

An application executable is typically located at the root level of a repository,
for example, building THM (with the default mode, `opt`) creates the `thm-opt`
executable.

Suppose an executable is called `/path/to/executable`. Then an input file
`my_input_file.i` can be run as follows:

```
/path/to/executable -i my_input_file.i
```

To run with multiple threads, use the `--n-threads` argument:

```
/path/to/executable -i my_input_file.i --n-threads=4
```

See [command line usage](/command_line_usage.md) for more information.

# Output

Output can be in a number of different formats (see [/Outputs/index.md]), but
the most common is the Exodus II format. This can be viewed by a number of
different applications, including VisIt and Paraview. A basics tutorial for
Paraview can found [here](paraview_basics.md).
