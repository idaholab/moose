# Output System

A system for producing outputting simulation data to the screen or files.

!---

The output system is designed to be just like any other system in MOOSE: modular and expandable.

It is possible to create multiple output objects for outputting:

- at specific time or timestep intervals,
- custom subsets of variables, and
- to various file types.

There exists a short-cut syntax for common output types as well as common parameters.

!---

## Short-cut Syntax

The following two methods for creating an Output object are equivalent within the internals of MOOSE.

```text
[Outputs]
  exodus = true
[]
```

```text
[Outputs]
  [out]
    type = Exodus
  []
[]
```

!---

## Common Parameters

```text
[Outputs]
  interval = 10
  exodus = true
  [all]
    type = Exodus
    interval = 1 # overrides interval from top-level
  []
[]
```

!---

## Output Names

The default naming scheme for output files utilizes the input file name (e.g., input.i) with a suffix
that differs depending on how the output is defined: An "_out" suffix is used for Outputs created
using the short-cut syntax.  sub-blocks use the actual sub-block name as the suffix.

```text
[Outputs]
  exodus = true    # creates input_out.e
  [other]          # creates input_other.e
     type = Exodus
     interval = 2
  []
  [base]
    type = Exodus
    file_base = out # creates out.e
  []
[]
```

The use of 'file_base' anywhere in the `[Outputs]` block disables all default naming behavior.

!---

!style fontsize=85%
| Short-cut | Sub-block ("type=") | Description |
| :- | :- | :- |
| console    | Console | Writes to the screen and optionally a file |
| exodus     | Exodus  | The most common,well supported, and controllable output type |
| vtk        | VTK     | Visual Analysis Toolkit format, requires `--enable-vtk` when building libMesh|
| gmv        | GMV     | General Mesh Viewer format |
| nemesis    | Nemesis | Parallel ExodusII format |
| tecplot    | Tecplot | Requires `--enable-tecplot` when building libMesh |
| xda        | XDA     | libMesh internal format (ascii) |
| xdr        | XDR     | libMesh internal format (binary) |
| csv        | CSV     | Comma separated scalar values |
| gnuplot    | GNUPlot | Only support scalar outputs |
| checkpoint | Checkpoint | MOOSE internal format used for restart and recovery |
| solution_history | SolutionHistory | MOOSE internal format used for writing solution history |
