# Postprocessor System

A system for computing a "reduction" or "aggregation" calculation based on the solution variables
that results in a +single+ scalar value.

!---

## Types of Postprocessors

ElementPostprocessor: operate on each element

NodalPostprocessor: operate on each node

SidePostprocessor: operate on boundaries

InternalSidePostprocessor: operate on internal element sides

InterfacePostprocessor: operator on subdomain interfaces

GeneralPostprocessor: operates once per execution

!---

## Postprocessor Anatomy

`Postprocessor` is a UserObject, so `initialize`, `execute`, `threadJoin`, and `finalize` methods
are used for implementing the aggregation operation.

`Real getValue()`\\
This is called internally within MOOSE to retrieve the final scalar value, the value returned by
this function is referenced by all other objects that are using the postprocessor.

!---

## Aggregation Routines

If the Postprocessor created has custom data it must be ensured that the value is communicated
properly in (both MPI and thread-based) parallel simulations.

For MPI several utility methods exist to perform common aggregation operations:

- `gatherSum(scalar)`: sum across all processors.
- `gatherMin(scalar)`: min from all processors.
- `gatherMax(scalar)`: max from all processors.

!---

## Built-in Postprocessor Types

MOOSE includes a large number built-in `Postprocessors`: `ElementAverageValue`, `SideAverageValue`,
`ElementL2Error`, `ElementH1Error`, and many more

By default, `Postprocessors` will output to a formatted table on the screen and optionally using
the `[Outputs]` block be stored in CSV file.

```text
[Output]
  csv = true
[]
```

!---

## Using a Postprocessor

Postprocessor values are used within an object by creating a `const` reference to a
`PostprocessorValue` and initializing the reference in the initialization list.

!listing PostprocessorDT.h line=PostprocessorValue

!listing PostprocessorDT.C line=getPostprocessorValue

!---

## Default Postprocessor Values

It is possible to set default values for `Postprocessors` to allow an object to operate without
creating `Postprocessor` object.

```cpp
params.addParam<PostprocessorName>("postprocessor", 1.2345, "Doc String");
```

Additionally, a value may be supplied in the input file in lieu of a `Postprocessor` name.
