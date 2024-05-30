# [Output System](syntax/Outputs/index.md)

A system for outputting simulation data to the screen or files.

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

## Customizing Output

The content of each `Output` can customized, see for example for an [Exodus](Exodus.md) output:

```
[Outputs]
  [out]
    type = Exodus
    output_material_properties = true
    # removes some quantities from the output
    hide = 'power_pp pressure_var'
  []
[]
```

!---

## Common Parameters

```text
[Outputs]
  interval = 10 # this is a time step interval
  [exo]
    type = Exodus
    interval = 1 # overrides interval from top-level
  []
  [cp]
    type = Checkpoint # Uses interval specified from top-level
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

!---

!style fontsize=85%
!include output_types.md

Paraview can read many of these (CSV, Exodus, Nemesis, VTK, GMV)
