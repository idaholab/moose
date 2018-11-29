# VectorPostprocessorComparison

!syntax description /Postprocessors/VectorPostprocessorComparison

This post-processor is used to compare two vector post-processor vectors that
have the same length. It returns a value of 1 (denoting "true") if the
comparison tests between all pairs of elements between the vectors are true.
Else it returns a value of 0 (denoting "false"). There are a number of
different options for the parameter `comparison_type`. Denoting the first
vector as `a` and the second as `b`, these options are as follows:

| Value                 | Test      |
|-----------------------|-----------|
| `equals`              | `a == b`? |
| `greater_than`        | `a > b`?  |
| `greater_than_equals` | `a >= b`? |
| `less_than`           | `a < b`?  |
| `less_than_equals`    | `a <= b`? |

All tests use a "fuzzy" comparison; see the corresponding functions in
[MooseUtils.md].

!syntax parameters /Postprocessors/VectorPostprocessorComparison

!syntax inputs /Postprocessors/VectorPostprocessorComparison

!syntax children /Postprocessors/VectorPostprocessorComparison

!bibtex bibliography
