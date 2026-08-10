# ArcLengthLoadParameter

!syntax description /Postprocessors/ArcLengthLoadParameter

The load parameter $\lambda$ scales the residual objects that [ArcLengthProblem.md] routes to its load
vector tag, and is an unknown of the continuation rather than a prescribed value, so it is the
quantity an equilibrium path is plotted against. This postprocessor reports the value $\lambda$ holds
at the most recently published continuation state. It requires
`[Problem] type = ArcLengthProblem` and errors under any other problem type. Under a [Transient.md]
executioner it reports the total accumulated load factor, which the committed steps and the step being
traced add up to, rather than the step-local parameter of that step.

[!param](/Postprocessors/ArcLengthLoadParameter/execute_on) defaults to both
`ARC_LENGTH_INCREMENT` and `TIMESTEP_END`. The first samples $\lambda$ at every equilibrium the
continuation converges to, the same states [ArcLengthHistory.md] tabulates a row at; the second
reports the value at the end of the solve, which is the load parameter the path actually finished on
and is not part of the recorded history.

## Example Input File Syntax

!listing test/tests/problems/arc_length/bratu_source.i block=Postprocessors

!syntax parameters /Postprocessors/ArcLengthLoadParameter

!syntax inputs /Postprocessors/ArcLengthLoadParameter

!syntax children /Postprocessors/ArcLengthLoadParameter
