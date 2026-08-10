# ArcLengthHistory

!syntax description /VectorPostprocessors/ArcLengthHistory

`ArcLengthHistory` assembles the load-displacement curve of an [ArcLengthProblem.md] continuation. It
requires that problem type and errors under any other.

The table it produces has an `increment` column counting the published equilibrium states from zero, a
`lambda` column holding the load parameter at each of them, and one further column per entry of
[!param](/VectorPostprocessors/ArcLengthHistory/postprocessors), named after that postprocessor and
holding its value at the same states.

Sampling happens on the `ARC_LENGTH_INCREMENT` execution flag, which
[!param](/VectorPostprocessors/ArcLengthHistory/execute_on) defaults to. Every postprocessor named in
[!param](/VectorPostprocessors/ArcLengthHistory/postprocessors) has to carry that flag in its own
`execute_on` as well; otherwise setup errors and names the offending postprocessor, because a
postprocessor on any other schedule would contribute whatever value it last happened to compute.

This object contains its complete history, so the table accumulates over the whole solve and [CSV.md]
writes it as one file for the run rather than one file per step.

!alert note title=The final equilibrium is not in the table
PETSc calls the hook this object records on at the top of each corrector iteration, so the row for a
converged increment is written once the next increment starts. The equilibrium that ends the
continuation, at [!param](/Problem/ArcLengthProblem/lambda_max), has no increment after it and so does
not reach the table — the last row stops short of it. [ArcLengthLoadParameter.md] executing on
`TIMESTEP_END` reports that final value.

## Example Input File Syntax

!listing test/tests/problems/arc_length/bratu_source.i block=VectorPostprocessors

Each sampled postprocessor carries `ARC_LENGTH_INCREMENT` alongside whatever other schedule it is
needed on:

!listing test/tests/problems/arc_length/bratu_source.i block=Postprocessors

That run writes a `path` table with `increment`, `lambda` and `u_center` columns and one row per
continuation increment.

!syntax parameters /VectorPostprocessors/ArcLengthHistory

!syntax inputs /VectorPostprocessors/ArcLengthHistory

!syntax children /VectorPostprocessors/ArcLengthHistory
