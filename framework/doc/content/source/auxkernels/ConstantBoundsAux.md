# ConstantBoundsAux

!syntax description /AuxKernels/ConstantBoundsAux

## Description

`ConstantBoundsAux` is used for setting bounds on a nonlinear variable specified
through the parameter `bounded_variable`. The bound type of `upper` or `lower` is set with `bound_type` parameter. The `bound_value` is used to specify the value of a bound. `ConstantBoundsAux` expects a `variable` parameter to be set (as do all `AuxKernels`). This can be a dummy
`AuxVariable`; the `ConstantBoundsAux` actually operates on `NumericVectors` held by the
nonlinear system and does nothing but return 0 for the value of the specified
`variable`.

Note that in order for these bounds to have an effect, the user has to specify the
PETSc options `-snes_type vinewtonssls` or `-snes_type vinewtonrsls`. A warning will be generated if neither options are specified. The PETSc manual pages for the `vinewtonssls` algorithm
can be found
[here](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/SNES/SNESVINEWTONSSLS.html)
while the manual page for `vinewtonrsls` can be found
[here](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/SNES/SNESVINEWTONRSLS.html#SNESVINEWTONRSLS).

MOOSE users can also enforce constraints through MOOSE using
[`UpperBoundNodalKernel`](/UpperBoundNodalKernel.md) and
[`LowerBoundNodalKernel`](/LowerBoundNodalKernel.md). However, using these
objects requires introduction of Lagrange multiplier variables that increase the
size of the nonlinear system as well as render some preconditioning methods
like algebraic multi-grid ineffective. The number of nonlinear iterations
required to solve a constrained PDE defined by
[this input file](/upper-and-lower-bound.i) with the various constraint
algorithms is summarized below:

- `viewntonrsls` and `ConstantBoundsAux`:                      22
- `vinewtonssls` and `ConstantBoundsAux`:                      53
- `UpperBoundNodalKernel` and `LowerBoundNodalKernel`: 25

## Example Syntax

!listing test/tests/auxkernels/bounds/constant_bounds.i block=Bounds

!syntax parameters /AuxKernels/ConstantBoundsAux

!syntax inputs /AuxKernels/ConstantBoundsAux

!syntax children /AuxKernels/ConstantBoundsAux
