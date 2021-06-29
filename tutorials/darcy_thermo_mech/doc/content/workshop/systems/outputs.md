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
!include output_types.md
