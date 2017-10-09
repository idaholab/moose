# ReferenceResidualProblem
!syntax description /Problem/ReferenceResidualProblem

## Description

Reference residual is an alternative way to signify convergence
of a timestep by using the initial value of a variable, set with an AuxKernel,
to as the initial convergence residual value.

When using reference residual it is typically acceptable to loosen the relative tolerance for
convergence by an order of magnitude. `ReferenceResidualProblem` requires the creation of
separate `AuxKernels` for each of the reference residual variables.
An additional option in each `Kernel` must be set to apply the solution variables
to the reference residual variables.

## Example Input syntax
!listing /modules/combined/test/tests/reference_residual/reference_residual.i block=Problem

where additional AuxVariables for the displacements (e.g. x, y, and z) and temperature must be created, as shown below
!listing /modules/combined/test/tests/reference_residual/reference_residual.i block=AuxVariables

and then called in the respective kernels. For the displacement reference residual `saved` variables, the option `save_in` is set in the Tensor Mechanics Action
!listing /modules/combined/test/tests/reference_residual/reference_residual.i start=Modules/TensorMechanics/Master end=Kernels

and the temperature `saved` variable is applied in the heat conduction kernel
!listing /modules/combined/test/tests/reference_residual/reference_residual.i start=Kernels end=Functions

!syntax parameters /Problem/ReferenceResidualProblem

!syntax inputs /Problem/ReferenceResidualProblem

!syntax children /Problem/ReferenceResidualProblem
