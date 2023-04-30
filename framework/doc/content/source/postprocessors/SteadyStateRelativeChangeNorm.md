# SteadyStateRelativeChangeNorm

The SteadyStateRelativeChangeNorm Postprocessor returns the relative differential
norm of the solution between the two previous time steps. For example, if this postprocessor
is called on time step 3, it will display the relative change between time step 2 and
time step 1.

## Syntax and Description

As an example usage, see the input below.

!listing test/tests/postprocessors/steady_state_relative_change_norm/transient.i
  block=Postprocessors

!syntax description /Postprocessors/SteadyStateRelativeChangeNorm

!syntax parameters /Postprocessors/SteadyStateRelativeChangeNorm

!syntax inputs /Postprocessors/SteadyStateRelativeChangeNorm

!syntax children /Postprocessors/SteadyStateRelativeChangeNorm

!bibtex bibliography
