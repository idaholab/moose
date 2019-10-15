# BoundsAux

!syntax description /AuxKernels/BoundsAux

## Description

`BoundsAux` is used for setting bounds on a non-linear variable specified
through the parameter `bounded_variable`. The `upper` and `lower` parameters can
be used to specify the upper and lower bound respectively. `BoundsAux` expects a
`variable` parameter to be set (as do all `AuxKernels`). This can be a dummy
`AuxVariable`; the `BoundsAux` actually operates on `NumericVectors` held by the
nonlinear system and does nothing but return 0 for the value of the specified
`variable`.

Note that in order for these bounds to have an effect, the user has to specify the
PETSc options `-snes_type vinewtonssls` or `-snes_type vinewtonrsls`. Both
options emply semi-smooth non-linear solution algorithms to solve the
variational inequality. However, the `vinewtonrsls` option employs a reduced
space active set method that actually reduces the size of the linear system by
the number of active constraints. The PETSc manual pages for the `vinewtonssls` algorithm
can be found
[here](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/SNES/SNESVINEWTONSSLS.html)
while the manual page for `vinewtonrsls` can be found
[here](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/SNES/SNESVINEWTONRSLS.html#SNESVINEWTONRSLS).

MOOSE users can also enforce constraints through MOOSE using
[`UpperBoundNodalKernel`](/UpperBoundNodalKernel.md) and
[`LowerBoundNodalKernel`](/LowerBoundNodalKernel.md). However, using these
objects requires introduction of Lagrange multiplier variables that increase the
size of the non-linear system as well as render some preconditioning methods
like algebraic multi-grid ineffective. The number of non-linear iterations
required to solve a constrained PDE defined by
[this input file](/upper-and-lower-bound.i) with the various constraint
algorithms is summarized below:

- `viewntonrsls` and `BoundsAux`:                      22
- `vinewtonssls` and `BoundsAux`:                      53
- `UpperBoundNodalKernel` and `LowerBoundNodalKernel`: 25

## Example Syntax

!listing test/tests/nodalkernels/constraint_enforcement/vi-bounding.i block=Bounds

!syntax parameters /AuxKernels/BoundsAux

!syntax inputs /AuxKernels/BoundsAux

!syntax children /AuxKernels/BoundsAux
