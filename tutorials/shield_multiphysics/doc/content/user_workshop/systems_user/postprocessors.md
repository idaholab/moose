# [Postprocessor System](syntax/Postprocessors/index.md)

A system for computing a "reduction" or "aggregation" calculation based on the solution variables
that results in a +single+ scalar value.

!---

## Types of Postprocessors

The operation defined in the `::compute...` routine is applied at various locations
depending on the Postprocessor type.

ElementPostprocessor: operates on each element

NodalPostprocessor: operates on each node

SidePostprocessor: operates on each element side on a boundary

InternalSidePostprocessor: operates on internal element sides

InterfacePostprocessor: operates on each element side on subdomain interfaces

GeneralPostprocessor: operates once per execution

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
