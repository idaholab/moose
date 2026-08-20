# ArcLengthHistory

!syntax description /VectorPostprocessors/ArcLengthHistory

`ArcLengthHistory` assembles the load-displacement curve of an [ArcLengthProblem.md] continuation. It
requires that problem type and errors under any other.

The table it produces has an `increment` column counting the published equilibrium states from zero, a
`lambda` column holding the load parameter at each of them, and one further column per entry of
[!param](/VectorPostprocessors/ArcLengthHistory/postprocessors), named after that postprocessor and
holding its value at the same states. A sampled postprocessor cannot itself be named `increment` or
`lambda`, because those two column names are already taken.

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

Under a [Transient.md] executioner, where [ArcLengthProblem.md] traces a path per time step, the
`lambda` column holds the total accumulated load factor rather than the step-local parameter of the
step being traced, so the curve runs continuously across the steps instead of restarting at each one.
The `increment` column counts the rows of the whole run, unlike the increment index the console banner
prints, which restarts in every step.

Every step publishes its first increment at a step-local load parameter of zero, so each one opens with
a row at the equilibrium the previous step committed, which is the point the note above leaves out of
that step's own rows. The repeat is harmless: the row carries a converged state, and the `lambda`
column does not step backwards at it.

!alert warning title=A rejected step leaves its increments in the table
The history is never reset, so the increments a step records before it is rejected stay in the table
and the retry appends its own after them. A run that cuts a step back therefore carries a dead branch
in its path file at every cutback: rows belonging to an attempt the executioner discarded and then
re-traced at a smaller load increment. The `lambda` column returns to the load factor of the last
committed step where an abandoned attempt ends and its retry begins, so a curve plotted straight from
the file doubles back on itself there. Read the file as a record of what the continuation tried
rather than only of what it committed. The shipped tests set `restep = false` for this reason.

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
