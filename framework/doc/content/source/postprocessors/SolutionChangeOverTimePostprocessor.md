# SolutionChangeOverTimePostprocessor

The SolutionChangeOverTimePostprocessor returns the relative differential
norm of the solution between the two previous time steps. For example, if this postprocessor
is called on time step 3, it will display the relative change between time step 2 and
time step 1. The value reported by this postprocessor is consistent with the
steady state relative norm used to evaluate steady state detection for
Transient executioners.

## Syntax and Description

As an example usage, see the input below.

!listing test/tests/postprocessors/steady_state_relative_change_norm/transient.i
  block=Postprocessors

!syntax description /Postprocessors/SolutionChangeOverTimePostprocessor

!syntax parameters /Postprocessors/SolutionChangeOverTimePostprocessor

!syntax inputs /Postprocessors/SolutionChangeOverTimePostprocessor

!syntax children /Postprocessors/SolutionChangeOverTimePostprocessor

!bibtex bibliography
