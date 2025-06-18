# [VectorPostprocessor System](syntax/VectorPostprocessors/index.md)

A system for "reduction" or "aggregation" calculations based on the solution variables
that results in one or many vectors of values.

!---

## Built-in VectorPostprocessor Types

MOOSE includes a large number built-in `VectorPostprocessors`: `NodalValueSampler`,
`LineValueSampler`, `PointValueSampler`, and many more.

`VectorPostprocessors` output is optionally enabled using the `[Outputs]` block. A CSV file
for each vector and timestep will be created.

```text
[Output]
  csv = true
[]
```
