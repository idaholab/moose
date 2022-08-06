# The `hit` command

*Hierarchial Input Text* (HIT) is the file format that MOOSE input files (and
`tests` specs) are built upon. HIT is a key-value pair syntax with multilevel
block hierarchies. To deal with HIT files we provide the `hit` executable, which
is automatically built under `framework/contrib/hit`. This command line tool
offers multiple functions for

- Finding parameters across many input files in a context sensitive way
- Validating input files
- Formatting input files according to a user defined style
- Merging input files
- Diffing input files
- Extracting common portions of input files
- Subtracting out common portions of input files

## `hit find` - Searching for parameters

```
hit find [-i] [-v] [-p additional_pattern [...] --] main_pattern files ...
```

`hit find` looks for a full parameter path that can be specified with wildcards
as `main_pattern`. `*` will match an arbitrary set of characters and `?` will
match a single character. In the input

```
[Kernels]
  [diff]
    type = Diffusion
    variable = a
  []
[]
```

the main pattern `Kernels/*/variable` will match the variable line, as will
`Kernels/*/variable=a`, `*iable=?`, and `*/diff/*=a`, but `Kernels/*/variable=b`
will not match.

An `!=` operator is also available for negative matching. In the example above
`Kernels/*/variable!=b` will match the variable line.

The `-i` option will turn all matching case insensitive (useful to match
MooseEnum values).

The `-v` option will list all files that do not have a single match, which can
be used to check for the absence of patterns.

The `-p` option will match additional parameters inside the blocks matched by
the main pattern. For example `hit find -p type=Diffusion -- Kernels/*/variable=a`
will match the snippet above, and any kernel acting on the variable `a` that has
`type = Diffusion` (and so would `hit find -p type!=NeumannBC -- Kernels/*/variable=a`)

### Examples

```
hit find Outputs/file_base **/*.i
```

Will find all inputs that contain the `file_base` parameter in the `[Outputs]` block.

```
hit find Outputs/file_base **/*.i
```

```
hit find -v Modules/TensorMechanics/Master/* **/*.i
```

Will find all files that do not (`-v`) contain use of the TensorMechanics master
action.

```
hit find BCs/*/boundary=left **/*.i
```

Will find all boundary conditions that are applied to just the `left` sideset.

```
hit find BCs/*/boundary!=left **/*.i
```

Will find all boundary conditions that are *not* applied to just the `left` sideset.

```
hit find -p type=FunctionDirichletBC -- BCs/*/boundary=\*right\* **/*.i
```

Will find all `FunctionDirichletBC`s that are applied to the `right` sideset
(and possibly others). This will also match `boundary=inner_right` so some
filtering of the results with `grep` may be necessary.

```
hit find -i -p formulation=mortar mortar_approach -- Contact/*/model=coulomb **/*.i
```

Will find all inputs that contain a `Contact` action block with `model = COULOMB`
(case insensitive match using `-i`) and have the `formulation` parameter set to
`mortar` *and* have also specified a `mortar_approach` parameter (with whatever
value). Not that multiple arguments are supplied to the `-p` parameter and the
list is terminated with a double dash `--`, after which the main pattern and the
list of files to search follow.

## `hit format` - Formatting input files

```
hit format [-i] [-style file] input
```

The format subcommand will reformat a valid HIT file into a cannonical form with
a consistent indentation and potentially sorted sections and parameters. The
name of the file to be formatted is given as the `input` parameter. When
specifying `-` as the filename the input is taken from stdin.

The formatted input will be output to terminal unless the `-i` option is supplied,
which enables inplace formatting of the supplied `input` file.

The formatting style can be user defined using the `-style` parameter. The
argument is the name of a style file of the format:

```
    [format]
        indent_string = "  "
        line_length = 100
        canonical_section_markers = true

        [sorting]
            [pattern]
                section = "[^/]+/[^/]+"
                order = "type"
            []
            [pattern]
                section = ""
                order = "Mesh ** Executioner Outputs"
            []
        []
    []
```

where all fields are optional and the sorting section is also optional.  If the
sorting section is present, you can have as many patterns as you want, but each
pattern section must have 'section' and 'order' fields.

Note that single `'` and double `"` quotes behave differently when auto-formatting input files with `hit format`.
Single quotes strings will not be reformatted, while double quoted strings are reindented
and reflowed. *Multiline strings* will only be reindented through inserting or removing spaces on the lines
following the first line, to match any changes in indentation of the first line. *Single line strings*
will be reflowed according to the set `line_length`.

## `hit merge` - Combining input files

```
hit merge -output outfile.i infile1.i infile2.i ...
```

will produce a single file `outfile.i` that is a merge of the `infile*.i` inputs
(which would result in the same MOOSE simulation as the multiple input files).

## `hit diff` - Highlighting differences between input files

```
hit diff left.i right.i
```

or

```
hit diff -left left1.i left2.i ... -right right1.i right2.i ...
```

performs a diff on the left and right files (in the second example merging all
left and right files respectively first). This diff is not sensitive to order,
formatting, or comments in the input files.

## `hit common` - Extract common parameters between inputs

```
hit common file1.i file2.i ... > common_parameters.i
```

Will extract all parameters that are common to the specified input files (and
have the same values). This can be used as the **first step** in factoring out
common settings into a single input file.

## `hit subtract` - Removing common parameters parameters

```
hit subtract simulation_1_full.i common_parameters.i > simulation_1.i
```

removes the parameters in `common_parameters.i` from `simulation_1_full.i`,
creating `simulation_1.i`. This is the **second step** in factoring out common
parameters from a set of input files. The resulting file can be run as
`./mooseapp-opt -i common.i simulation_1.i` and will result in the same
simulation as `./mooseapp-opt -i simulation_1_full.i`.
