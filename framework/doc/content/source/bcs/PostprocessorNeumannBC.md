# PostprocessorNeumannBC

!syntax description /BCs/PostprocessorNeumannBC

More information about Neumann boundary conditions and their mathematical meaning may be found in the
[NeumannBC.md] documentation.

A common postprocessor input for the `PostprocessorNeumannBC` is the [Receiver.md] postprocessor,
which is populated by a transfer from a [syntax/MultiApps/index.md]. The two simulations are then coupled through
this boundary condition, by a numerical flux on a boundary.

## Example Input Syntax

In this example, the value of variable `aux` is sampled using a `PointValue` postprocessor then
reused as a Neumann boundary condition for the nonlinear variable `u` using a `PostprocessorNeumannBC`.

!listing test/tests/bcs/pp_neumann/pp_neumann.i block=Postprocessors BCs

!syntax parameters /BCs/PostprocessorNeumannBC

!syntax inputs /BCs/PostprocessorNeumannBC

!syntax children /BCs/PostprocessorNeumannBC
