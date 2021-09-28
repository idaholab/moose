# SolutionFunction

!syntax description /Functions/SolutionFunction

The solution is a `variable` in the file but the `SolutionFunction` makes it available
as a function. The [SolutionUserObject.md] specified as the `solution` parameter is used
to compute the variable values.

!alert note
The accuracy on the evaluation of this variable in various locations may be diminished
by the solution file format. `Exodus` for example does not store higher order variables
accurately.

## Example input syntax

In this input file, we load in `u_xda_func` a reference solution for the problem, previously
computed and stored in `aux_nonlinear_solution_out_0001.xda`, then compare it to the current solution `u`.

!listing test/tests/auxkernels/solution_aux/thread_xda.i block=Functions UserObjects Postprocessors

!syntax parameters /Functions/SolutionFunction

!syntax inputs /Functions/SolutionFunction

!syntax children /Functions/SolutionFunction
