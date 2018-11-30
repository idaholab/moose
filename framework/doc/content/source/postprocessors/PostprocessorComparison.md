# PostprocessorComparison

!syntax description /Postprocessors/PostprocessorComparison

This post-processor is used to compare two post-processor (or constant) values.
It returns a value of 1 for "true" and 0 for "false". There are a number of
different options for the parameter `comparison_type`. Denoting the first
value as `a` and the second as `b`, these options are as follows:

| Value                 | Test      |
|-----------------------|-----------|
| `equals`              | `a == b`? |
| `greater_than`        | `a > b`?  |
| `greater_than_equals` | `a >= b`? |
| `less_than`           | `a < b`?  |
| `less_than_equals`    | `a <= b`? |

All tests use a "fuzzy" comparison; see the corresponding functions in
[MooseUtils.md].

!syntax parameters /Postprocessors/PostprocessorComparison

!syntax inputs /Postprocessors/PostprocessorComparison

!syntax children /Postprocessors/PostprocessorComparison

!bibtex bibliography
